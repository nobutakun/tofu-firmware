#include <Arduino.h>
#include <driver/i2s.h>

#define PDM_CLK_PIN     7
#define PDM_DATA_PIN    9
#define BUFFER_SIZE     512

int32_t sBuffer[BUFFER_SIZE];

// Audio level processing variables
const int avgSamples = 8;    // Increased for smoother response
int32_t readings[avgSamples];
int readIndex = 0;
int32_t total = 0;
int32_t average = 0;

// Dynamic range variables
const float ALPHA = 0.1;     // Smoothing factor (0-1), lower = smoother
float minLevel = 0;
float maxLevel = 1;
const float MIN_THRESHOLD = 1000;  // Noise floor

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

// Function to calculate decibels from amplitude
float amplitudeToDb(float amplitude) {
    if (amplitude < MIN_THRESHOLD) return 0;
    return 20 * log10(amplitude / 32767.0);  // 32767 is max for 16-bit
}

void loop() {
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_NUM_0, sBuffer, sizeof(sBuffer), &bytesRead, portMAX_DELAY);
    
    if (err != ESP_OK) {
        Serial.printf("Read failed: %d\n", err);
        return;
    }

    // Calculate RMS value of current buffer
    uint64_t sum_squared = 0;
    int samples = bytesRead / 4;
    for (int i = 0; i < samples; i++) {
        int32_t sample = abs(sBuffer[i]);
        sum_squared += (uint64_t)sample * sample;
    }
    float rms = sqrt(sum_squared / samples);

    // Update dynamic range
    if (rms > maxLevel) {
        maxLevel = rms;
    } else {
        maxLevel = maxLevel * (1 - ALPHA) + rms * ALPHA;
    }
    
    if (rms < minLevel || minLevel == 0) {
        minLevel = rms;
    } else {
        minLevel = minLevel * (1 - ALPHA) + rms * ALPHA;
    }

    // Calculate decibels
    float db = amplitudeToDb(rms);
    
    // Map to percentage with dynamic range
    int scaled = 0;
    if (rms > MIN_THRESHOLD) {
        float range = maxLevel - minLevel;
        if (range > 0) {
            scaled = map(rms, minLevel, maxLevel, 10, 100);
            scaled = constrain(scaled, 0, 100);
        }
    }

    // Print visual meter with different characters for different levels
    Serial.print("Level: [");
    for (int i = 0; i < 50; i++) {
        int threshold = i * 2; // Convert position to percentage
        if (threshold < scaled) {
            if (threshold < 30) Serial.print("-");      // Low level
            else if (threshold < 60) Serial.print("=");  // Medium level
            else if (threshold < 80) Serial.print("#");  // High level
            else Serial.print("*");                      // Peak level
        } else {
            Serial.print(" ");
        }
    }
    Serial.print("] ");
    Serial.print(scaled);
    Serial.print("% ");
    Serial.printf("(%.1f dB)", db);
    Serial.println();

    delay(30);  // Slightly faster update rate
}