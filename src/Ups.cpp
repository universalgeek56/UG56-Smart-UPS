#include "Ups.h"
#include "Board.h"
#include "Settings.h"
#include "Globals.h"
#include <Arduino.h>

namespace Ups {

static UpsMode upsMode = UpsMode::CYCLE;
static bool upsIsOn = false;
static bool upsAllowed = true;
static uint32_t nextSwitchAt = 0;
static uint32_t startupAt = 0;
static bool stateHold = true;

// Saved state for cycle restoration after protection
static bool savedUpsIsOn = false;
static uint32_t savedNextSwitchAt = 0;
static bool restoringCycle = false;

// Schedule the next switch based on current UPS state and cycle times
static void scheduleNextSwitch() {
  uint32_t now = millis();
  uint32_t on_ms, off_ms;
  Settings::getCycle(on_ms, off_ms);
  nextSwitchAt = upsIsOn ? now + on_ms : now + off_ms;
}

void setCycleTimes(uint32_t on_ms, uint32_t off_ms) {
  Settings::setCycle(on_ms, off_ms);
  if (upsMode == UpsMode::CYCLE && !stateHold)
    scheduleNextSwitch();
}

void begin() {
  upsIsOn = false;
  upsMode = Settings::getUpsMode();

  upsAllowed = false;
  stateHold = true;

  startupAt = millis();
  nextSwitchAt = 0;
}

void setMode(UpsMode m) {
  upsMode = m;
  Settings::setUpsMode(m);
  if (!stateHold && upsMode == UpsMode::CYCLE)
    scheduleNextSwitch();
}

UpsMode getMode() {
  return upsMode;
}
bool isOn() {
  return upsIsOn;
}

uint32_t getSecondsToNextSwitch() {
  if (!nextSwitchAt) return 0;
  uint32_t now = millis();
  return (nextSwitchAt > now) ? (nextSwitchAt - now) / 1000 : 0;
}

// Allow or block UPS operation (used for low voltage protection)
void setAllowed(bool allowed) {
  upsAllowed = allowed;

  if (!allowed) {
    // Save state before protection
    savedUpsIsOn = upsIsOn;
    savedNextSwitchAt = nextSwitchAt - millis();

    // Turn off UPS if it is currently on
    if (!stateHold && upsIsOn && !Board::upsButtonBusy())
      Board::upsPressButton();

    upsMode = UpsMode::MANUAL_OFF;
    nextSwitchAt = 0;

  } else {
    // Restore mode from EEPROM
    upsMode = Settings::getUpsMode();

    if (!stateHold && upsMode == UpsMode::CYCLE) {
      // Prepare to restore cycle
      nextSwitchAt = millis() + savedNextSwitchAt;
      restoringCycle = true;
    }
  }
}

// Main UPS update loop
void update() {
  uint32_t now = millis();

  // Handle startup hold
  if (stateHold) {
    if (now - startupAt >= STARTUP_HOLD_MS) {
      stateHold = false;
      upsAllowed = true;
      if (upsMode == UpsMode::CYCLE && !restoringCycle)
        scheduleNextSwitch();  // normal startup
    }
    return;
  }

  // Read UPS state and synchronize
  bool nowOn = Board::readUpsState();
  if (nowOn != upsIsOn && !restoringCycle) {
    upsIsOn = nowOn;
    if (upsMode == UpsMode::CYCLE)
      scheduleNextSwitch();
  }

  // If UPS is not allowed, ensure it is turned off
  if (!upsAllowed) {
    nextSwitchAt = 0;
    if (upsIsOn && !Board::upsButtonBusy()) {
      Board::upsPressButton();
    }
    return;
  }

  switch (upsMode) {
    case UpsMode::MANUAL_ON:
      nextSwitchAt = 0;
      if (!upsIsOn && !Board::upsButtonBusy()) Board::upsPressButton();
      break;

    case UpsMode::MANUAL_OFF:
      nextSwitchAt = 0;
      if (upsIsOn && !Board::upsButtonBusy()) Board::upsPressButton();
      break;

    case UpsMode::CYCLE:
      if (restoringCycle) {
        // Restore UPS state if it was on before protection
        if (savedUpsIsOn && !upsIsOn && !Board::upsButtonBusy()) {
          Board::upsPressButton();
          upsIsOn = true;
        }

        // Wait for the saved timer to trigger next switch
        if (nextSwitchAt && now >= nextSwitchAt && !Board::upsButtonBusy()) {
          Board::upsPressButton();
          scheduleNextSwitch();
          restoringCycle = false;
        }

      } else {
        // Normal cycle operation
        if (nextSwitchAt && now >= nextSwitchAt && !Board::upsButtonBusy()) {
          Board::upsPressButton();
          scheduleNextSwitch();
        }
      }
      break;
  }
}

bool isAllowed() {
  return upsAllowed;
}

}  // namespace Ups
