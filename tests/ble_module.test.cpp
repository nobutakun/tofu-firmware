#include <Arduino.h>
#include <unity.h>
#include "../library/bluetooth_manager.h"

// Test instance
BluetoothManager* btManager = nullptr;

// Test data
const char* TEST_DEVICE_NAME = "XiaoESP32Test";
const uint8_t TEST_DATA[] = {0x01, 0x02, 0x03, 0x04, 0x05};
const size_t TEST_DATA_SIZE = sizeof(TEST_DATA);
const String TEST_STRING = "Hello from ESP32";

// Callback flags
bool dataReceivedFlag = false;
bool connectionChangedFlag = false;
bool connectionStatus = false;

// Callback functions
void onDataReceived(const uint8_t* data, size_t length) {
    dataReceivedFlag = true;
    Serial.printf("Data received callback triggered with %d bytes\n", length);
    
    // Print first few bytes for verification
    Serial.print("Data: ");
    for (size_t i = 0; i < min(length, (size_t)10); i++) {
        Serial.printf("%02X ", data[i]);
    }
    Serial.println();
}

void onConnectionChanged(bool connected) {
    connectionChangedFlag = true;
    connectionStatus = connected;
    Serial.printf("Connection changed to: %s\n", connected ? "Connected" : "Disconnected");
}

void setUp(void) {
    Serial.println("\nSetting up test...");
    
    // Reset flags
    dataReceivedFlag = false;
    connectionChangedFlag = false;
    connectionStatus = false;
    
    // Create new instance before each test
    btManager = new BluetoothManager();
}

void tearDown(void) {
    Serial.println("Tearing down test...");
    
    if (btManager != nullptr) {
        btManager->deinit();
        delete btManager;
        btManager = nullptr;
    }
    
    delay(100);  // Give time for cleanup
}

void test_bluetooth_initialization() {
    Serial.println("Testing Bluetooth initialization...");
    TEST_ASSERT_NOT_NULL(btManager);
    
    bool init_result = btManager->init(TEST_DEVICE_NAME);
    TEST_ASSERT_TRUE(init_result);
    
    // Verify device name was set correctly
    // Note: We can't directly check the name, but we can verify initialization succeeded
    Serial.println("Bluetooth initialized successfully");
    
    // Deinitialize for next test
    btManager->deinit();
}

void test_connection_status() {
    Serial.println("Testing connection status...");
    TEST_ASSERT_TRUE(btManager->init(TEST_DEVICE_NAME));
    
    // Initially should not be connected
    TEST_ASSERT_FALSE(btManager->isConnected());
    
    // Set up connection callback
    btManager->setConnectionCallback(onConnectionChanged);
    
    // Manual verification step
    Serial.println("\n*** MANUAL TEST STEP ***");
    Serial.println("Please connect to the Bluetooth device named: " + String(TEST_DEVICE_NAME));
    Serial.println("You have 30 seconds to connect...");
    
    // Wait for connection or timeout
    int timeout = 30; // 30 seconds
    while (timeout > 0 && !btManager->isConnected()) {
        btManager->update(); // Process events
        Serial.printf("Waiting for connection... %d seconds remaining\n", timeout);
        timeout--;
        delay(1000);
    }
    
    if (btManager->isConnected()) {
        Serial.println("Device connected!");
        TEST_ASSERT_TRUE(btManager->isConnected());
        TEST_ASSERT_TRUE(connectionChangedFlag);
        TEST_ASSERT_TRUE(connectionStatus);
    } else {
        Serial.println("No device connected within timeout period.");
        // Skip this test if no connection
        TEST_IGNORE();
    }
}

