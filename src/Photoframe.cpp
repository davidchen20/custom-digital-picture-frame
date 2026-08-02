#include "PhotoFrame.h"

PhotoFrame::PhotoFrame(DisplayManager &disp, StorageManager &stor, unsigned long interval, bool isShuffle)
  : display(disp), storage(stor), intervalMs(interval), shuffle(isShuffle), lastChangeTime(0), currentIndex(0) {}

bool PhotoFrame::begin(const char* ssid, const char* password) {
  display.begin(1);
  display.showMessage("Initializing Storage...");

  if (!storage.begin(ssid, password)) {
    display.showMessage("No Photos Found (Wi-Fi & SD Failed)", TFT_RED);
    return false;
  }

  if (storage.getMode() == StorageMode::CLOUD) {
    display.showMessage("Mode: Cloud Active", TFT_GREEN);
  } else {
    display.showMessage("Mode: Offline SD Card", TFT_YELLOW);
  }
  
  delay(1000);
  nextImage(); // Display first frame
  return true;
}

void PhotoFrame::update() {
  if (storage.getPhotoCount() == 0) return;

  if (millis() - lastChangeTime >= intervalMs) {
    storage.refreshPhotos(); // Refresh links/files if needed
    nextImage();
  }
}

void PhotoFrame::nextImage() {
  size_t count = storage.getPhotoCount();
  if (count == 0) return;

  if (shuffle) {
    currentIndex = random(0, count);
  } else {
    currentIndex = (currentIndex + 1) % count;
  }

  // Stream raw bytes directly to LovyanGFX
  storage.streamPhoto(currentIndex, [this](Stream* stream) {
    display.drawBmpStream(stream);
  });

  lastChangeTime = millis();
}