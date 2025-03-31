#ifndef PDM_PROCESSING_H
#define PDM_PROCESSING_H

#include <Arduino.h>
#include "dsps_fir.h"  // ESP-DSP FIR filter
#include "dsps_conv.h" // ESP-DSP convolution
#include <vector>

namespace audio_processing {

class PDMProcessing {
public:
    PDMProcessing();
    ~PDMProcessing();

    bool init(int sample_rate, int bit_depth);
    bool convertPDMtoPCM(const uint8_t* pdm_data, unsigned int pdm_size, 
                        int16_t* pcm_data, unsigned int* pcm_samples);
    bool convertPDMtoWAV(const uint8_t* pdm_data, unsigned int pdm_size, 
                         std::vector<uint8_t>& wav_data);
    bool applyFilter(int16_t* pcm_data, unsigned int pcm_samples);
    void deinit();

    // Add these methods to access internal buffers
    float* getPDMFloatBuffer() const { return _pdm_float_buffer; }
    float* getPCMFloatBuffer() const { return _pcm_float_buffer; }

private:
    bool _initialized;
    int _sample_rate;
    int _bit_depth;
    
    // ESP-DSP related members
    fir_f32_t _fir_filter;    // FIR filter structure
    float* _fir_coeffs;       // FIR filter coefficients
    float* _delay_line;       // Delay line for FIR filter
    float* _pdm_float_buffer; // Temporary buffer for float conversion
    float* _pcm_float_buffer; // Temporary buffer for float conversion
    int _filter_len;          // Length of the FIR filter
    int _decimation_factor;   // Decimation factor for PDM to PCM
    
    // Helper methods
    void pdmBitsToFloat(const uint8_t* pdm_data, unsigned int pdm_size, float* float_buffer);
    void floatToPCM(const float* float_buffer, unsigned int buffer_size, int16_t* pcm_data);
    bool createFIRFilter();
    void writeWAVHeader(std::vector<uint8_t>& wav_data, int sample_rate, int num_samples);
};

} // namespace audio_processing

#endif // PDM_PROCESSING_H
