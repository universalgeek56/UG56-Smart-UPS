#pragma once
#include <stdint.h>

namespace Ups {
    enum class UpsMode : uint8_t { MANUAL_OFF, MANUAL_ON, CYCLE };

    void begin();
    void update();

    void setMode(UpsMode m);
    UpsMode getMode();
    bool isOn();
    uint32_t getSecondsToNextSwitch();
    void setCycleTimes(uint32_t on_ms,uint32_t off_ms);
    
    void setAllowed(bool allowed);
    bool isAllowed();
}






