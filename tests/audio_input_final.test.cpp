#include <Arduino.h>
#include <unity.h>
#include "../library/audio_input.h"
#include "../library/i2s_config.h"
#include "../library/pdm_processing.h"
#include "driver/i2s.h"
#include "esp_heap_caps.h"

using namespace audio_processing;

// Test instances
AudioInput* audioInput = nullptr;
PDMProcessing* pdmProc = nullptr;
I2SConfig* i2sConfig = nullptr;   

// Test configuration constants
const int TEST_SAMPLE_RATE = 16000;
const int TEST_DECIMATION_FACTOR = 64;
const size_t TEST_BUFFER_SIZE = 512;
const int TEST_I2S_WS = 7;   // PDM Clock (CLK)
const int TEST_I2S_SD = 8;   // PDM Data (DAT)

// Test data
const uint8_t TEST_PDM_DATA[] = {
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // Alternating pattern
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,  // Alternating pattern
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,  // All zeros then all ones
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00   // All ones then all zeros
};
const size_t TEST_PDM_SIZE = sizeof(TEST_PDM_DATA);

void setUp(void) {
    Serial.println("\nSetting up test...");
    
    // Print initial heap info
    Serial.printf("Free heap: %d, Largest block: %d\n", 
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    
    // Create new instances before each test
    audioInput = new AudioInput();
    pdmProc = new PDMProcessing();
    i2sConfig = new I2SConfig();
}

void tearDown(void) {
    Serial.println("Tearing down test...");
    
    // Print heap info before cleanup
    Serial.printf("Pre-cleanup heap - Free: %d, Largest block: %d\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    
    if (audioInput != nullptr) {
        Serial.println("Cleaning up audioInput...");
        audioInput->stopRecording();
        delay(100);
        audioInput->deinit();
        delete audioInput;
        audioInput = nullptr;
    }
    
    if (pdmProc != nullptr) {
        Serial.println("Cleaning up pdmProc...");
        pdmProc->deinit();
        delete pdmProc;
        pdmProc = nullptr;
    }
    
    if (i2sConfig != nullptr) {
        Serial.println("Cleaning up i2sConfig...");
        i2sConfig->deinit();
        delete i2sConfig;
        i2sConfig = nullptr;
    }
    
    // Print final heap info
    Serial.printf("Post-cleanup heap - Free: %d, Largest block: %d\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        
    delay(100);  // Give time for cleanup
}

void test_i2s_initialization() {
    Serial.println("Testing I2S initialization...");
    TEST_ASSERT_NOT_NULL(i2sConfig);
    bool init_result = i2sConfig->init();
    TEST_ASSERT_TRUE(init_result);
}

void test_i2s_pin_configuration() {
    Serial.println("Testing I2S pin configuration...");
    TEST_ASSERT_TRUE(i2sConfig->init());
    
    // Test reading samples to verify pin configuration works
    int16_t buffer[TEST_BUFFER_SIZE];
    size_t bytes_read = 0;
    bool read_result = i2sConfig->readSamples(buffer, TEST_BUFFER_SIZE, &bytes_read);
    TEST_ASSERT_TRUE(read_result);
    TEST_ASSERT_GREATER_THAN(0, bytes_read);
    
    Serial.println("I2S pin configuration verified through sample reading");
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
    
    Serial.printf("Read %d bytes from I2S\n", bytes_read);
}

void test_i2s_audio_level() {
    Serial.println("Testing I2S audio level calculation...");
    TEST_ASSERT_TRUE(i2sConfig->init());
    
    // Read some samples
    int16_t buffer[TEST_BUFFER_SIZE];
    size_t bytes_read = 0;
    TEST_ASSERT_TRUE(i2sConfig->readSamples(buffer, TEST_BUFFER_SIZE, &bytes_read));
    
    // Calculate audio level
    float level = i2sConfig->calculateAudioLevel(buffer, bytes_read / sizeof(int16_t));
    TEST_ASSERT_GREATER_OR_EQUAL(-100.0f, level);
    
    Serial.printf("I2S audio level: %.2f dB\n", level);
}

void test_pdm_processing_initialization() {
    Serial.println("Testing PDM processing initialization...");
    TEST_ASSERT_NOT_NULL(pdmProc);
    bool init_result = pdmProc->init(TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR);
    TEST_ASSERT_TRUE(init_result);
}

void test_pdm_to_pcm_conversion() {
    Serial.println("Testing PDM to PCM conversion...");
    TEST_ASSERT_TRUE(pdmProc->init(TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR));
    
    // Prepare output buffer
    int16_t pcm_buffer[TEST_BUFFER_SIZE];
    size_t pcm_size = 0;
    
    // Convert PDM to PCM
    bool convert_result = pdmProc->convertPDMtoPCM(TEST_PDM_DATA, TEST_PDM_SIZE, pcm_buffer, &pcm_size);
    TEST_ASSERT_TRUE(convert_result);
    TEST_ASSERT_GREATER_THAN(0, pcm_size);
    
    // Verify PCM data characteristics
    bool has_non_zero = false;
    for (size_t i = 0; i < pcm_size; i++) {
        if (pcm_buffer[i] != 0) {
            has_non_zero = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_non_zero);
    
    Serial.printf("Converted %d PDM bytes to %d PCM samples\n", TEST_PDM_SIZE, pcm_size);
}

void test_pdm_filtering() {
    Serial.println("Testing PDM filtering...");
    TEST_ASSERT_TRUE(pdmProc->init(TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR));
    
    // Create test PCM buffer with known pattern
    int16_t pcm_buffer[TEST_BUFFER_SIZE];
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        pcm_buffer[i] = (i % 2 == 0) ? 1000 : -1000; // Alternating pattern
    }
    
    // Apply filter
    bool filter_result = pdmProc->applyFilter(pcm_buffer, TEST_BUFFER_SIZE);
    TEST_ASSERT_TRUE(filter_result);
    
    // Verify filtering smoothed the signal
    bool is_smoothed = false;
    for (size_t i = 1; i < TEST_BUFFER_SIZE; i++) {
        if (abs(pcm_buffer[i] - pcm_buffer[i-1]) < 1000) {
            is_smoothed = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(is_smoothed);
}

void test_audio_input_pdm_integration() {
    Serial.println("Testing AudioInput PDM integration...");
    TEST_ASSERT_NOT_NULL(audioInput);
    
    Serial.printf("Pre-init heap - Free: %d, Largest: %d\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    
    bool init_result = audioInput->init(TEST_BUFFER_SIZE, TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR);
    TEST_ASSERT_TRUE(init_result);
    
    Serial.printf("Post-init heap - Free: %d, Largest: %d\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    
    TEST_ASSERT_TRUE(audioInput->startRecording());
    
    int16_t output_buffer[TEST_BUFFER_SIZE];
    size_t samples_read = 0;
    
    Serial.println("About to read audio data...");
    bool read_result = audioInput->readAudioData(output_buffer, TEST_BUFFER_SIZE, &samples_read);
    
    Serial.printf("Read result: %d, samples_read: %d\n", read_result, samples_read);
    Serial.printf("Post-read heap - Free: %d, Largest: %d\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    
    TEST_ASSERT_TRUE(read_result);
    TEST_ASSERT_GREATER_THAN(0, samples_read);
    
    Serial.println("About to stop recording...");
    TEST_ASSERT_TRUE(audioInput->stopRecording());
    
    Serial.printf("Final heap - Free: %d, Largest: %d\n",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

void test_audio_level_with_pdm() {
    Serial.println("Testing audio level calculation with PDM processing...");
    TEST_ASSERT_TRUE(audioInput->init(TEST_BUFFER_SIZE, TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR));
    TEST_ASSERT_TRUE(audioInput->startRecording());
    
    float level = audioInput->getAudioLevel();
    TEST_ASSERT_GREATER_OR_EQUAL(-100.0f, level);
    
    Serial.printf("Audio level: %.2f dB\n", level);
}

void test_real_pdm_microphone() {
    Serial.println("\nTesting with real PDM microphone using ESP-DSP...");
    TEST_ASSERT_NOT_NULL(audioInput);
    
    // Initialize with PDM parameters
    bool init_result = audioInput->init(TEST_BUFFER_SIZE, TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR);
    TEST_ASSERT_TRUE(init_result);
    
    // Start recording
    TEST_ASSERT_TRUE(audioInput->startRecording());
    
    // Read multiple buffers to ensure we get real data
    int16_t output_buffer[TEST_BUFFER_SIZE];
    size_t samples_read = 0;
    bool has_valid_data = false;
    
    // Try reading multiple times to ensure we get valid data
    for (int attempt = 0; attempt < 5; attempt++) {
        // Clear the buffer before each read
        memset(output_buffer, 0, sizeof(int16_t) * TEST_BUFFER_SIZE);
        samples_read = 0;
        
        bool read_result = audioInput->readAudioData(output_buffer, TEST_BUFFER_SIZE, &samples_read);
        
        // Print debug info regardless of result
        Serial.printf("Read attempt %d: result=%d, samples_read=%d\n", 
                     attempt + 1, read_result, samples_read);
        
        if (!read_result) {
            Serial.println("Read failed, trying again...");
            delay(200);  // Longer delay to allow system to recover
            continue;
        }
        
        TEST_ASSERT_GREATER_THAN(0, samples_read);
        
        // Check for non-zero data
        for (size_t i = 0; i < samples_read; i++) {
            if (output_buffer[i] != 0) {
                has_valid_data = true;
                break;
            }
        }
        
        if (has_valid_data) {
            Serial.printf("Successfully read real PDM data on attempt %d\n", attempt + 1);
            Serial.printf("Samples read: %d\n", samples_read);
            
            // Print some sample values for debugging
            Serial.println("Sample values:");
            for (size_t i = 0; i < 5 && i < samples_read; i++) {
                Serial.printf("Sample[%d]: %d\n", i, output_buffer[i]);
            }
            break;
        } else {
            Serial.println("Read succeeded but no non-zero data found, trying again...");
        }
        
        delay(100); // Wait a bit before next attempt
    }
    
    // Clean up even if test fails
    bool stop_result = audioInput->stopRecording();
    
    // Now assert that we found valid data
    TEST_ASSERT_TRUE(has_valid_data);
    
    // Only test audio level if we successfully got data
    if (has_valid_data) {
        // Start recording again for audio level test
        TEST_ASSERT_TRUE(audioInput->startRecording());
        
        float level = audioInput->getAudioLevel();
        TEST_ASSERT_GREATER_OR_EQUAL(-100.0f, level);
        Serial.printf("Real microphone audio level: %.2f dB\n", level);
        
        // Stop recording again
        TEST_ASSERT_TRUE(audioInput->stopRecording());
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting Audio Processing Tests ===\n");
    UNITY_BEGIN();
    
    // First run I2S configuration tests
    Serial.println("\n--- I2S Configuration Tests ---");
    RUN_TEST(test_i2s_initialization);
    RUN_TEST(test_i2s_pin_configuration);
    RUN_TEST(test_i2s_sample_reading);
    RUN_TEST(test_i2s_audio_level);
    
    // Then run PDM processing tests
    Serial.println("\n--- PDM Processing Tests ---");
    RUN_TEST(test_pdm_processing_initialization);
    RUN_TEST(test_pdm_to_pcm_conversion);
    RUN_TEST(test_pdm_filtering);
    RUN_TEST(test_audio_input_pdm_integration);
    RUN_TEST(test_audio_level_with_pdm);
    
    // Finally run real microphone test
    Serial.println("\n--- Real Microphone Test ---");
    RUN_TEST(test_real_pdm_microphone);
    
    int result = UNITY_END();
    Serial.printf("\n=== Test Results: %d tests completed ===\n", result);
}

void loop() {
    // Empty loop
}
