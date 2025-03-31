#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include <Arduino.h>
#include "i2s_config.h"
#include "pdm_processing.h"

namespace audio_processing {

/**
 * @class AudioInput
 * @brief Handles audio input from PDM microphone using I2S
 */
class AudioInput {
private:
    I2SConfig _i2s_config;     // I2S configuration
    PDMProcessing _pdm_proc;   // PDM processing
    int16_t* _buffer;          // Buffer for audio samples
    size_t _buffer_size;       // Size of the buffer
    bool _is_recording;        // Recording state flag
    int _sample_rate;          // Audio sample rate
    int _decimation_factor;    // PDM decimation factor

public:
    /**
     * Constructor
     */
    AudioInput();
    
    /**
     * Destructor - ensures proper cleanup
     */
    ~AudioInput();
    
    /**
     * Initialize audio input with specified buffer size
     * 
     * @param buffer_size Size of the internal buffer in samples
     * @param sample_rate Audio sample rate (default: 16000)
     * @param decimation_factor PDM decimation factor (default: 64)
     * @return true if initialization was successful, false otherwise
     */
    bool init(size_t buffer_size = 512, int sample_rate = 16000, int decimation_factor = 64);
    
    /**
     * Start recording audio
     * 
     * @return true if recording started successfully, false otherwise
     */
    bool startRecording();
    
    /**
     * Stop recording audio
     * 
     * @return true if recording stopped successfully, false otherwise
     */
    bool stopRecording();
    
    /**
     * Read audio data into the provided buffer
     * 
     * @param output_buffer Buffer to store the audio samples
     * @param output_size Size of the output buffer in samples
     * @param samples_read Pointer to store the number of samples read
     * @return true if read was successful, false otherwise
     */
    bool readAudioData(int16_t* output_buffer, size_t output_size, size_t* samples_read);
    
    /**
     * Get current audio level in decibels
     * 
     * @return Audio level in decibels
     */
    float getAudioLevel();
    
    /**
     * Deinitialize audio input and free resources
     */
    void deinit();
    
    /**
     * Check if currently recording
     * 
     * @return true if recording, false otherwise
     */
    inline bool isRecording() const { return _is_recording; }
};

} // namespace audio_processing

#endif // AUDIO_INPUT_H 