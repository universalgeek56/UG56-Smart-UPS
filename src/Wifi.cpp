#include "Wifi.h"
#include "Globals.h"
#include "Settings.h"
#include <ESP8266WiFi.h>

namespace Wifi {

static uint32_t connectStart = 0;
static uint32_t lastReconnect = 0;
static bool connecting = false;
static bool wifiConnected = false;

constexpr uint32_t WIFI_TIMEOUT_MS = 15000;
constexpr uint32_t RECONNECT_MS    = 10000;

void begin() {
  const char* ssid = Settings::getSTA_SSID();
  const char* pass = Settings::getSTA_PASS();

  if (Settings::isApMode()) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(wifiApSSID, wifiApPass);
    wifiConnected = true;
    connecting = false;
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  connectStart = millis();
  connecting = true;
}

void update() {
  uint32_t now = millis();

  if (Settings::isApMode()) {
    wifiConnected = true;
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    connecting = false;
    return;
  }

  wifiConnected = false;

  if (connecting && now - connectStart > WIFI_TIMEOUT_MS) {
    connecting = false;
    WiFi.disconnect();
    lastReconnect = now;
  }

  if (!connecting && now - lastReconnect > RECONNECT_MS) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(Settings::getSTA_SSID(), Settings::getSTA_PASS());
    connecting = true;
    connectStart = now;
  }
}

void setMode(bool apMode) {
  Settings::setWifi(apMode, Settings::getSTA_SSID(), Settings::getSTA_PASS());
  ESP.restart();
}

bool isConnected() {
  return wifiConnected;
}

} // namespace Wifi