#ifndef PHOTO_FRAME_H
#define PHOTO_FRAME_H

#include "DisplayManager.h"
#include "StorageManager.h"

class PhotoFrame {
private:
  DisplayManager &display;
  StorageManager &storage;
  
  unsigned long intervalMs;
  bool shuffle;
  unsigned long lastChangeTime;
  size_t currentIndex;

public:
  PhotoFrame(DisplayManager &disp, StorageManager &stor, unsigned long interval = 5000, bool isShuffle = true);

  bool begin(const char* ssid, const char* password);
  void update();
  void nextImage();
  
  void setShuffle(bool enable) { shuffle = enable; }
  void setInterval(unsigned long ms) { intervalMs = ms; }
};

#endif