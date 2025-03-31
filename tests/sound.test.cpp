
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"

#define I2S_DOUT  D4
#define I2S_BCLK  D0
#define I2S_LRC   D7

const char* ssid = "Homy Home Linh Trung 5G";
const char* password = "homy@linhtrung";
const char* streamURL = "http://vis.media-ice.musicradio.com/CapitalMP3";

Audio audio;

void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(50);
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connecting to audio stream...");
    audio.connecttohost(streamURL);
  } else {
    Serial.println("Skipping audio connection due to WiFi failure.");
  }
}

void loop() {
  audio.loop();
}