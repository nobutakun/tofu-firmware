#ifndef SD_CONFIG_H
#define SD_CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// *** SPI HOST CONFIGURATION ***
// Uncomment only ONE of these options to select the SPI host
#define USE_SPI_HOST_0   // SPI1_HOST/SPI_HOST (0)
//#define USE_SPI_HOST_1   // SPI2_HOST/HSPI (1)
//#define USE_SPI_HOST_2   // SPI3_HOST/VSPI (2)

// SD Card SPI pins for Xiao ESP32S3
#define SD_CS_PIN    44   // GPIO44 (Chip Select)
#define SD_SCK_PIN   7    // GPIO7 (Clock)
#define SD_MISO_PIN  8    // GPIO8 (MISO)
#define SD_MOSI_PIN  9    // GPIO9 (MOSI)

class SDConfig {
private:
    static SPIClass* _spi;

public:
    static bool begin() {
        Serial.println("Initializing SD card with SPI...");
        
        // Configure SPI pins
        pinMode(SD_CS_PIN, OUTPUT);
        digitalWrite(SD_CS_PIN, HIGH);  // Deselect SD card initially
        
        // Initialize SPI with selected host
        if (_spi == nullptr) {
            #if defined(USE_SPI_HOST_0)
            _spi = new SPIClass(0); // SPI1_HOST/SPI_HOST
            Serial.println("Using SPI Host 0 (SPI1_HOST/SPI_HOST)");
            #elif defined(USE_SPI_HOST_1)
            _spi = new SPIClass(1); // SPI2_HOST/HSPI
            Serial.println("Using SPI Host 1 (SPI2_HOST/HSPI)");
            #elif defined(USE_SPI_HOST_2)
            _spi = new SPIClass(2); // SPI3_HOST/VSPI
            Serial.println("Using SPI Host 2 (SPI3_HOST/VSPI)");
            #else
            #error "No SPI host selected! Uncomment one of the USE_SPI_HOST_x defines at the top of the file."
            #endif
        }
        
        _spi->begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
        
        // Send dummy clock cycles with CS high to initialize card
        digitalWrite(SD_CS_PIN, HIGH);
        for(int i = 0; i < 10; i++) {
            _spi->transfer(0xFF);
        }
        
        // Try to initialize SD card
        for (int retry = 0; retry < 5; retry++) {
            Serial.printf("Initialization attempt %d/5\n", retry+1);
            
            if (SD.begin(SD_CS_PIN, *_spi, 400000)) {  // Start with 400kHz
                uint8_t cardType = SD.cardType();
                if (cardType == CARD_NONE) {
                    Serial.println("No SD card detected");
                    SD.end();
                    delay(500);
                    continue;
                }
                
                // Card is working!
                Serial.printf("Card Type: %s\n", 
                    cardType == CARD_MMC ? "MMC" :
                    cardType == CARD_SD ? "SDSC" :
                    cardType == CARD_SDHC ? "SDHC" : "UNKNOWN");
                
                Serial.printf("Card Size: %lluMB\n", SD.cardSize()/(1024*1024));
                Serial.println("SD Card successfully mounted!");
                return true;
            }
            
            Serial.printf("Initialization attempt %d failed\n", retry+1);
            delay(1000);
        }
        
        Serial.println("SD Card initialization failed!");
        return false;
    }

    static void end() {
        SD.end();
        if (_spi != nullptr) {
            _spi->end();
        }
    }

    // Helper method to check if file exists
    static bool exists(const char* path) {
        if (!SD.exists(path)) {
            Serial.printf("File %s does not exist\n", path);
            return false;
        }
        return true;
    }

    // Helper method to get file size
    static size_t getFileSize(const char* path) {
        if (!exists(path)) {
            return 0;
        }
        File file = SD.open(path);
        if (!file) {
            Serial.printf("Failed to open file %s\n", path);
            return 0;
        }
        size_t size = file.size();
        file.close();
        return size;
    }

    // Helper method to write WAV data to SD card
    static bool writeWAVFile(const char* path, const uint8_t* wav_data, size_t wav_data_size) {
        if (!wav_data || wav_data_size == 0) {
            Serial.println("Invalid WAV data");
            return false;
        }

        File file = SD.open(path, FILE_WRITE);
        if (!file) {
            Serial.printf("Failed to open file %s for writing\n", path);
            return false;
        }

        size_t bytesWritten = file.write(wav_data, wav_data_size);
        file.close();

        if (bytesWritten != wav_data_size) {
            Serial.printf("Failed to write complete WAV data. Written: %d, Expected: %d\n", 
                         bytesWritten, wav_data_size);
            return false;
        }

        Serial.printf("Successfully wrote %d bytes to %s\n", bytesWritten, path);
        return true;
    }
};

// Initialize static member
SPIClass* SDConfig::_spi = nullptr;

#endif // SD_CONFIG_H
