#include "env.h"
#include "StorageManager.h"

StorageManager::StorageManager(const char* apiUrl, int sdCsPin)
  : apiUrl(apiUrl), sdCsPin(sdCsPin), storageMode(StorageMode::NONE) {}

bool StorageManager::begin(const char* ssid, const char* password) {
  Serial.println("\n--- [StorageManager Initializing] ---");
  
  // 1. Force Station Mode and Disconnect any stale connection
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  Serial.printf("[Wi-Fi] Connecting to SSID: %s\n", ssid);
  WiFi.begin(ssid, password);

  // 2. Wait up to 12 seconds for Wi-Fi & DHCP IP assignment
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt < 12000)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // 3. Verify Connection State BEFORE calling refreshPhotos()
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[Wi-Fi] Connected! IP: %s (Signal: %d dBm)\n", 
                  WiFi.localIP().toString().c_str(), 
                  WiFi.RSSI());
    
    storageMode = StorageMode::CLOUD;
    
    // Now call refreshPhotos() safely
    bool hasPhotos = refreshPhotos();
    if (hasPhotos) {
      Serial.printf("[Cloud] Success! Loaded %d photo URL(s).\n", (int)photoList.size());
      return true;
    } else {
      Serial.println("[Cloud] Connected to Wi-Fi, but failed to retrieve valid photo URLs from Vercel.");
    }
  } else {
    Serial.printf("[Wi-Fi] Failed to connect. Status Code: %d\n", WiFi.status());
  }

  // 4. Fallback to SD Card if Wi-Fi or Cloud fails
  Serial.println("[SD] Falling back to SD Card initialization...");
  WiFi.disconnect(true);
  
  SPI.begin(12 /* SCK */, 13 /* MISO */, 11 /* MOSI */, sdCsPin);

  if (SD.begin(sdCsPin)) {
    Serial.println("[SD] SD Card mounted successfully.");
    storageMode = StorageMode::SD_CARD;
    
    bool hasPhotos = scanSdCard();
    if (hasPhotos) {
      Serial.printf("[SD] Found %d .bmp image(s).\n", (int)photoList.size());
      return true;
    }
  } else {
    Serial.println("[SD] Failed to mount SD Card.");
  }

  storageMode = StorageMode::NONE;
  return false;
}

bool StorageManager::refreshPhotos() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Cloud] Skipped: Wi-Fi not connected.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Skip TLS certificate verification for testing

  HTTPClient http;
  
  Serial.printf("[HTTP] Connecting to API: %s\n", VERCEL_API);
  
  if (!http.begin(client, VERCEL_API)) {
    Serial.println("[HTTP] Unable to connect to server endpoint.");
    return false;
  }

  // Handle Next.js / Vercel HTTPS redirects (307/308)
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(10000); // 10s timeout

  int httpCode = http.GET();
  Serial.printf("[HTTP] Response code: %d\n", httpCode);

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.printf("[HTTP] Payload received (%d bytes):\n%s\n", payload.length(), payload.c_str());

    // 👈 ArduinoJson v7 syntax: simple JsonDocument replaces DynamicJsonDocument
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
      Serial.printf("[JSON] Deserialization failed: %s\n", err.c_str());
      http.end();
      return false;
    }

    photoList.clear();
    JsonArray arr = doc.as<JsonArray>();
    for (JsonVariant v : arr) {
      String url = v.as<String>();
      if (url.length() > 0) {
        photoList.push_back(url);
      }
    }

    http.end();
    Serial.printf("[Cloud] Successfully loaded %d photo URL(s)!\n", (int)photoList.size());
    return !photoList.empty();
  } else {
    Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return false;
}

bool StorageManager::scanSdCard() {
  photoList.clear();
  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("[StorageManager] Failed to open SD root directory!");
    return false;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String fileName = String(file.name());
      if (fileName.endsWith(".bmp") || fileName.endsWith(".BMP")) {
        // Prepend slash if missing
        if (!fileName.startsWith("/")) fileName = "/" + fileName;
        photoList.push_back(fileName);
      }
    }
    file = root.openNextFile();
  }

  Serial.printf("[StorageManager] Found %d BMP files on SD Card.\n", photoList.size());
  return !photoList.empty();
}

size_t StorageManager::getPhotoCount() const {
  return photoList.size();
}

bool StorageManager::streamPhoto(size_t index, std::function<void(Stream*)> onDataReady) {
  if (index >= photoList.size() || !onDataReady) return false;

  String target = photoList[index];

  // CLOUD MODE STREAMING
  if (storageMode == StorageMode::CLOUD) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    if (http.begin(client, target)) {
      if (http.GET() == HTTP_CODE_OK) {
        WiFiClient* stream = http.getStreamPtr();
        onDataReady(stream);
        http.end();
        return true;
      }
      http.end();
    }
  }
  // SD CARD MODE STREAMING
  else if (storageMode == StorageMode::SD_CARD) {
    File bmpFile = SD.open(target.c_str(), FILE_READ);
    if (bmpFile) {
      onDataReady(&bmpFile); // Pass SD File stream (inherits from Stream)
      bmpFile.close();
      return true;
    }
  }

  return false;
}