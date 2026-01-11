#include "Wifi.h"
#include "Globals.h"
#include <ESP8266WiFi.h>

namespace Wifi {

static uint32_t connectStart = 0;
static uint32_t lastReconnect = 0;
static bool connecting = false;

constexpr uint32_t WIFI_TIMEOUT_MS = 15000;
constexpr uint32_t RECONNECT_MS    = 10000;

void begin() {
    if (!wifiEnabled) return;

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPass);

    connectStart = millis();
    connecting = true;
}

void update() {
    if (!wifiEnabled) return;

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        return;
    }

    wifiConnected = false;
    uint32_t now = millis();

    if (connecting && now - connectStart > WIFI_TIMEOUT_MS) {
        connecting = false;
        WiFi.disconnect();
        lastReconnect = now;
    }

    if (!connecting && now - lastReconnect > RECONNECT_MS) {
        WiFi.begin(wifiSSID, wifiPass);
        connecting = true;
        connectStart = now;
    }
}

} // namespace Wifi
