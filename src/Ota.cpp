#include "Ota.h"
#include "Globals.h"

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

namespace Ota {

// ====== НАСТРОЙКИ ======
static constexpr uint32_t OTA_TIMEOUT_MS = 5 * 60 * 1000; // 5 минут

// ====== СОСТОЯНИЕ ======
static bool otaRequested = false;
static bool otaStarted   = false;

static uint32_t otaStartedAt = 0;
static char otaHostname[24];

// ====== ВНУТРЕННИЕ ======

static void makeOtaHostname() {
    uint8_t mac[6];
    WiFi.macAddress(mac);

    snprintf(
        otaHostname,
        sizeof(otaHostname),
        "UG-UPS-%02X%02X%02X",
        mac[3], mac[4], mac[5]
    );
}

static void beginInternal() {
    if (otaStarted) return;

    makeOtaHostname();
    ArduinoOTA.setHostname(otaHostname);
    ArduinoOTA.begin();

    otaStarted = true;
}

// ====== API ======

void request() {
    if (otaRequested) return;

    otaRequested = true;
    otaStartedAt = millis();
}

bool active() {
    return otaRequested;
}

const char* hostname() {
    return otaHostname;
}

uint32_t secondsLeft() {
    if (!otaRequested) return 0;

    uint32_t elapsed = millis() - otaStartedAt;
    if (elapsed >= OTA_TIMEOUT_MS) return 0;

    return (OTA_TIMEOUT_MS - elapsed) / 1000;
}

// ====== LOOP ======

void update() {
    if (!otaRequested) return;

    if (!otaStarted) {
        beginInternal();
    }

    ArduinoOTA.handle();
    yield();

    if (millis() - otaStartedAt >= OTA_TIMEOUT_MS) {
        ESP.restart();
    }
}

} // namespace Ota
