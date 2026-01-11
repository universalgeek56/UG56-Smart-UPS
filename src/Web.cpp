#include "Web.h"
#include <ESP8266WebServer.h>
#include "Globals.h"

namespace Web {

ESP8266WebServer server(80);

// ================= MAIN PAGE =================

static const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="utf-8">
<title>UPS</title>
<style>
body{
  font-family:system-ui,sans-serif;
  background:#111;
  color:#eee;
  margin:0;
  min-height:100vh;
  display:flex;
  align-items:center;
  justify-content:center;
}
.wrap{width:100%;max-width:420px;padding:10px}
h3{text-align:center;margin:10px 0}
.block{
  background:#222;
  padding:12px;
  margin-bottom:10px;
  border-radius:8px;
}
.state{font-size:18px;text-align:center}
button{
  width:100%;
  padding:12px;
  margin:4px 0;
  font-size:16px;
  border:0;
  border-radius:8px;
  background:#333;
  color:#eee;
}
button.active{background:#4caf50;color:#000}
button.warn{background:#c62828}
input[type=range]{width:100%}
.row{display:flex;align-items:center;gap:8px}
small{opacity:.6}
</style>
</head>
<body>

<div class="wrap">

<h3>UPS Control</h3>

<div class="block state">
State: <b id="ups">?</b><br>
Mode: <b id="mode">?</b><br>
Next: <b id="next">-</b>
</div>

<div class="block">
<button id="on"  onclick="setMode(1)">ON</button>
<button id="off" onclick="setMode(0)">OFF</button>
<button id="cyc" onclick="setMode(2)">CYCLE</button>
</div>

<div class="block">
<div>ON time (min)</div>
<div class="row">
<input type="range" min="1" max="120" id="onRange"
 oninput="onVal.innerText=this.value">
<span id="onVal"></span>
</div>

<div>OFF time (min)</div>
<div class="row">
<input type="range" min="1" max="120" id="offRange"
 oninput="offVal.innerText=this.value">
<span id="offVal"></span>
</div>

<button onclick="save()">Save</button>
</div>

<div class="block">
<button class="warn" onclick="startOTA()">OTA UPDATE</button>
<small>Device will wait 5 minutes for firmware</small>
</div>

</div>

<script>
function setMode(m){ fetch('/mode?m='+m); }

function save(){
 fetch('/save?on='+onRange.value+'&off='+offRange.value);
}

function startOTA(){
 fetch('/ota').then(r=>r.text()).then(t=>{
   document.open();document.write(t);document.close();
 });
}

function upd(){
 fetch('/status').then(r=>r.json()).then(s=>{
   ups.innerText  = s.ups ? 'ON':'OFF';
   mode.innerText = s.mode;
   next.innerText = s.next ? s.next+' s':'-';

   onRange.value=s.on; offRange.value=s.off;
   onVal.innerText=s.on; offVal.innerText=s.off;

   on.classList.toggle('active', s.mode=='ON');
   off.classList.toggle('active', s.mode=='OFF');
   cyc.classList.toggle('active', s.mode=='CYCLE');
 });
}
setInterval(upd,1000); upd();
</script>
</body></html>
)rawliteral";

// ================= OTA PAGE =================

static const char OTA_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="utf-8">
<title>OTA</title>
<style>
body{
  font-family:system-ui,sans-serif;
  background:#111;
  color:#eee;
  margin:0;
  height:100vh;
  display:flex;
  align-items:center;
  justify-content:center;
}
.box{text-align:center}
h2{margin-bottom:10px}
#t{font-size:28px}
small{opacity:.6}
</style>
</head>
<body>
<div class="box">
<h2>OTA MODE</h2>
<div>Waiting for firmware</div>
<div id="t">300</div>
<small>seconds</small>
</div>

<script>
let t=300;
setInterval(()=>{
  t--;
  document.getElementById('t').innerText=t;
  if(t<=0){
    document.body.innerHTML='<h2>Rebooting...</h2>';
  }
},1000);
</script>
</body></html>
)rawliteral";

// ================= HANDLERS =================

void begin() {

    server.on("/", [](){
        if (otaMode) {
            server.send_P(200, "text/html", OTA_PAGE);
        } else {
            server.send_P(200, "text/html", PAGE);
        }
    });

    server.on("/mode", [](){
        uint8_t m = server.arg("m").toInt();
        if (m == 0) upsMode = UpsMode::MANUAL_OFF;
        if (m == 1) upsMode = UpsMode::MANUAL_ON;
        if (m == 2) upsMode = UpsMode::CYCLE;
        server.send(200, "text/plain", "OK");
    });

    server.on("/save", [](){
        cycleOn_ms  = server.arg("on").toInt() * 60000UL;
        cycleOff_ms = server.arg("off").toInt() * 60000UL;
        server.send(200, "text/plain", "SAVED");
    });

    server.on("/ota", [](){
        otaMode = true;
        otaStart_ms = millis();
        server.send_P(200, "text/html", OTA_PAGE);
    });

    server.on("/status", [](){
        String j = "{";
        j += "\"ups\":" + String(upsIsOn?"true":"false") + ",";
        j += "\"on\":" + String(cycleOn_ms/60000UL) + ",";
        j += "\"off\":" + String(cycleOff_ms/60000UL) + ",";
        j += "\"next\":" + String(
            cycleNextSwitchAt && cycleNextSwitchAt > millis()
            ? (cycleNextSwitchAt - millis()) / 1000
            : 0) + ",";
        j += "\"mode\":\"";
        if (upsMode==UpsMode::MANUAL_ON) j+="ON";
        else if (upsMode==UpsMode::MANUAL_OFF) j+="OFF";
        else j+="CYCLE";
        j += "\"}";
        server.send(200,"application/json",j);
    });

    server.begin();
}

void update() {
    if (!otaMode) {
        server.handleClient();
    }
}

} // namespace Web
