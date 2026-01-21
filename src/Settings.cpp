#include "Settings.h"
#include <EEPROM.h>
#include <string.h>

namespace Settings {

// -------- Settings data structure --------
struct Data {
    uint32_t magic;
    uint8_t  version;

    uint32_t on_ms;
    uint32_t off_ms;
    uint8_t  wifiMode;   // 0 = STA, 1 = AP
    uint8_t  upsMode;

    char     ssid[32];
    char     pass[32];

    // -------- Battery thresholds --------
    float vLow;
    float vCritical;
    float vRecover;
};

// -------- Magic and version --------
static constexpr uint32_t MAGIC   = 0x55505331;  // "UPS1"
static constexpr uint8_t  VERSION = 4;

static Data cfg;

// -------- Init --------
void begin() {
    EEPROM.begin(sizeof(Data));
    EEPROM.get(0, cfg);

    if (cfg.magic != MAGIC || cfg.version != VERSION) {
        cfg.magic   = MAGIC;
        cfg.version = VERSION;

        cfg.on_ms  = 5UL * 60 * 1000;
        cfg.off_ms = 5UL * 60 * 1000;

        cfg.wifiMode = 0;
        cfg.upsMode  = static_cast<uint8_t>(Ups::UpsMode::CYCLE);

        strncpy(cfg.ssid, "anton", sizeof(cfg.ssid) - 1);
        cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';

        strncpy(cfg.pass, "yokohama12", sizeof(cfg.pass) - 1);
        cfg.pass[sizeof(cfg.pass) - 1] = '\0';

        cfg.vLow      = 12.1f;
        cfg.vCritical = 11.9f;
        cfg.vRecover  = 12.5f;

        EEPROM.put(0, cfg);
        EEPROM.commit();
    }
}

// -------- UPS cycle --------
void getCycle(uint32_t &on, uint32_t &off) {
    on  = cfg.on_ms;
    off = cfg.off_ms;
}

void setCycle(uint32_t on, uint32_t off) {
    cfg.on_ms  = on;
    cfg.off_ms = off;
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

// -------- Wi-Fi --------
void setWifi(bool ap, const char* s, const char* p) {
    cfg.wifiMode = ap ? 1 : 0;

    strncpy(cfg.ssid, s, sizeof(cfg.ssid) - 1);
    cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';

    strncpy(cfg.pass, p, sizeof(cfg.pass) - 1);
    cfg.pass[sizeof(cfg.pass) - 1] = '\0';

    EEPROM.put(0, cfg);
    EEPROM.commit();
}

bool isApMode() {
    return cfg.wifiMode == 1;
}

const char* getSTA_SSID() {
    return cfg.ssid;
}

const char* getSTA_PASS() {
    return cfg.pass;
}

// -------- UPS mode --------
void setUpsMode(Ups::UpsMode m) {
    cfg.upsMode = static_cast<uint8_t>(m);
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

Ups::UpsMode getUpsMode() {
    return static_cast<Ups::UpsMode>(cfg.upsMode);
}

// -------- Battery thresholds --------
void getBatteryThresholds(float &vLow, float &vCritical, float &vRecover) {
    vLow      = cfg.vLow;
    vCritical = cfg.vCritical;
    vRecover  = cfg.vRecover;
}

void setBatteryThresholds(float vLow, float vCritical, float vRecover) {
    // Safety clamps
    if (vLow < 11.7f) vLow = 11.7f;
    if (vCritical < 9.9f) vCritical = 9.9f;
    if (vRecover < vCritical) vRecover = vCritical;
    if (vCritical > vRecover) vCritical = vRecover;

    cfg.vLow      = vLow;
    cfg.vCritical = vCritical;
    cfg.vRecover  = vRecover;

    EEPROM.put(0, cfg);
    EEPROM.commit();
}

} // namespace Settings
