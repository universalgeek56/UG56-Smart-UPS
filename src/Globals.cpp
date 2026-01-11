#include "Globals.h"

/* DEVICE */
String deviceName = "UG-UPS";

/* WIFI */
bool wifiEnabled   = true;
bool wifiConnected = false;

char wifiSSID[33] = "ASUS";
char wifiPass[65] = "RememberToChange!7";

/* OTA */
bool otaActive = false;
String otaHostname;
uint32_t otaTimeout_ms = 120000;
uint32_t otaStartedAt  = 0;

/* UPS */
UpsMode upsMode = UpsMode::MANUAL_OFF;
bool upsIsOn = false;

/* CYCLE */
uint32_t cycleOn_ms  = 60000;
uint32_t cycleOff_ms = 60000;
uint32_t cycleNextSwitchAt = 0;
