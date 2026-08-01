#include "PhotoFrame.h"

PhotoFrame::PhotoFrame(DisplayManager &disp, StorageManager &stor, unsigned long interval, bool isShuffle)
  : display(disp), storage(stor), intervalMs(interval), shuffle(isShuffle), lastChangeTime(0), currentIndex(0) {}

bool PhotoFrame::begin() {
  display.begin(1);
  display.showMessage("Mounting SD Card...");

  if (!storage.begin()) {
    display.showMessage("Failed to Mount SD!", TFT_RED);
    return false;
  }

  display.showMessage("Scanning SD for images...");
  size_t count = storage.indexImages();

  if (count == 0) {
    display.showMessage("No BMP images found!", TFT_RED);
    return false;
  }

  nextImage(); // Load initial frame
  return true;
}

void PhotoFrame::update() {
  const std::vector<String>& files = storage.getImages();
  if (files.empty()) return;

  if (millis() - lastChangeTime >= intervalMs) {
    nextImage();
  }
}

void PhotoFrame::nextImage() {
  const std::vector<String>& files = storage.getImages();
  if (files.empty()) return;

  if (shuffle) {
    currentIndex = random(0, files.size());
  } else {
    currentIndex = (currentIndex + 1) % files.size();
  }

  display.renderCenteredBMP(storage.getFS(), files[currentIndex].c_str());
  lastChangeTime = millis();
}