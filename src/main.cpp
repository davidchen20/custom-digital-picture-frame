#include <Arduino.h>
#include "env.h"

#include "DisplayManager.h"
#include "StorageManager.h"
#include "PhotoFrame.h"

// Hardware Pin Definitions
#define TFT_BL 2
#define SD_CS  10

// Instance Instantiation
DisplayManager display(TFT_BL);
StorageManager storage(VERCEL_API, SD_CS);
PhotoFrame frame(display, storage, 10000 /* ms */, false /* shuffle */);

void setup() {
  Serial.begin(115200);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) delay(10);

  if (!frame.begin(WIFI_SSID, WIFI_PASS)) {
    Serial.println("PhotoFrame initialization failed!");
  }
}

void loop() {
  frame.update(); // Non-blocking state update
}