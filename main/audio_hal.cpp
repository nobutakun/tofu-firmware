#include <Arduino.h>
#include <driver/i2s.h>

// PDM Microphone pins for Xiao ESP32
#define PDM_DATA_PIN 8  // Data pin for PDM microphone
#define PDM_CLK_PIN  7  // Clock pin for PDM microphone

// I2S configuration
#define SAMPLE_RATE     16000  // Audio sampling rate
#define SAMPLE_BITS     16     // Sample bits
#define CHANNELS        1      // Mono channel
#define I2S_PORT       I2S_NUM_0

class AudioManager {
private:
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = PDM_CLK_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PDM_DATA_PIN
    };

public:
    bool begin() {
        // Install and start I2S driver
        esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
        if (err != ESP_OK) {
            Serial.println("Failed to install I2S driver");
            return false;
        }

        // Set I2S pins
        err = i2s_set_pin(I2S_PORT, &pin_config);
        if (err != ESP_OK) {
            Serial.println("Failed to set I2S pins");
            return false;
        }

        Serial.println("I2S PDM Microphone initialized successfully");
        return true;
    }

    void readSamples(int16_t* samples, size_t count) {
        size_t bytes_read = 0;
        i2s_read(I2S_PORT, samples, count * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    }

    void end() {
        i2s_driver_uninstall(I2S_PORT);
    }
};

// Global instance
AudioManager audioManager;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("Initializing PDM Microphone...");
    
    if (!audioManager.begin()) {
        Serial.println("Failed to initialize audio!");
        while (1) delay(100);
    }
}

void loop() {
    // Buffer for audio samples
    const int SAMPLES_COUNT = 256;
    int16_t samples[SAMPLES_COUNT];
    
    // Read audio samples
    audioManager.readSamples(samples, SAMPLES_COUNT);
    
    // Process samples here as needed
    // For example, print the first few samples
    for (int i = 0; i < 5; i++) {
        Serial.print(samples[i]);
        Serial.print(" ");
    }
    Serial.println();
    
    delay(100); // Small delay to prevent flooding the serial monitor
}
