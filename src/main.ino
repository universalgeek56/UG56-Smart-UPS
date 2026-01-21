#include "Board.h"
#include "Globals.h"
#include "Wifi.h"
#include "Ota.h"
#include "Ups.h"
#include "Settings.h"
#include "Web.h"
#include "ADC.h"

void setup() {
    Board::begin();
    Settings::begin();
    ADC::begin();
    Wifi::begin();
    Ups::begin();
    Web::begin();

}

void loop() {
    ADC::update();
    Wifi::update();
    Board::update();
    Ota::update();

    if (Ota::active()) return;

    Ups::update();
    Web::update();
}





