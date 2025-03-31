#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <functional>
#include <vector>
#include <string>

class BluetoothManager {
public:
    // Callback types
    using DataReceivedCallback = std::function<void(const uint8_t*, size_t)>;
    using ConnectionCallback = std::function<void(bool)>;
    
    BluetoothManager();
    ~BluetoothManager();
    
    // Initialization
    bool init(const String& deviceName = "XiaoESP32");
    void deinit();
    
    // Connection management
    bool isConnected();
    void setConnectionCallback(ConnectionCallback callback);
    
    // Data transmission
    bool sendData(const uint8_t* data, size_t length);
    bool sendString(const String& message);
    void setDataReceivedCallback(DataReceivedCallback callback);
    
    // Audio streaming
    bool startAudioStream(int sampleRate = 16000, int bitsPerSample = 16);
    bool stopAudioStream();
    bool sendAudioData(const int16_t* audioData, size_t samples);
    
    // Utility functions
    void update(); // Call this in the main loop
    
private:
    BluetoothSerial _serialBT;
    String _deviceName;
    bool _initialized;
    bool _streaming;
    
    // Callbacks
    DataReceivedCallback _dataCallback;
    ConnectionCallback _connectionCallback;
    
    // Buffer for receiving data
    static const size_t RX_BUFFER_SIZE = 1024;
    uint8_t _rxBuffer[RX_BUFFER_SIZE];
    
    // Internal methods
    void handleReceivedData();
};

#endif // BLUETOOTH_MANAGER_H 