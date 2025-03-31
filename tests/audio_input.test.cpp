#include <Arduino.h>
#include <unity.h>
#include "../library/audio_input.h"
#include "../library/i2s_config.h"
#include "driver/i2s.h"

using namespace audio_processing;

// Test instances
AudioInput* audioInput = nullptr;
I2SConfig* i2sConfig = nullptr;

// Test configuration constants
const int TEST_I2S_WS = 7;   // PDM Clock (CLK)
const int TEST_I2S_SD = 8;   // PDM Data (DAT)
const int TEST_SAMPLE_RATE = 16000;
const size_t TEST_BUFFER_SIZE = 512;

void setUp(void) {
    Serial.println("Setting up test...");
    // Create new instances before each test
    i2sConfig = new I2SConfig();
    audioInput = new AudioInput();
}

void tearDown(void) {
    Serial.println("Tearing down test...");
    // Clean up after each test
    if (audioInput != nullptr) {
        audioInput->deinit();
        delete audioInput;
        audioInput = nullptr;
    }
    if (i2sConfig != nullptr) {
        i2sConfig->deinit();
        delete i2sConfig;
        i2sConfig = nullptr;
    }
}

void test_i2s_configuration() {
    Serial.println("Testing I2S configuration...");
    TEST_ASSERT_NOT_NULL(i2sConfig);
    bool init_result = i2sConfig->init();
    TEST_ASSERT_TRUE(init_result);
}

void test_i2s_sample_reading() {
    Serial.println("Testing I2S sample reading...");
    TEST_ASSERT_TRUE(i2sConfig->init());
    
    // Test reading samples
    int16_t buffer[TEST_BUFFER_SIZE];
    size_t bytes_read = 0;
    bool read_result = i2sConfig->readSamples(buffer, TEST_BUFFER_SIZE, &bytes_read);
    TEST_ASSERT_TRUE(read_result);
    TEST_ASSERT_GREATER_THAN(0, bytes_read);
    Serial.printf("Bytes read: %d\n", bytes_read);
}

void test_audio_input_initialization() {
    Serial.println("Testing audio input initialization...");
    TEST_ASSERT_NOT_NULL(audioInput);
    bool init_result = audioInput->init(TEST_BUFFER_SIZE);
    TEST_ASSERT_TRUE(init_result);
}

void test_recording_state() {
    Serial.println("Testing recording state...");
    TEST_ASSERT_TRUE(audioInput->init(TEST_BUFFER_SIZE));
    
    // Test initial state
    TEST_ASSERT_FALSE(audioInput->isRecording());
    
    // Test start recording
    TEST_ASSERT_TRUE(audioInput->startRecording());
    TEST_ASSERT_TRUE(audioInput->isRecording());
    
    // Test stop recording
    TEST_ASSERT_TRUE(audioInput->stopRecording());
    TEST_ASSERT_FALSE(audioInput->isRecording());
}

void test_audio_data_reading() {
    Serial.println("Testing audio data reading...");
    int16_t buffer[TEST_BUFFER_SIZE];
    size_t samples_read = 0;
    
    // Initialize and start recording
    TEST_ASSERT_TRUE(audioInput->init(TEST_BUFFER_SIZE));
    TEST_ASSERT_TRUE(audioInput->startRecording());
    
    // Test reading data
    bool read_result = audioInput->readAudioData(buffer, TEST_BUFFER_SIZE, &samples_read);
    TEST_ASSERT_TRUE(read_result);
    TEST_ASSERT_GREATER_THAN(0, samples_read);
    Serial.printf("Samples read: %d\n", samples_read);
    
    // Test audio level
    float level = audioInput->getAudioLevel();
    TEST_ASSERT_GREATER_OR_EQUAL(-100.0f, level);
    Serial.printf("Audio level: %.2f dB\n", level);
}

void test_invalid_operations() {
    Serial.println("Testing invalid operations...");
    int16_t buffer[TEST_BUFFER_SIZE];
    size_t samples_read = 0;
    
    // Test reading without initialization
    TEST_ASSERT_FALSE(audioInput->readAudioData(buffer, TEST_BUFFER_SIZE, &samples_read));
    
    // Initialize but don't start recording
    TEST_ASSERT_TRUE(audioInput->init(TEST_BUFFER_SIZE));
    TEST_ASSERT_FALSE(audioInput->readAudioData(buffer, TEST_BUFFER_SIZE, &samples_read));
    
    // Test audio level without recording
    float level = audioInput->getAudioLevel();
    TEST_ASSERT_EQUAL_FLOAT(-100.0f, level);
}

void test_audio_level_calculation() {
    Serial.println("Testing audio level calculation...");
    TEST_ASSERT_TRUE(i2sConfig->init());
    
    // Create test buffer with known values
    int16_t test_buffer[TEST_BUFFER_SIZE];
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        test_buffer[i] = 1000; // Fixed amplitude for predictable result
    }
    
    float level = i2sConfig->calculateAudioLevel(test_buffer, TEST_BUFFER_SIZE);
    TEST_ASSERT_GREATER_THAN(0.0f, level);
    Serial.printf("Calculated audio level: %.2f dB\n", level);
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting Audio Input Tests ===\n");
    UNITY_BEGIN();
    
    RUN_TEST(test_i2s_configuration);
    RUN_TEST(test_i2s_sample_reading);
    RUN_TEST(test_audio_input_initialization);
    RUN_TEST(test_recording_state);
    RUN_TEST(test_audio_data_reading);
    RUN_TEST(test_invalid_operations);
    RUN_TEST(test_audio_level_calculation);
    
    int result = UNITY_END();
    Serial.printf("\n=== Test Results: %d tests completed ===\n", result);
}

void loop() {
    // Empty loop
}
