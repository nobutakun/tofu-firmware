#include <Arduino.h>
#include "driver/i2s.h"

#define I2S_WS  6   // PDM Clock (CLK)
#define I2S_SD  2   // PDM Data (DAT)
#define I2S_PORT I2S_NUM_0

#define SAMPLE_RATE 16000    // Tần số lấy mẫu 16kHz
#define BUFFER_SIZE 512      // Kích thước buffer

int16_t i2s_buffer[BUFFER_SIZE];  // Buffer để lưu dữ liệu

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM), // Chế độ thu PDM
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // 16-bit hoặc 32-bit
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,   // Chỉ thu 1 kênh
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,    // Số buffer DMA
        .dma_buf_len = BUFFER_SIZE, // Độ dài mỗi buffer
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,   // Không dùng BCK (PDM không cần)
        .ws_io_num = I2S_WS,               // CLK
        .data_out_num = I2S_PIN_NO_CHANGE, // Không dùng output
        .data_in_num = I2S_SD              // DAT
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void setup() {
    Serial.begin(115200);
    setupI2S();
}

void loop() {
    size_t bytes_read;
    esp_err_t result = i2s_read(I2S_PORT, i2s_buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    if (result == ESP_OK) {
        Serial.print("Received Bytes: ");
        Serial.println(bytes_read);
        
        float amplitude = 0;
        for (int i = 0; i < bytes_read / 2; i++) {
            amplitude += abs(i2s_buffer[i]);
        }
        amplitude /= (bytes_read / 2);
        float decibels = 20 * log10(amplitude); 
        Serial.print("Âm lượng (dB): ");
        Serial.println(decibels);
        
        for (int i = 0; i < bytes_read / 2; i++) {
            Serial.println(i2s_buffer[i]); // In dữ liệu âm thanh
        }
    }
    delay(100);
}