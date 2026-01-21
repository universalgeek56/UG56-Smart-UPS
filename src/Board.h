#pragma once
#include <Arduino.h>

namespace Board {
    void begin();
    void update();

    bool readUpsState();
    void upsPressButton();
    bool upsButtonBusy();
}





