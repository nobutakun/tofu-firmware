#include <Arduino.h>
#include <unity.h>
#include "../library/sd_config.h"
#include <vector>

void test_sd_card_initialization() {
    Serial.println("Testing SD card initialization...");
    
    // Reset pins before test
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    delay(1000);
    
    // Test CS pin toggling
    Serial.println("Testing CS pin...");
    digitalWrite(SD_CS_PIN, LOW);
    delay(100);
    TEST_ASSERT_EQUAL(LOW, digitalRead(SD_CS_PIN));
    digitalWrite(SD_CS_PIN, HIGH);
    delay(100);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(SD_CS_PIN));
    
    // Initialize SD card
    bool initResult = SDConfig::begin();
    TEST_ASSERT_TRUE(initResult);
    
    // Verify card is actually mounted
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        TEST_FAIL_MESSAGE("SD card initialization returned true but card is not mounted!");
        return;
    }
    
    // Try a simple file operation to verify mounting
    File testFile = SD.open("/init_test.txt", FILE_WRITE);
    if (!testFile) {
        TEST_FAIL_MESSAGE("SD card mounted but file creation failed!");
        return;
    }
    testFile.println("Test");
    testFile.close();
    SD.remove("/init_test.txt");
    
    Serial.println("SD card successfully initialized and mounted!");
}

void test_file_write_read() {
    Serial.println("\nTesting file write and read operations...");
    
    // First verify card is mounted
    if(SD.cardType() == CARD_NONE) {
        TEST_FAIL_MESSAGE("SD card not mounted!");
        return;
    }
    
    // Create test data
    const char* test_file = "/test.txt";
    const char* test_data = "Hello, SD Card!";
    
    // Write test file
    File file = SD.open(test_file, FILE_WRITE);
    if (!file) {
        TEST_FAIL_MESSAGE("Failed to open file for writing");
        return;
    }
    
    size_t bytesWritten = file.print(test_data);
    file.close();
    
    TEST_ASSERT_EQUAL(strlen(test_data), bytesWritten);
    
    // Verify file exists
    if (!SDConfig::exists(test_file)) {
        TEST_FAIL_MESSAGE("File does not exist after writing");
        return;
    }
    
    // Read and verify content
    file = SD.open(test_file);
    if (!file) {
        TEST_FAIL_MESSAGE("Failed to open file for reading");
        return;
    }
    
    String content = file.readString();
    file.close();
    
    TEST_ASSERT_EQUAL_STRING(test_data, content.c_str());
    
    // Clean up
    if (!SD.remove(test_file)) {
        Serial.println("Warning: Failed to remove test file");
    }
}

void test_wav_file_operations() {
    Serial.println("\nTesting WAV file operations...");
    
    // First verify card is mounted
    if(SD.cardType() == CARD_NONE) {
        TEST_FAIL_MESSAGE("SD card not mounted!");
        return;
    }
    
    // Create sample WAV data
    uint8_t wav_header[] = {
        'R', 'I', 'F', 'F',  // RIFF header
        0x24, 0x00, 0x00, 0x00,  // File size (36 bytes)
        'W', 'A', 'V', 'E',  // WAVE header
        'f', 'm', 't', ' '   // fmt chunk
    };
    
    const char* wav_file = "/test.wav";
    
    // Test writing WAV file
    File file = SD.open(wav_file, FILE_WRITE);
    if (!file) {
        TEST_FAIL_MESSAGE("Failed to open WAV file for writing");
        return;
    }
    
    size_t bytesWritten = file.write(wav_header, sizeof(wav_header));
    file.close();
    
    TEST_ASSERT_EQUAL(sizeof(wav_header), bytesWritten);
    
    // Verify file exists and size
    TEST_ASSERT_TRUE(SDConfig::exists(wav_file));
    TEST_ASSERT_EQUAL(sizeof(wav_header), SDConfig::getFileSize(wav_file));
    
    // Clean up
    if (!SD.remove(wav_file)) {
        Serial.println("Warning: Failed to remove WAV file");
    }
}

void test_sd_card_cleanup() {
    Serial.println("\nTesting SD card cleanup...");
    SDConfig::end();
    TEST_PASS();
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting SD Card Tests ===\n");
    UNITY_BEGIN();
    
    RUN_TEST(test_sd_card_initialization);
    delay(500);  // Add delay between tests
    
    RUN_TEST(test_file_write_read);
    delay(500);
    
    RUN_TEST(test_wav_file_operations);
    delay(500);
    
    RUN_TEST(test_sd_card_cleanup);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}
