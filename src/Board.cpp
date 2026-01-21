#include "Board.h"
#include "Ups.h"
#include "Ota.h"
#include "Wifi.h"
#include "Settings.h"
#include <Arduino.h>

namespace Board {

static constexpr uint8_t MODE_BTN=0,MODE_LED=1,UPS_OUT=2,UPS_STATE=3;
static constexpr uint32_t BTN_AP_MS=5000,UPS_BTN_PRESS_MS=1000;

static bool btnPrev=HIGH,btnPressed=false;
static uint32_t btnPressTime=0;

static bool upsBtnActive=false;
static uint32_t upsBtnStart=0;

void begin(){
 pinMode(MODE_BTN,INPUT_PULLUP);
 pinMode(MODE_LED,OUTPUT);
 pinMode(UPS_OUT,OUTPUT);
 pinMode(UPS_STATE,INPUT);
 digitalWrite(MODE_LED,LOW);
 digitalWrite(UPS_OUT,HIGH);
}

// ================= UPS =================
bool readUpsState(){return digitalRead(UPS_STATE)==LOW;}

void upsPressButton(){
 if(upsBtnActive)return;
 upsBtnActive=true;
 upsBtnStart=millis();
 digitalWrite(UPS_OUT,LOW);
}

bool upsButtonBusy(){return upsBtnActive;}

// ================= BUTTON =================
static void handleModeButton(){
 bool state=digitalRead(MODE_BTN);
 uint32_t now=millis();

 if(state==LOW&&btnPrev==HIGH){
  btnPressed=true;
  btnPressTime=now;
 }

 if(state==HIGH&&btnPrev==LOW&&btnPressed){
  uint32_t held=now-btnPressTime;
  if(held>=BTN_AP_MS){
   Wifi::setMode(!Settings::isApMode());
  }else{
   Ups::UpsMode m=Ups::getMode();
   if(m==Ups::UpsMode::MANUAL_OFF)Ups::setMode(Ups::UpsMode::MANUAL_ON);
   else if(m==Ups::UpsMode::MANUAL_ON)Ups::setMode(Ups::UpsMode::CYCLE);
   else Ups::setMode(Ups::UpsMode::MANUAL_OFF);
  }
  btnPressed=false;
 }

 btnPrev=state;
}

// ================= LED HELPERS =================
static bool otaBlink(uint32_t now){
 uint32_t t=(now/100)%7;
 return t==0||t==2;
}

// ================= LED =================
static void updateLed(){
 uint32_t now=millis();

 if(btnPressed&&now-btnPressTime>=BTN_AP_MS){
  digitalWrite(MODE_LED,(now/100)%2);
  return;
 }

 if(Ota::active()){
  digitalWrite(MODE_LED,otaBlink(now));
  return;
 }

 switch(Ups::getMode()){
  case Ups::UpsMode::MANUAL_OFF:digitalWrite(MODE_LED,HIGH);break;
  case Ups::UpsMode::MANUAL_ON:digitalWrite(MODE_LED,LOW);break;
  case Ups::UpsMode::CYCLE:digitalWrite(MODE_LED,(now/500)%2);break;
 }
}

void update(){
 handleModeButton();
 updateLed();
 if(upsBtnActive&&millis()-upsBtnStart>=UPS_BTN_PRESS_MS){
  digitalWrite(UPS_OUT,HIGH);
  upsBtnActive=false;
 }
}

}
