#include "../library/bluetooth_manager.h"

BluetoothManager::BluetoothManager() 
    : _initialized(false), _streaming(false), _dataCallback(nullptr), _connectionCallback(nullptr) {
}

BluetoothManager::~BluetoothManager() {
    deinit();
}

bool BluetoothManager::init(const String& deviceName) {
    if (_initialized) {
        return true;
    }
    
    _deviceName = deviceName;
    
    // Initialize Bluetooth Serial
    if (!_serialBT.begin(_deviceName)) {
        Serial.println("Failed to initialize Bluetooth");
        return false;
    }
    
    Serial.println("Bluetooth initialized. Device name: " + _deviceName);
    _initialized = true;
    // Set up callbacks for connection events
    if (_serialBT.connected()) {
        Serial.println("Bluetooth device connected");
        if (_connectionCallback) {
            _connectionCallback(true);
        }
    if (_serialBT.hasClient()) {
        Serial.println("Bluetooth device disconnected");
        if (_connectionCallback) {
            _connectionCallback(false);
        }
    }
    
    return true;
}

void BluetoothManager::deinit() {
    if (_initialized) {
        _serialBT.end();
        _initialized = false;
        _streaming = false;
    }
}

bool BluetoothManager::isConnected() {
    return _initialized && _serialBT.connected();
}

void BluetoothManager::setConnectionCallback(ConnectionCallback callback) {
    _connectionCallback = callback;
}

bool BluetoothManager::sendData(const uint8_t* data, size_t length) {
    if (!isConnected()) {
        return false;
    }
    
    size_t written = _serialBT.write(data, length);
    return (written == length);
}

bool BluetoothManager::sendString(const String& message) {
    if (!isConnected()) {
        return false;
    }
    
    return _serialBT.println(message);
}

void BluetoothManager::setDataReceivedCallback(DataReceivedCallback callback) {
    _dataCallback = callback;
}

bool BluetoothManager::startAudioStream(int sampleRate, int bitsPerSample) {
    if (!isConnected()) {
        return false;
    }
    
    // Send stream start command with format info
    String startCmd = "START_AUDIO:" + String(sampleRate) + ":" + String(bitsPerSample);
    if (!sendString(startCmd)) {
        return false;
    }
    
    _streaming = true;
    return true;
}

bool BluetoothManager::stopAudioStream() {
    if (!isConnected() || !_streaming) {
        return false;
    }
    
    // Send stream stop command
    if (!sendString("STOP_AUDIO")) {
        return false;
    }
    
    _streaming = false;
    return true;
}

bool BluetoothManager::sendAudioData(const int16_t* audioData, size_t samples) {
    if (!isConnected() || !_streaming) {
        return false;
    }
    
    // Send audio data
    return sendData((const uint8_t*)audioData, samples * sizeof(int16_t));
}

void BluetoothManager::update() {
    if (!_initialized) {
        return;
    }
    
    // Check for incoming data
    if (_serialBT.available()) {
        handleReceivedData();
    }
}

void BluetoothManager::handleReceivedData() {
    size_t bytesRead = 0;
    
    // Read available data into buffer
    while (_serialBT.available() && bytesRead < RX_BUFFER_SIZE) {
        _rxBuffer[bytesRead++] = _serialBT.read();
    }
    
    // If we have a callback and received data, call it
    if (_dataCallback && bytesRead > 0) {
        _dataCallback(_rxBuffer, bytesRead);
    }
} 