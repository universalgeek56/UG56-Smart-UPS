#pragma once
#include <Arduino.h>

namespace Settings {

enum class Mode : uint8_t {
    OFF   = 0,
    ON    = 1,
    CYCLE = 2
};

struct Data {
    uint32_t magic;
    uint8_t  version;

    Mode     mode;

    uint32_t cycleOn_ms;
    uint32_t cycleOff_ms;
};

extern Data data;

void begin();
void update();
void forceSave();

}


