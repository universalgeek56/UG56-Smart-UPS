#include "Web.h"
#include <ESP8266WebServer.h>
#include "Globals.h"
#include "Ups.h"
#include "Ota.h"
#include "Wifi.h"
#include "Settings.h"
#include "ADC.h"
#include <ESP8266WiFi.h>

namespace Web {

ESP8266WebServer server(80);

static const char STYLE_CSS[] PROGMEM = R"rawliteral(
* {box-sizing: border-box;margin: 0;padding: 0;}
body {font-family: system-ui, sans-serif;background: #0f0f0f;color: #e0e0e0;min-height: 100vh;display: flex;justify-content: center;}
.wrap {width: 100%;max-width: 420px;}
h3 {text-align: center;margin: 6px 0 8px;font-size: 1.3rem;font-weight: 500;}
.block {background: #181818;border: 2px solid #5c5c5c;border-radius: 8px;padding: 14px;margin-bottom: 8px;}
.btn-block {margin-bottom: 8px;}
.state {text-align: center;font-size: 1.05rem;line-height: 1.6;}
.state b {color: #fff;font-weight: 600;}
button {width: 100%;padding: 14px 16px;font-size: 1rem;font-weight: 500;border: 2px solid #5c5c5c;border-radius: 10px;background: #181818;color: #e0e0e0;cursor: pointer;}
button:active {background: #222;}
button.active {background: #4caf50;color: #000;border-color: #4caf50;font-weight: 600;}
button.warn {background: #e53935;border-color: #e53935;color: #fff;}
input[type="range"] {width: 100%;margin: 10px 0 6px;}
input[type="text"],input[type="password"] {width: 100%;padding: 12px 14px;margin: 8px 0 6px;border-radius: 8px;border: 2px solid #5c5c5c;background: #1e1e1e;color: #e0e0e0;font-size: 0.95rem;}
.row {display: flex;align-items: center;margin: 8px 0;}
.row span {min-width: 36px;text-align: right;font-weight: 500;color: #bbb;}
.ok {color: #81c784;}.low {color: #ffb74d;}.crit {color: #e53935;}
small {color: #888;font-size: 0.85rem;line-height: 1.4;display: block;margin-top: 10px;}
.label {margin-bottom: 8px;font-weight: 500;display: block;}
)rawliteral";

static const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!doctype html><html lang="ru"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="stylesheet" href="/style.css">
<title>Smart UPS</title>
</head><body><div class="wrap">
<h3>Smart UPS</h3>
<div class="block state">
UPS State: <b id="ups">?</b><br>
UPS Mode: <b id="mode">?</b><br>
Next Switch: <b id="next">--:--:--</b>
</div>
<div class="block state">
Voltage: <b id="vin">--.-</b> V<br>
Battery: <b id="bat">--%</b> <b id="dir">?</b><br>
Status: <span id="pwr">?</span>
</div>
<div class="btn-block"><button id="on" onclick="setMode(1)">ON</button></div>
<div class="btn-block"><button id="off" onclick="setMode(0)">OFF</button></div>
<div class="btn-block"><button id="cyc" onclick="setMode(2)">CYCLE</button></div>
<div class="block">
<div class="label">ON time (minutes)</div>
<div class="row"><input type="range" min="1" max="120" id="onRange"><span id="onVal">--</span></div>
<div class="label" style="margin-top:16px">OFF time (minutes)</div>
<div class="row"><input type="range" min="1" max="120" id="offRange"><span id="offVal">--</span></div>
</div>
<div class="btn-block"><a href="/battery"><button>⚡ Battery</button></a></div>
<div class="btn-block"><a href="/settings"><button>⚙️ Settings</button></a></div>
</div>
<script>
let userActive=false,releaseT,saveT;
const setMode=m=>fetch('/mode?m='+m);
function hms(s){if(s<0)return'--:--:--';let h=Math.floor(s/3600),m=Math.floor((s%3600)/60),x=s%60;return String(h).padStart(2,'0')+':'+String(m).padStart(2,'0')+':'+String(x).padStart(2,'0');}
function userTouch(){userActive=true;clearTimeout(releaseT);releaseT=setTimeout(()=>userActive=false,1500);}
function debounceSave(){clearTimeout(saveT);saveT=setTimeout(()=>fetch(`/save?on=${onRange.value}&off=${offRange.value}`),600);}
onRange.oninput=_=>{onVal.textContent=onRange.value;userTouch();debounceSave();}
offRange.oninput=_=>{offVal.textContent=offRange.value;userTouch();debounceSave();}
function upd(){fetch('/status').then(r=>r.json()).then(s=>{ups.textContent=s.ups?'ON':'OFF';mode.textContent=s.mode;next.textContent=hms(s.next);
  if(!userActive){onRange.value=s.on;onVal.textContent=s.on;offRange.value=s.off;offVal.textContent=s.off;}on.classList.toggle('active',s.mode==='ON');off.classList.toggle('active',s.mode==='OFF');cyc.classList.toggle('active',s.mode==='CYCLE');
  vin.textContent=s.vin.toFixed(2);bat.textContent=s.bat+'%';dir.textContent=s.charging?'↑ Charging':'↓ Discharging';pwr.className='';if(s.critical){pwr.textContent='CRITICAL';pwr.classList.add('crit');}
  else if(s.low){pwr.textContent='LOW';pwr.classList.add('low');}else{pwr.textContent='OK';pwr.classList.add('ok');}});}
setTimeout(()=>{upd();setInterval(upd,1200);},800);
</script>
</body></html>
)rawliteral";

static const char PAGE_BATTERY[] PROGMEM = R"rawliteral(
<!doctype html><html lang="ru"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="stylesheet" href="/style.css">
<title>Battery</title>
</head><body><div class="wrap">
<h3>Battery Settings</h3>
<div class="block">
<div class="label">Battery Low <span id="vLowWarn"></span></div>
<div class="row"><input type="range" id="vLowRange" min="11.7" max="13" step="0.1"><span id="vLowVal">--</span></div>
<div class="label" style="margin-top:16px">Battery Critical <span id="vCritWarn"></span></div>
<div class="row"><input type="range" id="vCritRange" min="9.9" max="12" step="0.1"><span id="vCritVal">--</span></div>
<div class="label" style="margin-top:16px">Recovery Voltage</div>
<div class="row"><input type="range" id="vRecRange" min="12" max="13" step="0.1"><span id="vRecVal">--</span></div>
</div>
<div class="btn-block"><a href="/"><button>← Back</button></a></div>
</div>
<script>
let saveT;
function debounce(){clearTimeout(saveT);saveT=setTimeout(()=>fetch(`/setadc?vLow=${vLowRange.value}&vCrit=${vCritRange.value}&vRec=${vRecRange.value}`),600);}
vLowRange.oninput=_=>{vLowVal.textContent=vLowRange.value;let isOff=vLowRange.value<=11.7;vLowWarn.textContent=isOff?'⚠ Off':'';vLowWarn.className=isOff?'crit':'';debounce();}
vCritRange.oninput=_=>{let v=+vCritRange.value,r=+vRecRange.value;if(v>r){vCritRange.value=v=r;}vCritVal.textContent=v<=9.9?'off':v.toFixed(1);let isDeg=v<11.8;vCritWarn.textContent=isDeg?'⚠ Battery degradation!':'';vCritWarn.className=isDeg?'crit':'';debounce();}
vRecRange.oninput=_=>{let v=+vRecRange.value,c=+vCritRange.value;if(v<c){vRecRange.value=v=c;}vRecVal.textContent=v.toFixed(1);debounce();}
fetch('/status').then(r=>r.json()).then(s=>{vLowRange.value=s.vLow;vLowVal.textContent=s.vLow.toFixed(1);let isOffLow=s.vLow<=11.7;vLowWarn.textContent=isOffLow?'⚠ Off':'';vLowWarn.className=isOffLow?'crit':'';vCritRange.value=s.vCrit;vCritVal.textContent=s.vCrit.toFixed(1);let isDegCrit=s.vCrit<11.8;vCritWarn.textContent=isDegCrit?'⚠ Battery degradation!':'';vCritWarn.className=isDegCrit?'crit':'';vRecRange.value=s.vRec;vRecVal.textContent=s.vRec.toFixed(1);});
</script>
</body></html>
)rawliteral";

static const char PAGE_SETTINGS[] PROGMEM = R"rawliteral(
<!doctype html><html lang="ru"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="stylesheet" href="/style.css">
<title>Settings</title>
</head><body><div class="wrap">
<h3>Settings</h3>
<div class="block">
<input type="text" id="ssid" placeholder="Wi-Fi SSID">
<input type="password" id="pass" placeholder="Wi-Fi Password">
<div class="btn-block"><button onclick="saveWifi()">Save Wi-Fi & Reboot</button></div>
<small>AP: Smart_UPS / 12345678<br>IP: 192.168.4.1</small>
<div class="btn-block"><button id="toggleWifiBtn" onclick="toggleAPMode()">Toggle AP Mode</button></div>
</div>
<div class="btn-block"><button class="warn" onclick="location.href='/ota.html'">OTA Mode</button></div>
<div class="btn-block"><a href="/"><button>← Back</button></a></div>
</div>
<script>
function saveWifi(){fetch(`/setwifi?ssid=${encodeURIComponent(ssid.value)}&pass=${encodeURIComponent(pass.value)}`).then(()=>alert('Saved. Rebooting...'));}
function toggleAPMode(){fetch('/toggleap').then(()=>alert('Switching mode. Rebooting...'));}
fetch('/apmode').then(r=>r.text()).then(m=>toggleWifiBtn.innerText=(m==='AP'?'Switch to STA & Reboot':'Switch to AP & Reboot'));
fetch('/status').then(r=>r.json()).then(s=>{ssid.value=s.ssid||'';pass.value=s.pass||'';});
</script>
</body></html>
)rawliteral";

static const char PAGE_OTA[] PROGMEM = R"rawliteral(
<!doctype html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="stylesheet" href="/style.css">
<title>OTA</title>
</head><body><div class="wrap">
<div class="block" style="text-align:center">
<h2>OTA MODE</h2>
<div>Waiting for firmware</div>
<div id="t">300</div>
<small>seconds</small>
</div>
</div>
<script>
fetch('/ota');
let t=300;
let tm=setInterval(()=>{t--;document.getElementById('t').innerText=t;if(t<=0){clearInterval(tm);location.reload();}},1000);
setInterval(()=>{fetch('/status').then(r=>r.ok&&location.replace('/'));},8000);
</script>
</body></html>
)rawliteral";

void begin() {

  // ===== CSS =====
  server.on("/style.css", []() {
    server.sendHeader("Cache-Control", "public, max-age=86400");
    server.send_P(200, "text/css", STYLE_CSS);
  });

  // ===== FRONT PAGES =====
  server.on("/", []() {
    server.send_P(200, "text/html", PAGE_INDEX);
    yield();
  });

  server.on("/battery", []() {
    server.send_P(200, "text/html", PAGE_BATTERY);
    yield();
  });

  server.on("/settings", []() {
    server.send_P(200, "text/html", PAGE_SETTINGS);
    yield();
  });

  server.on("/ota.html", []() {
    server.send_P(200, "text/html", PAGE_OTA);
    yield();
  });

  // ===== OTA =====
  server.on("/ota", []() {
    Ota::request();           // сервер уйдёт в OTA, фронт офлайн
    server.send(200, "text/plain", "OK");
  });

  // ===== UPS MODE =====
  server.on("/mode", []() {
    uint8_t m = server.arg("m").toInt();
    if (m == 0) Ups::setMode(Ups::UpsMode::MANUAL_OFF);
    else if (m == 1) Ups::setMode(Ups::UpsMode::MANUAL_ON);
    else if (m == 2) Ups::setMode(Ups::UpsMode::CYCLE);
    server.send(200, "text/plain", "OK");
  });

  // ===== CYCLE SAVE =====
  server.on("/save", []() {
    uint32_t on  = server.arg("on").toInt() * 60000UL;
    uint32_t off = server.arg("off").toInt() * 60000UL;
    Settings::setCycle(on, off);
    Ups::setMode(Ups::UpsMode::CYCLE);
    server.send(200, "text/plain", "OK");
  });

  // ===== BATTERY THRESHOLDS =====
  server.on("/setadc", []() {
    float vLow  = server.arg("vLow").toFloat();
    float vCrit = server.arg("vCrit").toFloat();
    float vRec  = server.arg("vRec").toFloat();

    if (vLow < 11.7f) vLow = 11.7f;
    if (vCrit < 9.9f) vCrit = 9.9f;
    if (vCrit > vRec) vCrit = vRec;
    if (vRec < vCrit) vRec = vCrit;

    Settings::setBatteryThresholds(vLow, vCrit, vRec);
    server.send(200, "text/plain", "OK");
  });

  // ===== STATUS =====
  server.on("/status", []() {
    uint32_t on, off;
    float vLow, vCrit, vRec;

    Settings::getCycle(on, off);
    Settings::getBatteryThresholds(vLow, vCrit, vRec);

    String j;
    j.reserve(256);
    j = "{";
    j += "\"ups\":" + String(Ups::isOn() ? "true" : "false") + ",";
    j += "\"on\":" + String(on / 60000UL) + ",";
    j += "\"off\":" + String(off / 60000UL) + ",";
    j += "\"next\":" + String(Ups::getSecondsToNextSwitch()) + ",";

    Ups::UpsMode m = Ups::getMode();
    j += "\"mode\":\"";
    j += (m == Ups::UpsMode::MANUAL_ON ? "ON" :
          m == Ups::UpsMode::MANUAL_OFF ? "OFF" : "CYCLE");
    j += "\",";

    j += "\"vin\":" + String(ADC::vin(), 2) + ",";
    j += "\"bat\":" + String(ADC::percent()) + ",";
    j += "\"low\":" + String(ADC::low() ? "true" : "false") + ",";
    j += "\"critical\":" + String(ADC::critical() ? "true" : "false") + ",";
    j += "\"charging\":" + String(ADC::isCharging() ? "true" : "false") + ",";

    j += "\"vLow\":" + String(vLow, 1) + ",";
    j += "\"vCrit\":" + String(vCrit, 1) + ",";
    j += "\"vRec\":" + String(vRec, 1) + ",";

    j += "\"ssid\":\"" + String(Settings::getSTA_SSID()) + "\",";
    j += "\"pass\":\"" + String(Settings::getSTA_PASS()) + "\"";
    j += "}";

    server.send(200, "application/json", j);
  });

  // ===== WIFI =====
  server.on("/setwifi", []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    Settings::setWifi(false, ssid.c_str(), pass.c_str());
    server.send(200, "text/plain", "OK");
    delay(100);
    ESP.restart();
  });

  server.on("/toggleap", []() {
    char ssid[32], pass[32];
    strcpy(ssid, Settings::getSTA_SSID());
    strcpy(pass, Settings::getSTA_PASS());
    Settings::setWifi(!Settings::isApMode(), ssid, pass);
    server.send(200, "text/plain", "OK");
    delay(100);
    ESP.restart();
  });

  server.on("/apmode", []() {
    server.send(200, "text/plain", Settings::isApMode() ? "AP" : "STA");
  });

  server.begin();
}


void update() {
  server.handleClient();
}

}  // namespace Web