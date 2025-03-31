#include <Arduino.h>
#include "pdm_processing.h"
#include "dsps_fir.h"
#include "dsps_conv.h"
#include "dsp_platform.h"
#include <math.h>
#include "esp_heap_caps.h"
#include <vector>

namespace audio_processing {

PDMProcessing::PDMProcessing() 
    : _initialized(false), _sample_rate(0), _bit_depth(0),
      _fir_coeffs(nullptr), _delay_line(nullptr),
      _pdm_float_buffer(nullptr), _pcm_float_buffer(nullptr),
      _filter_len(64), _decimation_factor(64) {
    memset(&_fir_filter, 0, sizeof(fir_f32_t));
}

PDMProcessing::~PDMProcessing() {
    deinit();
}

bool PDMProcessing::createFIRFilter() {
    // Calculate normalized cutoff frequency
    float cutoff = 0.5f / _decimation_factor;
    
    // Create filter coefficients using windowed sinc
    for (int i = 0; i < _filter_len; i++) {
        if (i == _filter_len / 2) {
            _fir_coeffs[i] = 2.0f * cutoff;
        } else {
            float x = M_PI * (i - _filter_len / 2);
            _fir_coeffs[i] = sin(2.0f * cutoff * x) / x;
        }
        // Apply Hamming window
        _fir_coeffs[i] *= (0.54f - 0.46f * cos(2.0f * M_PI * i / (_filter_len - 1)));
    }
    
    // Initialize FIR filter with ESP-DSP
    esp_err_t result = dsps_fir_init_f32(&_fir_filter, _fir_coeffs, _delay_line, _filter_len);
    if (result != ESP_OK) {
        Serial.println("Failed to initialize FIR filter");
        return false;
    }
    
    return true;
}

bool PDMProcessing::init(int sample_rate, int bit_depth) {
    _sample_rate = sample_rate;
    _bit_depth = bit_depth;
    _decimation_factor = bit_depth;
    
    // Add size checks
    size_t required_size = _filter_len * sizeof(float);
    if (heap_caps_get_free_size(MALLOC_CAP_8BIT) < required_size * 4) {
        Serial.println("Not enough memory for buffers");
        return false;
    }
    
    // Allocate memory with alignment
    _fir_coeffs = (float*)heap_caps_aligned_alloc(16, _filter_len * sizeof(float), MALLOC_CAP_8BIT);
    _delay_line = (float*)heap_caps_aligned_alloc(16, _filter_len * sizeof(float), MALLOC_CAP_8BIT);
    _pdm_float_buffer = (float*)heap_caps_aligned_alloc(16, 2048 * sizeof(float), MALLOC_CAP_8BIT);
    _pcm_float_buffer = (float*)heap_caps_aligned_alloc(16, 2048 * sizeof(float), MALLOC_CAP_8BIT);
    
    if (!_fir_coeffs || !_delay_line || !_pdm_float_buffer || !_pcm_float_buffer) {
        Serial.println("Failed to allocate memory for processing buffers");
        deinit();
        return false;
    }
    
    // Add debug logging
    Serial.printf("Memory allocated: fir_coeffs=%p, delay_line=%p\n", _fir_coeffs, _delay_line);
    Serial.printf("Memory allocated: pdm_buffer=%p, pcm_buffer=%p\n", _pdm_float_buffer, _pcm_float_buffer);
    
    // Initialize delay line
    memset(_delay_line, 0, _filter_len * sizeof(float));
    
    // Create and initialize FIR filter
    if (!createFIRFilter()) {
        deinit();
        return false;
    }
    
    _initialized = true;
    Serial.println("PDM Processing initialized successfully with ESP-DSP");
    return true;
}

void PDMProcessing::pdmBitsToFloat(const uint8_t* pdm_data, unsigned int pdm_size, float* float_buffer) {
    for (unsigned int i = 0; i < pdm_size; i++) {
        uint8_t byte = pdm_data[i];
        for (int bit = 0; bit < 8; bit++) {
            float_buffer[i * 8 + bit] = (byte & (1 << bit)) ? 1.0f : -1.0f;
        }
    }
}

void PDMProcessing::floatToPCM(const float* float_buffer, unsigned int buffer_size, int16_t* pcm_data) {
    for (unsigned int i = 0; i < buffer_size; i++) {
        float sample = float_buffer[i] * 32767.0f;
        sample = fminf(32767.0f, fmaxf(-32768.0f, sample));
        pcm_data[i] = (int16_t)sample;
    }
}

bool PDMProcessing::convertPDMtoPCM(const uint8_t* pdm_data, unsigned int pdm_size, 
                                   int16_t* pcm_data, unsigned int* pcm_samples) {
    if (!_initialized) {
        Serial.println("PDMProcessing not initialized.");
        return false;
    }
    if (!pdm_data || !pcm_data || !pcm_samples) {
        Serial.println("Invalid input parameters.");
        return false;
    }

    Serial.printf("Converting PDM to PCM. PDM size: %d\n", pdm_size);

    const unsigned int CHUNK_SIZE = 256;
    unsigned int total_samples = 0;
    
    // Pre-calculate total expected samples
    unsigned int expected_samples = pdm_size / _decimation_factor;
    Serial.printf("Expected PCM samples: %d\n", expected_samples);
    
    for (unsigned int offset = 0; offset < pdm_size; offset += CHUNK_SIZE) {
        unsigned int chunk_size = (offset + CHUNK_SIZE > pdm_size) ? 
                                (pdm_size - offset) : CHUNK_SIZE;
        
        Serial.printf("Processing chunk %d/%d, size: %d\n", 
                     offset/CHUNK_SIZE + 1, 
                     (pdm_size + CHUNK_SIZE - 1)/CHUNK_SIZE,
                     chunk_size);
        
        // Convert PDM chunk to float
        pdmBitsToFloat(&pdm_data[offset], chunk_size, _pdm_float_buffer);
        
        // Apply decimation and filtering
        unsigned int chunk_samples = chunk_size / _decimation_factor;
        
        if (chunk_samples == 0) {
            Serial.printf("Warning: Chunk size %d too small for decimation factor %d\n", 
                         chunk_size, _decimation_factor);
            continue;
        }
        
        // Allocate int16_t buffer using PSRAM if available
        int16_t* int_buffer = (int16_t*)heap_caps_malloc(chunk_samples * sizeof(int16_t), MALLOC_CAP_SPIRAM);
        if (!int_buffer) {
            Serial.println("Failed to allocate memory for int_buffer.");
            return false;
        }

        // Convert float buffer to int16_t
        for (unsigned int i = 0; i < chunk_samples; i++) {
            int_buffer[i] = static_cast<int16_t>(_pdm_float_buffer[i] * 32767.0f);
        }

        // Apply filter
        if (!applyFilter(int_buffer, chunk_samples)) {
            Serial.println("Filter application failed.");
            free(int_buffer);
            return false;
        }
        
        // Copy to PCM buffer
        if (total_samples + chunk_samples > expected_samples) {
            Serial.println("Error: Buffer overflow prevented.");
            free(int_buffer);
            return false;
        }
        
        memcpy(&pcm_data[total_samples], int_buffer, chunk_samples * sizeof(int16_t));
        total_samples += chunk_samples;
        
        free(int_buffer);
        yield();
    }
    
    *pcm_samples = total_samples;
    Serial.printf("PCM conversion successful. Total samples: %d\n", total_samples);
    return true;
}

bool PDMProcessing::convertPDMtoWAV(const uint8_t* pdm_data, unsigned int pdm_size, 
                                    std::vector<uint8_t>& wav_data) {
    if (!_initialized) {
        Serial.println("PDMProcessing not initialized.");
        return false;
    }
    if (!pdm_data || pdm_size == 0) {
        Serial.println("Invalid PDM data or size.");
        return false;
    }

    // Convert PDM to PCM first
    std::vector<int16_t> pcm_data(pdm_size / _decimation_factor);
    unsigned int pcm_samples = 0;
    if (!convertPDMtoPCM(pdm_data, pdm_size, pcm_data.data(), &pcm_samples)) {
        Serial.println("Failed to convert PDM to PCM.");
        return false;
    }

    // Write WAV header
    writeWAVHeader(wav_data, _sample_rate, pcm_samples);

    // Append PCM data to WAV data
    for (int16_t sample : pcm_data) {
        wav_data.push_back(sample & 0xFF);
        wav_data.push_back((sample >> 8) & 0xFF);
    }

    Serial.println("WAV conversion successful.");
    return true;
}

void PDMProcessing::writeWAVHeader(std::vector<uint8_t>& wav_data, int sample_rate, int num_samples) {
    int byte_rate = sample_rate * 2; // 16-bit audio
    int block_align = 2; // 16-bit audio
    int subchunk2_size = num_samples * 2;
    int chunk_size = 36 + subchunk2_size;

    // RIFF header
    wav_data.push_back('R');
    wav_data.push_back('I');
    wav_data.push_back('F');
    wav_data.push_back('F');
    wav_data.push_back(chunk_size & 0xFF);
    wav_data.push_back((chunk_size >> 8) & 0xFF);
    wav_data.push_back((chunk_size >> 16) & 0xFF);
    wav_data.push_back((chunk_size >> 24) & 0xFF);
    wav_data.push_back('W');
    wav_data.push_back('A');
    wav_data.push_back('V');
    wav_data.push_back('E');

    // fmt subchunk
    wav_data.push_back('f');
    wav_data.push_back('m');
    wav_data.push_back('t');
    wav_data.push_back(' ');
    wav_data.push_back(16);
    wav_data.push_back(0);
    wav_data.push_back(0);
    wav_data.push_back(0);
    wav_data.push_back(1);
    wav_data.push_back(0);
    wav_data.push_back(1);
    wav_data.push_back(0);
    wav_data.push_back(sample_rate & 0xFF);
    wav_data.push_back((sample_rate >> 8) & 0xFF);
    wav_data.push_back((sample_rate >> 16) & 0xFF);
    wav_data.push_back((sample_rate >> 24) & 0xFF);
    wav_data.push_back(byte_rate & 0xFF);
    wav_data.push_back((byte_rate >> 8) & 0xFF);
    wav_data.push_back((byte_rate >> 16) & 0xFF);
    wav_data.push_back((byte_rate >> 24) & 0xFF);
    wav_data.push_back(block_align);
    wav_data.push_back(0);
    wav_data.push_back(16);
    wav_data.push_back(0);

    // data subchunk
    wav_data.push_back('d');
    wav_data.push_back('a');
    wav_data.push_back('t');
    wav_data.push_back('a');
    wav_data.push_back(subchunk2_size & 0xFF);
    wav_data.push_back((subchunk2_size >> 8) & 0xFF);
    wav_data.push_back((subchunk2_size >> 16) & 0xFF);
    wav_data.push_back((subchunk2_size >> 24) & 0xFF);
}

bool PDMProcessing::applyFilter(int16_t* pcm_data, unsigned int pcm_samples) {
    if (!_initialized || pcm_samples < 2) {
        return false;
    }
    
    // Convert PCM to float
    for (unsigned int i = 0; i < pcm_samples; i++) {
        _pdm_float_buffer[i] = pcm_data[i] / 32768.0f;
    }
    
    // Apply additional filtering using ESP-DSP FIR filter
    esp_err_t result = dsps_fir_f32(&_fir_filter, _pdm_float_buffer, _pcm_float_buffer, pcm_samples);
    if (result != ESP_OK) {
        return false;
    }
    
    // Convert back to PCM
    floatToPCM(_pcm_float_buffer, pcm_samples, pcm_data);
    
    return true;
}

void PDMProcessing::deinit() {
    if (_initialized) {
        Serial.println("Starting deinitialization...");
        
        // Stop any ongoing processing first
        _initialized = false;
        
        // Add delay to ensure no ongoing operations
        delay(100);
        
        if (_fir_coeffs) {
            Serial.printf("Freeing fir_coeffs at %p\n", _fir_coeffs);
            heap_caps_free(_fir_coeffs);
            _fir_coeffs = nullptr;
        }
        
        delay(10); // Add small delay between frees
        
        if (_delay_line) {
            Serial.printf("Freeing delay_line at %p\n", _delay_line);
            heap_caps_free(_delay_line);
            _delay_line = nullptr;
        }
        
        delay(10);
        
        if (_pdm_float_buffer) {
            Serial.printf("Freeing pdm_buffer at %p\n", _pdm_float_buffer);
            heap_caps_free(_pdm_float_buffer);
            _pdm_float_buffer = nullptr;
        }
        
        delay(10);
        
        if (_pcm_float_buffer) {
            Serial.printf("Freeing pcm_buffer at %p\n", _pcm_float_buffer);
            heap_caps_free(_pcm_float_buffer);
            _pcm_float_buffer = nullptr;
        }
        
        Serial.println("PDM Processing deinitialized");
    }
}

} // namespace audio_processing 