#include "ADC.h"
#include "Globals.h"
#include "Ups.h"
#include "Settings.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

namespace ADC {

// -------- EMA filter --------
static constexpr float EMA_ALPHA = 0.15f;
static float adcEMA = 0.0f;
static bool first = true;

// -------- Divider --------
static constexpr float R1 = 20000.0f;
static constexpr float R2 = 1000.0f;

// -------- ADC constants --------
static constexpr float ADC_MAX = 1024.0f;
static constexpr float ADC_VREF = 1.0f;
static constexpr float V_CAL = 0.977f;

// -------- Timing --------
static constexpr uint32_t SAMPLE_MS = 500;
static uint32_t lastSample = 0;

// -------- State --------
static float v_in = 0.0f;
static bool isLow = false;
static bool isCritical = false;
static uint32_t recoverAt = 0;

// -------- Wi-Fi timing --------
static uint32_t wifiLowVoltageAt = 0;

// -------- Direction --------
static float v_prev = 0.0f;
static float acc = 0.0f;
static constexpr float DIR_THRESHOLD = 0.05f;
static bool charging = false;

// -------- Critical delay --------
static uint32_t criticalTriggeredAt = 0;
static constexpr uint32_t CRITICAL_DELAY = 5000;
static constexpr uint32_t WIFI_OFF_DELAY = 60000;

// -------- Cold-start hold --------
static bool stateHold = true;
static uint32_t startupAt = 0;

// -------- Saved UPS state --------
static bool upsAllowedBeforeCritical = true;

// -------- Initialize --------
void begin() {
  first = true;
  adcEMA = 0.0f;

  v_in = v_prev = 13.0f;
  isLow = false;
  isCritical = false;
  acc = 0.0f;
  charging = false;

  lastSample = 0;
  criticalTriggeredAt = 0;
  recoverAt = 0;
  wifiLowVoltageAt = 0;

  stateHold = true;
  startupAt = millis();

  upsAllowedBeforeCritical = Ups::isAllowed();
}

// -------- Update --------
void update() {
  uint32_t now = millis();
  if (now - lastSample < SAMPLE_MS) return;
  lastSample = now;

  int raw = analogRead(A0);
  adcEMA = first ? raw : EMA_ALPHA * raw + (1.0f - EMA_ALPHA) * adcEMA;
  first = false;

  float v_pin = adcEMA / ADC_MAX * ADC_VREF;
  v_in = v_pin * (R1 + R2) / R2 * V_CAL;

  if (stateHold) {
    if (now - startupAt >= STARTUP_HOLD_MS) stateHold = false;
    v_prev = v_in;
    return;
  }

  // -------- Load thresholds dynamically --------
  float vLow, vCrit, vRec;
  Settings::getBatteryThresholds(vLow, vCrit, vRec);

  bool lowEnabled = vLow > 0.0f;
  bool criticalEnabled = vCrit > 9.9f;  // 9.9 = OFF
  bool recoverEnabled = vRec > 0.0f;

  // -------- LOW warning --------
  isLow = lowEnabled && !isCritical && (v_in < vLow);

  // -------- CRITICAL --------
  if (criticalEnabled && !isCritical && v_in < vCrit) {
    if (!criticalTriggeredAt) {
      criticalTriggeredAt = now;
      upsAllowedBeforeCritical = Ups::isAllowed();
    } else if (now - criticalTriggeredAt >= CRITICAL_DELAY) {
      isCritical = true;
      recoverAt = 0;
      Ups::setAllowed(false);
      wifiLowVoltageAt = now;
    }
  } else {
    criticalTriggeredAt = 0;
  }

  // -------- RECOVERY --------
  if (recoverEnabled && isCritical && v_in > vRec) {
    if (!recoverAt) recoverAt = now;
    else if (now - recoverAt >= 30000) {
      isCritical = false;
      recoverAt = 0;
      Ups::setAllowed(upsAllowedBeforeCritical);
    }
  } else {
    recoverAt = 0;
  }

  // -------- Wi-Fi power save --------
  if (criticalEnabled && v_in < vCrit) {
    if (!wifiLowVoltageAt) wifiLowVoltageAt = now;
    else if (now - wifiLowVoltageAt >= WIFI_OFF_DELAY && wifiEnabled) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      wifiEnabled = false;
    }
  }

  // -------- Direction --------
  acc += (v_in - v_prev);
  if (acc > DIR_THRESHOLD) {
    charging = true;
    acc = 0;
  } else if (acc < -DIR_THRESHOLD) {
    charging = false;
    acc = 0;
  }
  v_prev = v_in;
}

// -------- API --------
float vin() {
  return v_in;
}
bool low() {
  return isLow;
}
bool critical() {
  return isCritical;
}
bool isCharging() {
  return charging;
}
uint8_t percent() {
  float v = v_in;

  struct P {
    float v;
    uint8_t p;
  };

  static const P dischargeTable[] = {
    { 11.50f, 0 }, { 11.70f, 10 }, { 11.85f, 20 }, { 11.95f, 30 }, { 12.05f, 40 }, { 12.15f, 50 }, { 12.25f, 60 }, { 12.35f, 70 }, { 12.45f, 80 }, { 12.55f, 90 }, { 12.65f, 100 }
  };
  constexpr size_t dischargeSize = sizeof(dischargeTable) / sizeof(dischargeTable[0]);

  static const P chargeTable[] = {
    { 11.60f, 0 }, { 11.80f, 10 }, { 12.00f, 20 }, { 12.10f, 30 }, { 12.20f, 40 }, { 12.30f, 50 }, { 12.40f, 60 }, { 12.50f, 70 }, { 12.60f, 80 }, { 12.70f, 90 }, { 12.80f, 100 }
  };
  constexpr size_t chargeSize = sizeof(chargeTable) / sizeof(chargeTable[0]);

  const P* table = charging ? chargeTable : dischargeTable;
  size_t tableSize = charging ? chargeSize : dischargeSize;

  if (v <= table[0].v) return 0;
  if (v >= table[tableSize - 1].v) return 100;

  for (size_t i = 1; i < tableSize; ++i) {
    if (v < table[i].v) {
      float dv = table[i].v - table[i - 1].v;
      float dp = table[i].p - table[i - 1].p;
      return table[i - 1].p + static_cast<uint8_t>((v - table[i - 1].v) * dp / dv + 0.5f);
    }
  }

  return 100;
}

}  // namespace ADC
