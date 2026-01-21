#pragma once
#include <Arduino.h>
#include "Ups.h"

/* DEVICE */
extern char deviceName[16];

/* WIFI */
extern bool wifiEnabled;
extern bool wifiConnected;

extern char wifiApSSID[32];
extern char wifiApPass[16];

static constexpr uint32_t STARTUP_HOLD_MS = 10000;



