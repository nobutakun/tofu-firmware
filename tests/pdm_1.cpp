#include <driver/i2s.h>
#include <Arduino.h>

// Try swapping these pins to test
#define PDM_CLK_PIN     9  // Changed from 7 to 9
#define PDM_DATA_PIN    7  // Changed from 9 to 7

// Buffer size
#define BUFFER_SIZE     512
int32_t sBuffer[BUFFER_SIZE];

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    while(!Serial) delay(10);
    delay(1000);
    Serial.println("\nPDM Microphone Diagnostic Test");

    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 44100,           // Changed to 44.1kHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Changed to 16-bit
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,  // Try RIGHT instead of LEFT
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,            // Increased buffer count
        .dma_buf_len = 64,             // Decreased buffer length
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

    // Install I2S driver
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Driver install failed: %d\n", err);
        while (1);
    }

    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Pin config failed: %d\n", err);
        while (1);
    }

    i2s_zero_dma_buffer(I2S_NUM_0);
    
    Serial.println("Testing microphone connection...");
    Serial.println("You should see varying numbers when tapping the mic");
}

void loop() {
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_NUM_0, sBuffer, sizeof(sBuffer), &bytesRead, portMAX_DELAY);
    
    if (err != ESP_OK) {
        Serial.printf("Read failed: %d\n", err);
        return;
    }

    // Print raw values for debugging
    Serial.print("Values: ");
    for (int i = 0; i < 5; i++) {
        Serial.print(sBuffer[i]);
        Serial.print(" ");
    }
    Serial.println();

    // Print in binary format to check bit patterns
    Serial.print("Binary: ");
    Serial.println(sBuffer[0], BIN);
    
    delay(100);
}