#include "StorageManager.h"

StorageManager::StorageManager(uint8_t cs, uint8_t mosi, uint8_t miso, uint8_t sck)
  : pinCS(cs), pinMOSI(mosi), pinMISO(miso), pinSCK(sck), sdSPI(HSPI) {}

bool StorageManager::begin(uint32_t frequency) {
  pinMode(pinCS, OUTPUT);
  digitalWrite(pinCS, HIGH);
  pinMode(pinMISO, INPUT_PULLUP);

  sdSPI.begin(pinSCK, pinMISO, pinMOSI, pinCS);
  delay(100);

  if (!SD.begin(pinCS, sdSPI, frequency)) return false;
  return (SD.cardType() != CARD_NONE);
}

size_t StorageManager::indexImages() {
  fileList.clear();
  File root = SD.open("/");
  scanDirectory(root);
  root.close();
  return fileList.size();
}

void StorageManager::scanDirectory(File dir, const String& parentPath) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    String fileName = entry.name();
    String fullPath = parentPath + "/" + fileName;

    if (entry.isDirectory()) {
      if (!fileName.startsWith(".")) {
        scanDirectory(entry, fullPath);
      }
    } else {
      String lowerName = fileName;
      lowerName.toLowerCase();
      if (lowerName.endsWith(".bmp")) {
        fileList.push_back(fullPath);
      }
    }
    entry.close();
  }
}