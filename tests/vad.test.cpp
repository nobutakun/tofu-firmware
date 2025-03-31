#include <Arduino.h>
#include <unity.h>
#include "../library/vad.h"

void test_vad_detects_speech() {
    VoiceActivityDetector vad(1000.0f, 160);
    std::vector<int16_t> speech_frame(160, 100); // Simulated speech frame
    bool result = vad.isSpeech(speech_frame);
    TEST_ASSERT_TRUE(result);
    Serial.println(result ? "test_vad_detects_speech: SUCCESS" : "test_vad_detects_speech: FAIL");
}

void test_vad_detects_silence() {
    VoiceActivityDetector vad(1000.0f, 160);
    std::vector<int16_t> silence_frame(160, 0); // Simulated silence frame
    bool result = vad.isSpeech(silence_frame);
    TEST_ASSERT_FALSE(result);
    Serial.println(!result ? "test_vad_detects_silence: SUCCESS" : "test_vad_detects_silence: FAIL");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting VAD Tests ===\n");
    UNITY_BEGIN();
    RUN_TEST(test_vad_detects_speech);
    RUN_TEST(test_vad_detects_silence);
    UNITY_END();
}

void loop() {
    // Empty loop
} 