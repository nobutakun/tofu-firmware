#include "audio_input.h"
#include <Arduino.h>

namespace audio_processing {

AudioInput::AudioInput() : _buffer(nullptr), _buffer_size(0), _is_recording(false), 
                          _sample_rate(16000), _decimation_factor(64) {
    // Constructor
}

AudioInput::~AudioInput() {
    // Destructor - ensure cleanup
    stopRecording();
    if (_buffer != nullptr) {
        delete[] _buffer;
        _buffer = nullptr;
    }
}

bool AudioInput::init(size_t buffer_size, int sample_rate, int decimation_factor) {
    // Store parameters
    _sample_rate = sample_rate;
    _decimation_factor = decimation_factor;
    
    // Initialize I2S configuration
    if (!_i2s_config.init()) {
        Serial.println("Failed to initialize I2S configuration");
        return false;
    }
    
    // Initialize PDM processing
    if (!_pdm_proc.init(_sample_rate, _decimation_factor)) {
        Serial.println("Failed to initialize PDM processing");
        return false;
    }
    
    // Allocate buffer for audio samples
    _buffer_size = buffer_size;
    _buffer = new int16_t[_buffer_size];
    if (_buffer == nullptr) {
        Serial.println("Failed to allocate audio buffer");
        return false;
    }
    
    Serial.println("Audio input initialized successfully");
    return true;
}

bool AudioInput::startRecording() {
    if (_is_recording) {
        Serial.println("Already recording");
        return true;
    }
    
    _is_recording = true;
    Serial.println("Recording started");
    return true;
}

bool AudioInput::stopRecording() {
    if (!_is_recording) {
        return true;
    }
    
    _is_recording = false;
    Serial.println("Recording stopped");
    return true;
}

bool AudioInput::readAudioData(int16_t* output_buffer, size_t output_size, size_t* samples_read) {
    if (!_is_recording) {
        Serial.println("[ERROR] Not recording");
        return false;
    }

    // Read raw PDM data from I2S
    size_t pdm_bytes_read = 0;
    if (!_i2s_config.readSamples(_buffer, _buffer_size, &pdm_bytes_read)) {
        Serial.println("[ERROR] Failed to read PDM samples from I2S");
        return false;
    }

    Serial.printf("[DEBUG] Read %d bytes of PDM data\n", pdm_bytes_read);

    // Debug: Print first 10 values of PDM buffer
    Serial.print("[DEBUG] PDM buffer (first 10 samples): ");
    for (size_t i = 0; i < 10 && i < pdm_bytes_read; i++) {
        Serial.printf("%02X ", ((uint8_t*)_buffer)[i]);
    }
    Serial.println();

    // Check if no data is available
    if (pdm_bytes_read == 0) {
        Serial.println("[ERROR] No PDM data available");
        return false;
    }

    // Convert PDM to PCM using ESP-DSP
    unsigned int pcm_size = output_size; // Initialize with max size
    if (!_pdm_proc.convertPDMtoPCM((uint8_t*)_buffer, pdm_bytes_read, output_buffer, &pcm_size)) {
        Serial.println("[ERROR] Failed to convert PDM to PCM");
        return false;
    }

    Serial.printf("[DEBUG] PCM conversion successful. Output size: %d samples\n", pcm_size);

    // Debug: Print first 10 values of PCM buffer
    Serial.print("[DEBUG] PCM buffer (first 10 samples): ");
    for (size_t i = 0; i < 10 && i < pcm_size; i++) {
        Serial.printf("%d ", output_buffer[i]);
    }
    Serial.println();

    // Apply additional filtering if needed
    if (!_pdm_proc.applyFilter(output_buffer, pcm_size)) {
        Serial.println("[ERROR] Failed to apply filter");
        return false;
    }

    Serial.println("[DEBUG] Filter applied successfully");

    // Set the number of samples read
    *samples_read = pcm_size;
    Serial.printf("[DEBUG] Total PCM samples read: %d\n", *samples_read);

    return true;
}


float AudioInput::getAudioLevel() {
    if (!_is_recording || _buffer == nullptr) {
        return -100.0f; // Return very low level if not recording
    }
    
    // Read and process audio data
    int16_t temp_buffer[_buffer_size];
    size_t samples_read = 0;
    if (!readAudioData(temp_buffer, _buffer_size, &samples_read)) {
        return -100.0f;
    }
    
    return _i2s_config.calculateAudioLevel(temp_buffer, samples_read);
}

void AudioInput::deinit() {
    stopRecording();
    _i2s_config.deinit();
    _pdm_proc.deinit();
    
    if (_buffer != nullptr) {
        delete[] _buffer;
        _buffer = nullptr;
        _buffer_size = 0;
    }
}

} // namespace audio_processing 