#pragma once
#include <Arduino.h>

namespace Wifi {
    void begin();
    void update();
    void setMode(bool apMode);
    bool isConnected();
}
