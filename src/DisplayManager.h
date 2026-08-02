#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

class CustomLGFX : public lgfx::LGFX_Device {
public:
    lgfx::Bus_RGB busInstance;
    lgfx::Panel_RGB panelInstance;

    CustomLGFX();
};

class DisplayManager {
private:
    CustomLGFX lcd;
    uint8_t backlightPin;

public:
    DisplayManager(uint8_t backlightPin);

    void begin(uint8_t rotation = 1);
    void setBacklight(bool enable);
    void clear(uint32_t color = TFT_BLACK);
    void showMessage(const char* text, uint16_t color = TFT_WHITE);
    
    // Legacy file-path method (kept for local SD compatibility)
    bool renderCenteredBMP(fs::FS &fs, const char *filename);
    
    // NEW: Generic stream renderer for Cloud socket or SD File stream
    bool drawBmpStream(Stream* stream) {
        return lcd.drawBmp(stream, 0, 0);
    }
    
    int32_t getWidth() const { return lcd.width(); }
    int32_t getHeight() const { return lcd.height(); }
};

#endif