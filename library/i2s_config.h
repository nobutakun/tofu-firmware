#ifndef I2S_CONFIG_H
#define I2S_CONFIG_H

#include <Arduino.h>

namespace audio_processing {

/**
 * @class I2SConfig
 * @brief Handles configuration and operation of the I2S interface for PDM microphone
 */
class I2SConfig {
private:
    bool _initialized;  // Flag to track initialization state

public:
    /**
     * Constructor
     */
    I2SConfig();
    
    /**
     * Destructor - ensures proper cleanup
     */
    ~I2SConfig();
    
    /**
     * Initialize the I2S interface for PDM microphone
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init();
    
    /**
     * Read audio samples from the PDM microphone
     * 
     * @param buffer Buffer to store the samples
     * @param buffer_size Size of the buffer in samples
     * @param bytes_read Pointer to store the number of bytes read
     * @return true if read was successful, false otherwise
     */
    bool readSamples(int16_t* buffer, size_t buffer_size, size_t* bytes_read);
    
    /**
     * Calculate audio level in decibels from raw samples
     * 
     * @param buffer Buffer containing audio samples
     * @param samples_count Number of samples in the buffer
     * @return Audio level in decibels
     */
    float calculateAudioLevel(int16_t* buffer, size_t samples_count);
    
    /**
     * Deinitialize the I2S interface
     */
    void deinit();
};

} // namespace audio_processing

#endif // I2S_CONFIG_H 