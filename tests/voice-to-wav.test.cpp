#include <Arduino.h>
#include <unity.h>
#include "../library/audio_input.h"
#include "../library/pdm_processing.h"
#include "driver/i2s.h"
#include "esp_heap_caps.h"

using namespace audio_processing;

// Test instances
AudioInput* audioInput = nullptr;
PDMProcessing* pdmProc = nullptr;

// Test configuration constants
const int TEST_SAMPLE_RATE = 16000;
const int TEST_DECIMATION_FACTOR = 64;
const size_t TEST_BUFFER_SIZE = 512;
const int RECORD_DURATION_MS = 1000; // 1 second recording

void setUp(void) {
    Serial.println("\nSetting up test...");
    
    // Create new instances before each test
    audioInput = new AudioInput();
    pdmProc = new PDMProcessing();
}

void tearDown(void) {
    Serial.println("Tearing down test...");
    
    if (audioInput != nullptr) {
        audioInput->stopRecording();
        audioInput->deinit();
        delete audioInput;
        audioInput = nullptr;
    }
    
    if (pdmProc != nullptr) {
        pdmProc->deinit();
        delete pdmProc;
        pdmProc = nullptr;
    }
}

void test_pdm_to_wav_conversion() {
    Serial.println("Testing PDM to WAV conversion with real microphone...");
    
    // Initialize audio input
    TEST_ASSERT_TRUE(audioInput->init(TEST_BUFFER_SIZE, TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR));
    
    // Initialize PDM processing
    TEST_ASSERT_TRUE(pdmProc->init(TEST_SAMPLE_RATE, TEST_DECIMATION_FACTOR));
    
    // Start recording
    TEST_ASSERT_TRUE(audioInput->startRecording());
    
    // Buffer for audio samples
    std::vector<int16_t> pcm_buffer(TEST_BUFFER_SIZE);
    std::vector<uint8_t> wav_data;
    std::vector<uint8_t> complete_pcm_data;
    
    // Record for specified duration
    unsigned long start_time = millis();
    size_t total_samples = 0;
    
    Serial.println("Recording audio...");
    while (millis() - start_time < RECORD_DURATION_MS) {
        size_t samples_read = 0;
        if (audioInput->readAudioData(pcm_buffer.data(), TEST_BUFFER_SIZE, &samples_read)) {
            // Append PCM data to complete buffer
            complete_pcm_data.insert(complete_pcm_data.end(), 
                                   (uint8_t*)pcm_buffer.data(),
                                   (uint8_t*)(pcm_buffer.data() + samples_read));
            total_samples += samples_read;
        }
        delay(10); // Small delay to prevent watchdog triggers
    }
    
    // Stop recording
    TEST_ASSERT_TRUE(audioInput->stopRecording());
    
    Serial.printf("Recorded %d total samples\n", total_samples);
    TEST_ASSERT_GREATER_THAN(0, total_samples);
    
    // Convert complete PCM data to WAV
    TEST_ASSERT_TRUE(pdmProc->convertPDMtoWAV(complete_pcm_data.data(), 
                                             complete_pcm_data.size(), 
                                             wav_data));
    
    // Verify WAV format
    TEST_ASSERT_GREATER_OR_EQUAL(44, wav_data.size()); // Minimum WAV header size
    
    // Check WAV header
    TEST_ASSERT_EQUAL('R', wav_data[0]);
    TEST_ASSERT_EQUAL('I', wav_data[1]);
    TEST_ASSERT_EQUAL('F', wav_data[2]);
    TEST_ASSERT_EQUAL('F', wav_data[3]);
    
    TEST_ASSERT_EQUAL('W', wav_data[8]);
    TEST_ASSERT_EQUAL('A', wav_data[9]);
    TEST_ASSERT_EQUAL('V', wav_data[10]);
    TEST_ASSERT_EQUAL('E', wav_data[11]);
    
    // Verify data chunk
    TEST_ASSERT_EQUAL('d', wav_data[36]);
    TEST_ASSERT_EQUAL('a', wav_data[37]);
    TEST_ASSERT_EQUAL('t', wav_data[38]);
    TEST_ASSERT_EQUAL('a', wav_data[39]);
    
    Serial.printf("WAV file size: %d bytes\n", wav_data.size());
    Serial.println("PDM to WAV conversion test completed successfully");
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting Voice-to-Text Tests ===\n");
    UNITY_BEGIN();
    RUN_TEST(test_pdm_to_wav_conversion);
    UNITY_END();
}

void loop() {
    // Empty loop
}
