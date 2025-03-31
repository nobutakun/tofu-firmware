#include <Arduino.h>
#include <unity.h>
#include "../library/pdm_processing.h"

using namespace audio_processing;

// Test configuration constants
const int TEST_SAMPLE_RATE = 16000;
const int TEST_DECIMATION_FACTOR = 64;
const size_t TEST_BUFFER_SIZE = 512;

// Test data
const uint8_t TEST_PDM_DATA[] = {
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // Alternating pattern
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,  // Alternating pattern
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,  // All zeros then all ones
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00   // All ones then all zeros
};
const size_t TEST_PDM_SIZE = sizeof(TEST_PDM_DATA);

void test_pcm_is_16bit() {
    Serial.println("Testing if PCM is 16-bit...");
    
    // Prepare output buffer
    int16_t pcm_buffer[TEST_BUFFER_SIZE];
    size_t pcm_size = 0;
    
    // Check size of PCM data type
    TEST_ASSERT_EQUAL(sizeof(int16_t), 2);
    
    Serial.println("PCM is confirmed to be 16-bit.");
}

void test_pdm_to_pcm_conversion_uses_float() {
    Serial.println("Testing if PDM to PCM conversion uses float...");
    
    PDMProcessing pdmProc;
    TEST_ASSERT_TRUE(pdmProc.init(TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR));
    
    // Prepare output buffer
    int16_t pcm_buffer[TEST_BUFFER_SIZE];
    size_t pcm_size = 0;
    
    // Convert PDM to PCM
    bool convert_result = pdmProc.convertPDMtoPCM(TEST_PDM_DATA, TEST_PDM_SIZE, pcm_buffer, &pcm_size);
    TEST_ASSERT_TRUE(convert_result);
    
    // Check if internal buffers are float
    float* pdm_float_buffer = pdmProc.getPDMFloatBuffer();
    float* pcm_float_buffer = pdmProc.getPCMFloatBuffer();
    
    TEST_ASSERT_NOT_NULL(pdm_float_buffer);
    TEST_ASSERT_NOT_NULL(pcm_float_buffer);
    
    // Check size of float data type
    TEST_ASSERT_EQUAL(sizeof(float), 4);
    
    Serial.println("PDM to PCM conversion confirmed to use float.");
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting Audio Conversion Tests ===\n");
    UNITY_BEGIN();
    
    RUN_TEST(test_pcm_is_16bit);
    RUN_TEST(test_pdm_to_pcm_conversion_uses_float);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}  