#include "DisplayManager.h"

CustomLGFX::CustomLGFX() {
    {
      auto cfg = busInstance.config();
      cfg.panel = &panelInstance;
      
      cfg.pin_d0  = GPIO_NUM_15; // B0
      cfg.pin_d1  = GPIO_NUM_7;  // B1
      cfg.pin_d2  = GPIO_NUM_6;  // B2
      cfg.pin_d3  = GPIO_NUM_5;  // B3
      cfg.pin_d4  = GPIO_NUM_4;  // B4
      
      cfg.pin_d5  = GPIO_NUM_9;  // G0
      cfg.pin_d6  = GPIO_NUM_46; // G1
      cfg.pin_d7  = GPIO_NUM_3;  // G2
      cfg.pin_d8  = GPIO_NUM_8;  // G3
      cfg.pin_d9  = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_1;  // G5
      
      cfg.pin_d11 = GPIO_NUM_14; // R0
      cfg.pin_d12 = GPIO_NUM_21; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_48; // R3
      cfg.pin_d15 = GPIO_NUM_45; // R4

      cfg.pin_henable = GPIO_NUM_41;
      cfg.pin_vsync   = GPIO_NUM_40;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_0;
      cfg.freq_write  = 15000000;

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 40;
      cfg.hsync_pulse_width = 48;
      cfg.hsync_back_porch  = 40;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 1;
      cfg.vsync_pulse_width = 31;
      cfg.vsync_back_porch  = 13;

      cfg.pclk_active_neg   = 1;
      cfg.de_idle_high      = 0;
      cfg.pclk_idle_high    = 0;

      busInstance.config(cfg);
    }
    {
      auto cfg = panelInstance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width   = 800;
      cfg.panel_height  = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      panelInstance.config(cfg);
    }
    panelInstance.setBus(&busInstance);
    setPanel(&panelInstance);
}

DisplayManager::DisplayManager(uint8_t backlightPin) : backlightPin(backlightPin) {}

void DisplayManager::begin(uint8_t rotation) {
    pinMode(backlightPin, OUTPUT);
    setBacklight(true);
    lcd.begin();
    lcd.setRotation(rotation);
    clear();
    lcd.setTextSize(2);
}

void DisplayManager::setBacklight(bool enable) {
  digitalWrite(backlightPin, enable ? HIGH : LOW);
}

void DisplayManager::clear(uint32_t color) {
  lcd.fillScreen(color);
}

void DisplayManager::showMessage(const char* text, uint16_t color) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(color, TFT_BLACK);
  lcd.drawString(text, 20, 20);
}

bool DisplayManager::renderCenteredBMP(fs::FS &fs, const char *filename) {
  if (!fs.exists(filename)) return false;

  File file = fs.open(filename, "r");
  if (!file) return false;

  file.seek(18); // Header offset for width
  uint32_t imgWidth  = file.read() | (file.read() << 8) | (file.read() << 16) | (file.read() << 24);
  uint32_t imgHeight = file.read() | (file.read() << 8) | (file.read() << 16) | (file.read() << 24);
  file.close();

  int x = (lcd.width() - (int)imgWidth) / 2;
  int y = (lcd.height() - (int)imgHeight) / 2;
  if (x < 0) x = 0;
  if (y < 0) y = 0;

  clear();
  return lcd.drawBmp(fs, filename, x, y);
}