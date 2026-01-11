#include "Settings.h"
#include "Globals.h"
#include <EEPROM.h>
#include <string.h>

namespace Settings {

static constexpr uint32_t MAGIC   = 0x55505331; // "UPS1"
static constexpr uint8_t  VERSION = 1;

static Data lastSaved;
static uint32_t lastChangeMs = 0;
static bool pendingSave = false;

static constexpr uint32_t SAVE_DELAY_MS = 3000;

Data data;

// ===== internal =====

static void applyDefaults() {
    data.magic = MAGIC;
    data.version = VERSION;

    data.mode = Mode::OFF;

    data.cycleOn_ms  = 60000;
    data.cycleOff_ms = 60000;
}

static void saveNow() {
    noInterrupts();
    EEPROM.put(0, data);
    EEPROM.commit();
    interrupts();

    lastSaved = data;
    pendingSave = false;
}

// ===== public =====

void begin() {
    EEPROM.begin(sizeof(Data));

    EEPROM.get(0, lastSaved);

    bool valid =
        lastSaved.magic == MAGIC &&
        lastSaved.version == VERSION;

    if (!valid) {
        applyDefaults();
        saveNow();
    } else {
        data = lastSaved;
    }
}

void update() {
    if (memcmp(&data, &lastSaved, sizeof(Data)) != 0) {
        lastSaved = data;
        lastChangeMs = millis();
        pendingSave = true;
    }

    if (pendingSave && millis() - lastChangeMs >= SAVE_DELAY_MS) {
        saveNow();
    }
}

void forceSave() {
    saveNow();
}

} // namespace Settings

