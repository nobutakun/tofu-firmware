#include <Arduino.h>
#include "driver/i2s.h"
#include "i2s_config.h"

// PDM Microphone pins for Xiao ESP32
#define I2S_WS  6   // PDM Clock (CLK)
#define I2S_SD  2   // PDM Data (DAT)
#define I2S_PORT I2S_NUM_0

// I2S configuration parameters
#define SAMPLE_RATE 16000    // Sample rate 16kHz
#define BUFFER_SIZE 512      // Buffer size

namespace audio_processing {

bool I2SConfig::init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM), // PDM receive mode
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // 16-bit samples
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,   // Mono channel
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,    // Number of DMA buffers
        .dma_buf_len = BUFFER_SIZE, // Length of each buffer
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,   // Not used for PDM
        .ws_io_num = I2S_WS,               // CLK pin
        .data_out_num = I2S_PIN_NO_CHANGE, // Not used for input
        .data_in_num = I2S_SD              // DAT pin
    };

    // Install and configure I2S driver
    esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.println("Failed to install I2S driver");
        return false;
    }

    // Set I2S pins
    result = i2s_set_pin(I2S_PORT, &pin_config);
    if (result != ESP_OK) {
        Serial.println("Failed to set I2S pins");
        return false;
    }

    // Set I2S clock
    result = i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    if (result != ESP_OK) {
        Serial.println("Failed to set I2S clock");
        return false;
    }

    Serial.println("I2S PDM Microphone initialized successfully");
    _initialized = true;
    return true;
}

bool I2SConfig::readSamples(int16_t* buffer, size_t buffer_size, size_t* bytes_read) {
    if (!_initialized) {
        Serial.println("I2S not initialized");
        return false;
    }
    
    esp_err_t result = i2s_read(I2S_PORT, buffer, buffer_size * sizeof(int16_t), bytes_read, portMAX_DELAY);
    return (result == ESP_OK);
}

float I2SConfig::calculateAudioLevel(int16_t* buffer, size_t samples_count) {
    float amplitude = 0;
    for (int i = 0; i < samples_count; i++) {
        amplitude += abs(buffer[i]);
    }
    amplitude /= samples_count;
    
    // Convert to decibels (avoid log of zero)
    if (amplitude < 1) amplitude = 1;
    float decibels = 20 * log10(amplitude);
    
    return decibels;
}

void I2SConfig::deinit() {
    if (_initialized) {
        i2s_driver_uninstall(I2S_PORT);
        _initialized = false;
    }
}

I2SConfig::I2SConfig() : _initialized(false) {
    // Constructor
}

I2SConfig::~I2SConfig() {
    // Destructor - ensure I2S is properly deinitialized
    deinit();
}

} // namespace audio_processing
