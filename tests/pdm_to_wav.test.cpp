#include <Arduino.h>
#include <unity.h>
#include "../library/pdm_processing.h"
#include <vector>

using namespace audio_processing;

// Test data - make sure it's large enough for the decimation factor
const uint8_t TEST_PDM_DATA[] = {
    // 256 bytes of test data (enough for at least one chunk)
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // 8 bytes pattern
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    // ... repeat this pattern 32 times to get 256 bytes
};
const size_t TEST_PDM_SIZE = 256; // Use a full chunk size

void test_pdm_to_wav_conversion() {
    Serial.println("Starting PDM to WAV conversion test...");

    PDMProcessing pdmProc;
    
    Serial.println("Initializing PDM Processing...");
    if (!pdmProc.init(16000, 64)) {
        Serial.println("Failed to initialize PDMProcessing.");
        TEST_FAIL();
        return;
    }
    Serial.println("PDM Processing initialized successfully.");

    std::vector<uint8_t> wav_data;
    Serial.printf("Converting PDM data (size: %d bytes)...\n", TEST_PDM_SIZE);
    
    bool result = pdmProc.convertPDMtoWAV(TEST_PDM_DATA, TEST_PDM_SIZE, wav_data);
    if (!result) {
        Serial.println("convertPDMtoWAV failed.");
        TEST_FAIL();
        return;
    }
    
    Serial.printf("WAV data size: %d bytes\n", wav_data.size());

    // Verify WAV header
    if (wav_data.size() < 44) {
        Serial.println("WAV data too small!");
        TEST_FAIL();
        return;
    }

    TEST_ASSERT_EQUAL('R', wav_data[0]);
    TEST_ASSERT_EQUAL('I', wav_data[1]);
    TEST_ASSERT_EQUAL('F', wav_data[2]);
    TEST_ASSERT_EQUAL('F', wav_data[3]);
    TEST_ASSERT_EQUAL('W', wav_data[8]);
    TEST_ASSERT_EQUAL('A', wav_data[9]);
    TEST_ASSERT_EQUAL('V', wav_data[10]);
    TEST_ASSERT_EQUAL('E', wav_data[11]);

    TEST_ASSERT_EQUAL('d', wav_data[36]);
    TEST_ASSERT_EQUAL('a', wav_data[37]);
    TEST_ASSERT_EQUAL('t', wav_data[38]);
    TEST_ASSERT_EQUAL('a', wav_data[39]);

    Serial.println("PDM to WAV conversion test passed.");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting PDM to WAV Tests ===\n");
    UNITY_BEGIN();
    RUN_TEST(test_pdm_to_wav_conversion);
    UNITY_END();
}

void loop() {
    // Empty loop
}
