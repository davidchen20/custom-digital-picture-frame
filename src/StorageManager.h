#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <vector>

class StorageManager {
private:
  uint8_t pinCS, pinMOSI, pinMISO, pinSCK;
  SPIClass sdSPI;
  std::vector<String> fileList;

  void scanDirectory(File dir, const String& parentPath = "");

public:
  StorageManager(uint8_t cs, uint8_t mosi, uint8_t miso, uint8_t sck);
  
  bool begin(uint32_t frequency = 10000000);
  size_t indexImages();
  const std::vector<String>& getImages() const { return fileList; }
  fs::FS& getFS() { return SD; }
};

#endif