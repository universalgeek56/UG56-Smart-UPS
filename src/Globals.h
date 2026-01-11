#pragma once
#include <Arduino.h>

/* ========= DEVICE ========= */
extern String deviceName;

/* ========= WIFI ========= */
extern bool wifiEnabled;
extern bool wifiConnected;

extern char wifiSSID[33];   // 32 + '\0'
extern char wifiPass[65];   // 64 + '\0'

/* ========= OTA ========= */
extern bool otaActive;
extern String otaHostname;
extern uint32_t otaTimeout_ms;
extern uint32_t otaStartedAt;

/* ========= UPS ========= */
enum class UpsMode : uint8_t {
    MANUAL_OFF = 0,
    MANUAL_ON  = 1,
    CYCLE      = 2
};

extern UpsMode upsMode;
extern bool upsIsOn;

/* ========= CYCLE ========= */
extern uint32_t cycleOn_ms;
extern uint32_t cycleOff_ms;
extern uint32_t cycleNextSwitchAt;
