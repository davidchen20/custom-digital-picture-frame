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

void frameWorkerTask(void *pvParameters) {
  // Initialize frame inside the dedicated task frame context
  if (!frame.begin(WIFI_SSID, WIFI_PASS)) {
    Serial.println("PhotoFrame initialization failed!");
  }

  // Infinite processing loop
  while (true) {
    frame.update();
    vTaskDelay(pdMS_TO_TICKS(10)); // Yield 10ms to feed the Task Watchdog
  }
}

void setup() {
  Serial.begin(115200);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) delay(10);

  // make an RTOS task with a bigger stack
  xTaskCreatePinnedToCore(
    frameWorkerTask,   // Task function
    "FrameTask",       // Task name
    32768,             // Stack size in bytes (32 KB - 4x loopTask default!)
    NULL,              // Task parameters
    1,                 // Priority
    NULL,              // Task handle
    1                  // Run on App CPU (Core 1)
  );
}

void loop() {
  // apparently this is best practice
  vTaskDelay(pdMS_TO_TICKS(1000));
}