void test_send_receive_data() {
    Serial.println("Testing data sending and receiving...");
    TEST_ASSERT_TRUE(btManager->init(TEST_DEVICE_NAME));
    
    // Set up data callback
    btManager->setDataReceivedCallback(onDataReceived);
    
    // Manual verification step for connection
    Serial.println("\n*** MANUAL TEST STEP ***");
    Serial.println("Please connect to the Bluetooth device named: " + String(TEST_DEVICE_NAME));
    Serial.println("You have 30 seconds to connect...");
    
    // Wait for connection or timeout
    int timeout = 30; // 30 seconds
    while (timeout > 0 && !btManager->isConnected()) {
        btManager->update();
        timeout--;
        delay(1000);
    }
    
    if (!btManager->isConnected()) {
        Serial.println("No device connected within timeout period.");
        TEST_IGNORE();
        return;
    }
    
    // Test sending data
    Serial.println("Sending test data...");
    bool send_result = btManager->sendData(TEST_DATA, TEST_DATA_SIZE);
    TEST_ASSERT_TRUE(send_result);
    
    // Test sending string
    Serial.println("Sending test string...");
    bool string_result = btManager->sendString(TEST_STRING);
    TEST_ASSERT_TRUE(string_result);
    
    // Manual verification step for receiving data
    Serial.println("\n*** MANUAL TEST STEP ***");
    Serial.println("Please send some data from your device to the ESP32.");
    Serial.println("You have 30 seconds to send data...");
    
    // Wait for data or timeout
    timeout = 30; // 30 seconds
    while (timeout > 0 && !dataReceivedFlag) {
        btManager->update(); // Process events
        timeout--;
        delay(1000);
    }
    
    if (dataReceivedFlag) {
        Serial.println("Data received successfully!");
        TEST_ASSERT_TRUE(dataReceivedFlag);
    } else {
        Serial.println("No data received within timeout period.");
        // Don't fail the test, just note that no data was received
        TEST_IGNORE_MESSAGE("No data received from connected device");
    }
}

void test_audio_streaming() {
    Serial.println("Testing audio streaming...");
    TEST_ASSERT_TRUE(btManager->init(TEST_DEVICE_NAME));
    
    // Manual verification step for connection
    Serial.println("\n*** MANUAL TEST STEP ***");
    Serial.println("Please connect to the Bluetooth device named: " + String(TEST_DEVICE_NAME));
    Serial.println("You have 30 seconds to connect...");
    
    // Wait for connection or timeout
    int timeout = 30; // 30 seconds
    while (timeout > 0 && !btManager->isConnected()) {
        btManager->update();
        timeout--;
        delay(1000);
    }
    
    if (!btManager->isConnected()) {
        Serial.println("No device connected within timeout period.");
        TEST_IGNORE();
        return;
    }
    
    // Start audio stream
    bool stream_result = btManager->startAudioStream(16000, 16);
    TEST_ASSERT_TRUE(stream_result);
    
    // Generate and send some test audio data
    const int SAMPLES = 320; // 20ms of audio at 16kHz
    int16_t testAudio[SAMPLES];
    
    // Generate a simple sine wave
    for (int i = 0; i < SAMPLES; i++) {
        testAudio[i] = (int16_t)(10000 * sin(2 * PI * 440 * i / 16000.0)); // 440Hz tone
    }
    
    // Send audio data
    Serial.println("Sending test audio data...");
    bool audio_result = btManager->sendAudioData(testAudio, SAMPLES);
    TEST_ASSERT_TRUE(audio_result);
    
    // Stop audio stream
    bool stop_result = btManager->stopAudioStream();
    TEST_ASSERT_TRUE(stop_result);
    
    Serial.println("Audio streaming test completed");
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    delay(2000);  // Allow serial to settle
    
    Serial.println("\n\n=== Starting Bluetooth Module Tests ===\n");
    UNITY_BEGIN();
    
    RUN_TEST(test_bluetooth_initialization);
    delay(500);
    
    RUN_TEST(test_connection_status);
    delay(500);
    
    RUN_TEST(test_send_receive_data);
    delay(500);
    
    RUN_TEST(test_audio_streaming);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}
