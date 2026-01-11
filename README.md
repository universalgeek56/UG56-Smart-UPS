

---

# UG56 Smart UPS

**Spartan ESP8266 UPS controller for keeping essential systems alive under limited power.**

---

## Overview

Many consumer UPS units, inverters and power stations have significant self-consumption even when the load is minimal.  
Keeping such devices permanently powered often wastes more energy than the protected equipment itself consumes.

This project addresses that problem by **periodic and controlled power management**.

Instead of keeping a UPS always on, the controller allows it to be:

- powered **only when needed**,

- switched on and off in **cyclic mode**,

- or kept **manually forced on or off**.

This approach makes it possible to support essential systems **much longer** under limited or unstable energy conditions.

---

## Design Principles

- **External control only**  
  The controller interacts with the UPS using optocouplers, simulating button presses or control signals.  
  No internal modification of the UPS, inverter, or power device is required.

- **Minimalism and robustness**  
  The firmware avoids unnecessary complexity and dependencies.  
  Predictable behavior is prioritized over features.

- **Offline-first**  
  The system remains fully operational without network connectivity.

- **Scalable by design**  
  When network connectivity is available, the controller can be extended without changing the core logic.

---

## Hardware Concept

The controller connects to the UPS externally:

- one optocoupler to control the UPS power or button input,

- one feedback signal to detect the actual UPS state.

This ensures:

- electrical isolation,

- no interference with internal UPS logic,

- compatibility with a wide range of devices.

The reference implementation uses **ESP8266**, but the design is **not tied to any specific microcontroller**.

---

## Operating Modes

The system supports three basic modes:

- **Manual OFF**  
  UPS is kept powered off.

- **Manual ON**  
  UPS is kept powered on.

- **Cycle**  
  UPS is periodically switched on and off using configurable intervals.

Mode selection is performed using a **single physical button**.

---

## Local Control

- **Short button press**  
  Cycles through operating modes.

- **Long button press**  
  Activates OTA update mode (if supported by the controller).

A status LED provides clear visual feedback:

- steady OFF — Manual OFF mode,

- steady ON — Manual ON mode,

- slow blinking — Cycle mode,

- fast blinking — OTA active.

---

## Network and Expansion

The project is **not network-dependent**, but when a network-capable controller is used, additional functionality becomes available:

- remote monitoring and control,

- configuration via web interface,

- OTA firmware updates.

This allows seamless scaling from a standalone offline device to integration with:

- smart home systems,

- automation servers,

- messaging bots,

- voice assistants.

The core power-management logic remains unchanged.

---

## Persistence

Operating mode and timing parameters are stored in non-volatile memory.  
Configuration is preserved across reboots and power loss.

Writes are delayed and batched to minimize EEPROM wear.

---

## Intended Use

This project is designed for:

- unattended or rarely visited locations,

- systems operating under limited energy availability,

- environments where simplicity and reliability matter more than features.

---

## Status

The project is actively developed and tested on real hardware.  
The current implementation focuses on stability and core functionality.

Documentation and hardware photos will be added as the project evolves.

---

## License

Open-source.  
Use, modify, and adapt as needed.

---
