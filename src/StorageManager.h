#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <vector>
#include <memory>

enum class StorageMode {
    CLOUD,
    SD_CARD,
    NONE
};

class StorageManager {
private:
    const char* apiUrl;
    int sdCsPin;
    StorageMode storageMode;

    std::vector<String> photoList;

  bool scanSdCard();

public:
  StorageManager(const char* apiUrl, int sdCsPin = 10);
  
  bool begin(const char* ssid, const char* password);
  bool refreshPhotos();

  StorageMode getMode() const { return storageMode; }
  size_t getPhotoCount() const;

  bool streamPhoto(size_t index, std::function<void(Stream*)> onDataReady);
};

#endif