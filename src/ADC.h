#pragma once
#include <Arduino.h>

namespace ADC {

    // -------- Initialization --------
    void begin();

    // -------- Main update, call in loop --------
    void update();

    // -------- Measured values --------
    float vin();  
    bool  critical();   
    bool  low();        
    uint8_t percent();  
    bool isCharging();  

} // namespace ADC



