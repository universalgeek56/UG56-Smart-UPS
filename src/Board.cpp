#include "Board.h"
#include "Globals.h"
#include "Ota.h"
#include "Ups.h"

#include <Arduino.h>

namespace Board {

// ===== ПИНЫ (ЛОКАЛЬНО!) =====
static constexpr uint8_t MODE_BTN  = 0;
static constexpr uint8_t MODE_LED  = 1;
static constexpr uint8_t UPS_OUT   = 2;
static constexpr uint8_t UPS_STATE = 3;

// ===== КНОПКА РЕЖИМОВ =====
static bool     btnPrev        = HIGH;
static bool     btnPressed     = false;
static bool     btnLongHandled = false;
static uint32_t btnPressTime   = 0;

static constexpr uint32_t BTN_LONG_MS = 3000;


// ===== UPS КНОПКА =====
static bool     upsBtnActive = false;
static uint32_t upsBtnStart  = 0;
static constexpr uint32_t UPS_BTN_PRESS_MS = 500;

// ============================

void begin() {
    pinMode(MODE_BTN, INPUT_PULLUP);
    pinMode(MODE_LED, OUTPUT);
    pinMode(UPS_OUT, OUTPUT);
    pinMode(UPS_STATE, INPUT);

    digitalWrite(MODE_LED, LOW);
    digitalWrite(UPS_OUT, HIGH); // кнопка через +V
}

// ============================
// UPS ЖЕЛЕЗО
// ============================

bool readUpsState() {
    return digitalRead(UPS_STATE) == LOW; // оптопара
}


void upsPressButton() {
    if (upsBtnActive) return;

    upsBtnActive = true;
    upsBtnStart  = millis();
    digitalWrite(UPS_OUT, LOW); // активный LOW
}

bool upsButtonBusy() {
    return upsBtnActive;
}

// ============================
// КНОПКА РЕЖИМОВ
// ============================

static void handleModeButton() {
    bool state = digitalRead(MODE_BTN);

    // нажатие
    if (state == LOW && btnPrev == HIGH) {
        btnPressed     = true;
        btnLongHandled = false;
        btnPressTime   = millis();
    }

    // длинное удержание → OTA
    if (btnPressed &&
        !btnLongHandled &&
        millis() - btnPressTime >= BTN_LONG_MS) {

        btnLongHandled = true;
        Ota::request();
    }

    // отпускание
    if (state == HIGH && btnPrev == LOW) {
        if (btnPressed && !btnLongHandled) {
            Ups::nextMode();   // короткое
        }
        btnPressed = false;
    }

    btnPrev = state;
}


// ============================
// LED
// ============================

void updateLed() {
    uint32_t now = millis();

    if (Ota::active()) {
        // быстрое мигание ~5 Гц
        digitalWrite(MODE_LED, (now / 100) % 2);
        return;
    }

    switch (upsMode) {
        case UpsMode::MANUAL_OFF:
            digitalWrite(MODE_LED, LOW);
            break;

        case UpsMode::MANUAL_ON:
            digitalWrite(MODE_LED, HIGH);
            break;

        case UpsMode::CYCLE:
            digitalWrite(MODE_LED, (now / 500) % 2);
            break;
    }
}


// ============================
// LOOP
// ============================

void update() {
    handleModeButton();
    updateLed();

    // отпускание UPS кнопки
    if (upsBtnActive && millis() - upsBtnStart >= UPS_BTN_PRESS_MS) {
        digitalWrite(UPS_OUT, HIGH);
        upsBtnActive = false;
    }
}

} // namespace Board


