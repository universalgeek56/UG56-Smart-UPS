#include "Ups.h"
#include "Globals.h"
#include "Board.h"

namespace Ups {

static uint32_t nextSwitchAt = 0;

// ========================

void begin() {
    upsIsOn = Board::readUpsState();
    nextSwitchAt = 0;
    cycleNextSwitchAt = 0;
}
// ========================

static void scheduleNextSwitch() {
    uint32_t now = millis();

    nextSwitchAt = upsIsOn
        ? now + cycleOn_ms
        : now + cycleOff_ms;

    cycleNextSwitchAt = nextSwitchAt;
}
// ========================

void nextMode() {
    switch (upsMode) {
        case UpsMode::MANUAL_OFF:
            upsMode = UpsMode::MANUAL_ON;
            break;

        case UpsMode::MANUAL_ON:
            upsMode = UpsMode::CYCLE;
            scheduleNextSwitch();
            break;

        case UpsMode::CYCLE:
            upsMode = UpsMode::MANUAL_OFF;
            break;
    }
}


// ========================

uint32_t secondsToNextSwitch() {
    if (!cycleNextSwitchAt) return 0;

    uint32_t now = millis();
    if (now >= cycleNextSwitchAt) return 0;

    return (cycleNextSwitchAt - now) / 1000;
}

// ========================

void update() {

    // реальное состояние
    bool nowOn = Board::readUpsState();
    if (nowOn != upsIsOn) {
        upsIsOn = nowOn;
        if (upsMode == UpsMode::CYCLE) {
            scheduleNextSwitch();
        }
    }

    // логика режимов
    switch (upsMode) {

        case UpsMode::MANUAL_ON:
            nextSwitchAt = 0;
            cycleNextSwitchAt = 0;
            if (!upsIsOn && !Board::upsButtonBusy()) {
                Board::upsPressButton();
            }
            break;

        case UpsMode::MANUAL_OFF:
            nextSwitchAt = 0;
            cycleNextSwitchAt = 0;
            if (upsIsOn && !Board::upsButtonBusy()) {
                Board::upsPressButton();
            }
            break;

        case UpsMode::CYCLE:
            if (nextSwitchAt &&
                millis() >= nextSwitchAt &&
                !Board::upsButtonBusy()) {

                Board::upsPressButton();
                scheduleNextSwitch();
            }
            break;
    }
}

} // namespace Ups

