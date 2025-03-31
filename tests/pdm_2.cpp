#include <Arduino.h>
#include <driver/i2s.h>

#define PDM_CLK_PIN     7
#define PDM_DATA_PIN    9
#define BUFFER_SIZE     512

int32_t sBuffer[BUFFER_SIZE];

// Moving average variables
const int avgSamples = 10;
int32_t readings[avgSamples];
int readIndex = 0;
int32_t total = 0;
int32_t average = 0;

void setup() {
    Serial.begin(115200);
    while(!Serial) delay(10);
    delay(1000);
    Serial.println("\nPDM Microphone Test");

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
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
    
    // Initialize moving average array
    for (int i = 0; i < avgSamples; i++) {
        readings[i] = 0;
    }
}

void loop() {
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_NUM_0, sBuffer, sizeof(sBuffer), &bytesRead, portMAX_DELAY);
    
    if (err != ESP_OK) {
        Serial.printf("Read failed: %d\n", err);
        return;
    }

    // Calculate average of current buffer
    int32_t sum = 0;
    int samples = bytesRead / 4;
    for (int i = 0; i < samples; i++) {
        sum += abs(sBuffer[i]);
    }
    int32_t bufferAvg = sum / samples;

    // Update moving average
    total = total - readings[readIndex];
    readings[readIndex] = bufferAvg;
    total = total + readings[readIndex];
    readIndex = (readIndex + 1) % avgSamples;
    average = total / avgSamples;

    // Scale the value to 0-100 for easier reading
    int scaled = map(average, 0, 20000000, 0, 100);
    
    // Print visual meter
    Serial.print("Level: [");
    for (int i = 0; i < 50; i++) {
        if (i < scaled/2) {
            Serial.print("=");
        } else {
            Serial.print(" ");
        }
    }
    Serial.print("] ");
    Serial.print(scaled);
    Serial.print("% (");
    Serial.print(average);
    Serial.println(")");

    delay(50);  // Adjust this value to change update speed
}