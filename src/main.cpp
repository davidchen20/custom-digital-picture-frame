#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

class LGFX : public lgfx::LGFX_Device
{
public:
  lgfx::Bus_RGB    _bus_instance;
  lgfx::Panel_RGB   _panel_instance;

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      
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

      _bus_instance.config(cfg);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width   = 800;
      cfg.panel_height  = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }
};

LGFX lcd;

#define TFT_BL 2 // Backlight Pin

#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  12
#define SD_CS   10

SPIClass sdSPI = SPIClass(HSPI);

int SD_init();
void displayCenteredBMP(const char * filename);

void setup() {
  Serial.begin(115200);

  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }

  // 1. Initialize Screen and Backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  lcd.begin();

  // Rotate screen 90 degrees clockwise (1 = portrait, 2 = flipped landscape, 3 = flipped portrait)
  lcd.setRotation(1); 

  lcd.fillScreen(TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("Mounting SD Card...", 20, 20);

  // 2. Configure SD pins
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(SD_MISO, INPUT_PULLUP);

  // 3. Initialize dedicated SPI bus
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(100);

  // 4. Mount SD Card & Render BMP
  if (SD_init() == 0) {
    lcd.fillScreen(TFT_BLACK);
    displayCenteredBMP("/test.bmp");
  } else {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_RED, TFT_BLACK);
    lcd.drawString("Failed to Mount SD!", 20, 20);
  }
}

void loop() {
  // Main code loop
}

int SD_init() {
  if (!SD.begin(SD_CS, sdSPI, 1000000)) {
    Serial.println("Card Mount Failed");
    return 1;
  }
  if (SD.cardType() == CARD_NONE) {
    Serial.println("No TF card attached");
    return 1;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("TF Card Size: %llu MB\n", cardSize);
  return 0;
}

void displayCenteredBMP(const char * filename) {
  if (!SD.exists(filename)) {
    Serial.printf("Error: %s not found on SD card!\n", filename);
    lcd.setTextColor(TFT_RED, TFT_BLACK);
    lcd.printf("File %s not found!", filename);
    return;
  }

  // Read dimensions directly from the BMP header
  File file = SD.open(filename, "r");
  if (!file) return;

  file.seek(18); // Offset for width
  uint32_t imgWidth  = file.read() | (file.read() << 8) | (file.read() << 16) | (file.read() << 24);
  uint32_t imgHeight = file.read() | (file.read() << 8) | (file.read() << 16) | (file.read() << 24);
  file.close();

  // Calculate centered X and Y offsets for rotated screen bounds
  int x = (lcd.width() - (int)imgWidth) / 2;
  int y = (lcd.height() - (int)imgHeight) / 2;

  if (x < 0) x = 0;
  if (y < 0) y = 0;

  Serial.printf("Screen: %dx%d | Image: %dx%d | Position: (%d, %d)\n", 
                lcd.width(), lcd.height(), imgWidth, imgHeight, x, y);

  bool success = lcd.drawBmp(SD, filename, x, y);

  if (!success) {
    Serial.println("Failed to render BMP! Check if image is 24-bit uncompressed BMP.");
  } else {
    Serial.println("BMP rendered successfully.");
  }
}