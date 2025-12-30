

---

# Smart UPS Controller (ESP8266)

Minimalistic controller for managing an external UPS or power source  
in conditions of limited, unstable, or intermittent energy availability.

Designed for autonomous operation, physical control, and long-term reliability.

---

## Purpose

Maintain minimal life-support or infrastructure operation  
at temporarily unused locations or under limited energy resources.

Typical scenarios include:

- remote or unattended sites

- backup-powered equipment

- power-constrained environments

- systems requiring periodic uptime

---

## Key Features

- Three operation modes
  
  - MANUAL_OFF — UPS forced off
  
  - MANUAL_ON — UPS forced on
  
  - CYCLE — periodic on/off cycling

- Physical control
  
  - Single button
    
    - short press — switch mode
    
    - long press — enable OTA update mode

- Cycle timing
  
  - Configurable ON / OFF durations
  
  - Automatic rescheduling on state change

- Non-blocking logic
  
  - No delays
  
  - Fully timer-based state machine

- State persistence
  
  - Operation mode and settings stored in EEPROM
  
  - Deferred writes to reduce flash wear

- OTA (optional, on demand)
  
  - Activated only by long button press
  
  - Automatically disabled after timeout

- Minimal UI
  
  - Single LED with mode-dependent indication
  
  - No display, no external dependencies

---

## Hardware Overview

Controller is designed around ESP8266  
and interfaces with an external UPS via optocoupler and control line.

Typical connections:

- MODE button — GPIO0 (INPUT_PULLUP)

- MODE LED — GPIO1

- UPS control — GPIO2 (active LOW, via pull-up to +V)

- UPS state — GPIO3 (via optocoupler)

Exact pin usage is centralized in the Board module.

---

## Architecture

The project is split into clear responsibility domains:

Board — all GPIO, buttons, LED, hardware timing  
Ups — UPS logic and mode state machine  
Ota — OTA activation and lifetime control  
Settings — EEPROM persistence  
Web — optional minimal web UI  
Wifi — connectivity

### Design principles

- All GPIO access is isolated in Board

- Logic modules do not touch pins directly

- Short press and long press are strictly separated

- OTA never runs by default

- Device remains functional even without Wi-Fi

---

## LED Indication

MANUAL_OFF — LED off  
MANUAL_ON — LED on  
CYCLE — slow blinking (~1 Hz)  
OTA active — fast blinking (~5 Hz)

---

## OTA Update Mode

OTA is explicitly opt-in.

- Activated by long button press

- Runs for a limited time

- Automatically reboots on timeout

- Normal operation is paused during OTA

This prevents:

- accidental exposure

- unnecessary memory usage

- unintended remote modification

---

## Images

Assembled board  
images/assembled_board.jpg

Wiring / connections  
images/wiring_diagram.jpg

Installed device  
images/installed_device.jpg

(Images will be added after final assembly.)

---

## Build Environment

- Arduino IDE or PlatformIO

- ESP8266 core

- No external libraries beyond standard ESP8266 stack

---

## Status

Project is production-ready for its intended scope.

No planned feature expansion beyond:

- minor UI improvements

- optional web-triggered OTA

---

## License

MIT

Use, modify, deploy — without restrictions.

---
