#pragma once
#include "Ups.h"
#include <Arduino.h>

namespace Settings {

// -------- Initialization --------
void begin();

// -------- UPS cycle --------
void getCycle(uint32_t &on, uint32_t &off);
void setCycle(uint32_t on, uint32_t off);

// -------- Wi-Fi --------
void setWifi(bool ap, const char* ssid, const char* pass);
bool isApMode();
const char* getSTA_SSID();
const char* getSTA_PASS();

// -------- UPS mode --------
void setUpsMode(Ups::UpsMode m);
Ups::UpsMode getUpsMode();

// -------- Battery voltage thresholds --------
// vLow      : warning level
// vCritical : protection / shutdown level (9.9 = OFF)
// vRecover  : recovery level (must be >= vCritical)

void getBatteryThresholds(float &vLow, float &vCritical, float &vRecover);
void setBatteryThresholds(float vLow, float vCritical, float vRecover);

} // namespace Settings






