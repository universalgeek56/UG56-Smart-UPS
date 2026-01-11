#include <ESP8266WiFi.h>
#include "Board.h"
#include "Globals.h"
#include "Wifi.h"
#include "Ota.h"
#include "Ups.h"
#include "Settings.h"
#include "Web.h"


void setup() {
  Board::begin();  
  Wifi::begin();
  Settings::begin();
  Ups::begin();
  Web::begin();
}

void loop() {
  Wifi::update();
  Board::update();
  Ota::update();    

  if (Ota::active()) {
    return;           
  }

  Settings::update();
  Ups::update();
  Web::update();
}



