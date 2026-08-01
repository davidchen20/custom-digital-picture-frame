#include <Arduino.h>
#include "DisplayManager.h"
#include "StorageManager.h"
#include "PhotoFrame.h"

// Hardware Pin Definitions
#define TFT_BL  2
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  12
#define SD_CS   10

// Instance Instantiation
DisplayManager display(TFT_BL);
StorageManager storage(SD_CS, SD_MOSI, SD_MISO, SD_SCK);
PhotoFrame frame(display, storage, 5000 /* ms */, true /* shuffle */);

void setup() {
  Serial.begin(115200);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) delay(10);

  if (!frame.begin()) {
    Serial.println("PhotoFrame initialization failed!");
  }
}

void loop() {
  frame.update(); // Non-blocking state update
}