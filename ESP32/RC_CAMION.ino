#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>

// --- CONFIGURACIÓN DE HARDWARE ---
const int pinServo    = 2;
const int pinMotorA   = 12;
const int pinMotorB   = 13;
const int pinBajas    = 14;
const int pinFreno    = 27;
const int pinReversa  = 26;
const int pinGiroIzq  = 25;
const int pinGiroDer  = 33;
const int pinStep     = 32;
const int pinDir      = 4;

// --- DIRECCIÓN (SERVO) ---
const int OFFSET_RECTO = 95;
const int MAX_IZQ      = 55;
const int MAX_DER      = 135;

// --- TOLVA (STEPPER) ---
const long PASOS_MAXIMOS = 9000;
const float STEPPER_VMAX = 1000.0;
const float STEPPER_ACEL = 400.0;

// --- TIMERS ---
const unsigned long BRAKE_DURATION_MS   = 1500;
const unsigned long FAILSAFE_TIMEOUT_MS = 1000;
const unsigned long BLINK_PERIOD_MS     = 500;

// --- WIFI ---
const char* ssid     = "Camion-RC-NeutronLab";
const char* password = "12345678";

WebServer server(80);
Servo direccion;
AccelStepper stepper(AccelStepper::DRIVER, pinStep, pinDir);

// --- ESTADO ---
int   motorState     = 0;     // 0=stop, 1=adelante, 2=reversa
float d_visual       = 0;     // -360..360
bool  lowLights      = false;
bool  hazard         = false;
bool  cardanAcoplado = false;

// --- TOLVA ---
enum TolvaEstado { T_LISTO, T_SUBIENDO, T_BAJANDO };
TolvaEstado tolvaEstado = T_LISTO;

// --- TIMERS DE ESTADO ---
unsigned long lastCommandTime = 0;
unsigned long brakeStartTime  = 0;
unsigned long lastBlink       = 0;
bool blinkState        = false;
bool brakeActive       = false;
int  lastMotorState    = 0;
bool failsafeActive    = false;

// ============================================================
// HTML EMBEBIDO
// ============================================================
const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>RC Truck Pro</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<style>
  body { margin: 0; background: #0b0b0b; color: white; font-family: sans-serif; overflow: hidden; touch-action: none; user-select: none; -webkit-user-select: none; }
  .status-bar { width: 100%; padding: 6px 8px; background: #151515; display: flex; justify-content: space-around; font-size: 11px; border-bottom: 1px solid #222; box-sizing: border-box; }
  .status-bar b { font-weight: bold; }
  .header-controls { background: #1a1a1a; border-bottom: 1px solid #333; padding: 8px; }
  .row { display: flex; gap: 6px; margin-bottom: 6px; align-items: center; }
  .row:last-child { margin-bottom: 0; }
  .btn { flex: 1; height: 34px; border-radius: 5px; border: 1px solid #444; background: #222; color: #888; font-size: 11px; font-weight: bold; }
  .arrow { width: 40px; height: 34px; display: flex; align-items: center; justify-content: center; font-size: 22px; color: #333; font-weight: bold; }
  .locked { opacity: 0.35; pointer-events: none; }
  .main-container { display: flex; flex-direction: column; align-items: center; padding: 14px 0; gap: 12px; }
  .wheel-container { width: 200px; height: 200px; }
  .wheel-svg { width: 100%; height: 100%; touch-action: none; }
  .trigger { width: 150px; height: 55px; font-size: 14px; font-weight: bold; border-radius: 12px; border: none; color: white; }
  .accel { background: #1b5e20; } .brake { background: #e65100; } .reverse { background: #b71c1c; }
  .pressed { filter: brightness(1.8); transform: scale(0.97); }
</style>
</head>
<body>

<div class="status-bar">
  <span>CONEXIÓN: <b id="connStatus" style="color:#2ecc71">ONLINE</b></span>
  <span>TOLVA: <b id="tolvaStatus" style="color:#888">LISTO</b></span>
</div>

<div class="header-controls">
  <div class="row">
    <div id="turnLeft" class="arrow">&#9668;</div>
    <button class="btn" id="swDrive">MODO: RÁP</button>
    <div id="turnRight" class="arrow">&#9658;</div>
  </div>
  <div class="row">
    <button class="btn" id="btnUp">TOLVA UP</button>
    <button class="btn" id="btnDown">TOLVA DOWN</button>
  </div>
  <div class="row">
    <button class="btn" id="lowBtn">LUCES</button>
    <button class="btn" id="hazardBtn">BALIZAS</button>
  </div>
  <div class="row">
    <button class="btn" id="cardanBtn">CARDÁN: DESACOPL.</button>
  </div>
</div>

<div class="main-container">
  <div class="wheel-container">
    <svg id="wheel" class="wheel-svg" viewBox="0 0 200 200">
      <circle cx="100" cy="100" r="92" fill="#1a1a1a" stroke="#0a0a0a" stroke-width="3"/>
      <circle cx="100" cy="100" r="86" fill="none" stroke="#2a2a2a" stroke-width="14"/>
      <circle cx="100" cy="100" r="78" fill="none" stroke="#3a3a3a" stroke-width="2"/>
      <rect x="20" y="96" width="160" height="8" fill="#2a2a2a"/>
      <rect x="20" y="96" width="160" height="2" fill="#3d3d3d"/>
      <polygon points="96,100 104,100 110,170 90,170" fill="#2a2a2a"/>
      <polygon points="96,100 100,100 102,170 98,170" fill="#3d3d3d"/>
      <circle cx="100" cy="100" r="32" fill="#1a1a1a" stroke="#0a0a0a" stroke-width="2"/>
      <circle cx="100" cy="100" r="28" fill="#222"/>
      <circle cx="100" cy="100" r="22" fill="#1a1a1a"/>
      <text x="100" y="105" text-anchor="middle" fill="#666" font-family="sans-serif" font-size="11" font-weight="bold">NL</text>
      <rect x="96" y="6" width="8" height="22" fill="#f44336" rx="1"/>
      <rect x="97" y="6" width="6" height="22" fill="#ff6659" rx="1"/>
    </svg>
  </div>
  <button class="trigger accel"   id="btnAccel">ACELERAR</button>
  <button class="trigger brake"   id="btnBrake">FRENO</button>
  <button class="trigger reverse" id="btnRev">REVERSA</button>
</div>

<script>
const wheel = document.getElementById("wheel");
const turnLeft = document.getElementById("turnLeft");
const turnRight = document.getElementById("turnRight");
const hazardBtn = document.getElementById("hazardBtn");
const lowBtn = document.getElementById("lowBtn");
const swDrive = document.getElementById("swDrive");
const cardanBtn = document.getElementById("cardanBtn");
const btnAccel = document.getElementById("btnAccel");
const btnRev = document.getElementById("btnRev");
const btnBrake = document.getElementById("btnBrake");
const btnUp = document.getElementById("btnUp");
const btnDown = document.getElementById("btnDown");
const connStatus = document.getElementById("connStatus");
const tolvaStatus = document.getElementById("tolvaStatus");

const ORANGE="#ff9800", ORANGE_BORDER="#ffb74d", ORANGE_BG="rgba(255,152,0,0.15)";
const GREEN="#2ecc71", RED="#f44336";
const OFF_TEXT="#888", OFF_BG="#222", OFF_BORDER="#444", ARROW_OFF="#333";

let curAng=0, tarAng=0, lastAng=0, isTouchWheel=false;
let hazardOn=false, lowOn=false, driveFast=true, cardanOn=false;
let motorSt=0, tolvaMoving=false, isOnline=true;
let blinkState=false, tolvaCmd=' ';
let lastOkTime = Date.now();

setInterval(()=>{ blinkState=!blinkState; updateBlinkVisuals(); }, 500);

function setBtnStyle(btn,active,bg,col,bd){
  if(active){ btn.style.background=bg; btn.style.color=col; btn.style.borderColor=bd; }
  else { btn.style.background=OFF_BG; btn.style.color=OFF_TEXT; btn.style.borderColor=OFF_BORDER; }
}
function setLock(btn,locked){ btn.classList.toggle("locked", locked); }

function updateBlinkVisuals(){
  const tL = curAng < -45, tR = curAng > 45;
  const lA = (hazardOn||tL) && isOnline;
  const rA = (hazardOn||tR) && isOnline;
  turnLeft.style.color  = (lA && blinkState) ? ORANGE : ARROW_OFF;
  turnRight.style.color = (rA && blinkState) ? ORANGE : ARROW_OFF;
  if(hazardOn){
    if(blinkState){ hazardBtn.style.background=ORANGE; hazardBtn.style.color="white"; hazardBtn.style.borderColor=ORANGE_BORDER; }
    else { hazardBtn.style.background=ORANGE_BG; hazardBtn.style.color=ORANGE; hazardBtn.style.borderColor=ORANGE_BORDER; }
  }
}

function updateLocks(){
  const stopped = motorSt===0;
  const accelOk = !cardanOn && isOnline;
  setLock(btnAccel, !accelOk);
  setLock(btnRev,   !accelOk);
  setLock(cardanBtn, !(stopped && isOnline));
  const tolvaOk = cardanOn && stopped && !tolvaMoving && isOnline;
  setLock(btnUp,   !tolvaOk);
  setLock(btnDown, !tolvaOk);
}

function getAngle(t){
  const r = wheel.getBoundingClientRect();
  return Math.atan2(t.clientY-(r.top+r.height/2), t.clientX-(r.left+r.width/2))*180/Math.PI;
}
wheel.addEventListener("touchstart",e=>{isTouchWheel=true;lastAng=getAngle(e.touches[0]);},{passive:true});
wheel.addEventListener("touchmove",e=>{
  e.preventDefault();
  const n=getAngle(e.touches[0]); let d=n-lastAng;
  if(d>180)d-=360; if(d<-180)d+=360;
  tarAng=Math.max(-360,Math.min(360,tarAng+d)); lastAng=n;
},{passive:false});
wheel.addEventListener("touchend",()=>{isTouchWheel=false;});

function setMotor(val,btn){
  if(cardanOn && val!==0) return;
  if(!isOnline && val!==0) return;
  motorSt = val;
  [btnAccel,btnRev,btnBrake].forEach(b=>b.classList.remove("pressed"));
  if(btn) btn.classList.add("pressed");
  updateLocks();
}
btnAccel.ontouchstart = e=>{e.preventDefault(); setMotor(1,btnAccel);};
btnRev.ontouchstart   = e=>{e.preventDefault(); setMotor(2,btnRev);};
btnBrake.ontouchstart = e=>{e.preventDefault(); setMotor(0,btnBrake);};
const handleRelease = ()=>{ if(!driveFast) setMotor(0); };
btnAccel.ontouchend = handleRelease;
btnRev.ontouchend   = handleRelease;

hazardBtn.onclick = ()=>{ hazardOn=!hazardOn; if(!hazardOn) setBtnStyle(hazardBtn,false); };
lowBtn.onclick = ()=>{ lowOn=!lowOn; setBtnStyle(lowBtn,lowOn,"#00838f","white","#00e5ff"); };
swDrive.onclick = ()=>{
  driveFast=!driveFast;
  swDrive.innerText="MODO: "+(driveFast?"RÁP":"LEN");
  setBtnStyle(swDrive,!driveFast,GREEN,"white","white");
};
cardanBtn.onclick = ()=>{
  if(motorSt!==0 || !isOnline) return;
  cardanOn=!cardanOn;
  cardanBtn.innerText="CARDÁN: "+(cardanOn?"ACOPLADO":"DESACOPL.");
  setBtnStyle(cardanBtn,cardanOn,GREEN,"white","white");
  updateLocks();
};
btnUp.onclick   = ()=>{ tolvaCmd='U'; tolvaMoving=true; setBtnStyle(btnUp,true,GREEN,"white","white"); updateLocks(); };
btnDown.onclick = ()=>{ tolvaCmd='D'; tolvaMoving=true; setBtnStyle(btnDown,true,GREEN,"white","white"); updateLocks(); };

function setConnState(online){
  if(online===isOnline) return;
  isOnline = online;
  if(online){ connStatus.innerText="ONLINE"; connStatus.style.color=GREEN; }
  else { connStatus.innerText="OFFLINE"; connStatus.style.color=RED; setMotor(0); }
  updateLocks();
}

setInterval(()=>{
  const url = `/c?m=${motorSt}&d=${Math.round(curAng)}&l=${lowOn?1:0}&h=${hazardOn?1:0}&c=${cardanOn?1:0}&t=${tolvaCmd}`;
  fetch(url).then(r=>r.text()).then(msg=>{
    tolvaCmd=' ';
    lastOkTime = Date.now();
    setConnState(true);
    // msg = "LISTO" | "SUBIENDO" | "BAJANDO"
    tolvaStatus.innerText = msg;
    if(msg==="SUBIENDO" || msg==="BAJANDO"){
      tolvaStatus.style.color = ORANGE;
      tolvaMoving = true;
    } else {
      tolvaStatus.style.color = "#888";
      if(tolvaMoving){
        setBtnStyle(btnUp,false);
        setBtnStyle(btnDown,false);
        tolvaMoving = false;
        updateLocks();
      }
    }
  }).catch(()=>{});
}, 100);

setInterval(()=>{
  if(Date.now() - lastOkTime > 500) setConnState(false);
}, 200);

function animate(){
  if(!isTouchWheel) tarAng *= 0.85;
  curAng += (tarAng - curAng) * 0.2;
  wheel.style.transform = `rotate(${curAng}deg)`;
  requestAnimationFrame(animate);
}
animate();
updateLocks();
</script>
</body></html>
)rawliteral";

// ============================================================
// HANDLER PRINCIPAL
// ============================================================
void handleControl() {
  lastCommandTime = millis();
  failsafeActive = false;

  int    nuevoMotorState = motorState;
  bool   nuevoCardan     = cardanAcoplado;
  char   tCmd            = ' ';

  if (server.hasArg("m")) nuevoMotorState = server.arg("m").toInt();
  if (server.hasArg("d")) d_visual        = server.arg("d").toFloat();
  if (server.hasArg("l")) lowLights       = (server.arg("l").toInt() == 1);
  if (server.hasArg("h")) hazard          = (server.arg("h").toInt() == 1);
  if (server.hasArg("c")) nuevoCardan     = (server.arg("c").toInt() == 1);
  if (server.hasArg("t")) {
    String s = server.arg("t");
    if (s.length() > 0) tCmd = s.charAt(0);
  }

  // --- Bloqueos cruzados (defensa en profundidad) ---
  // 1. Cardán solo cambia con motor detenido
  if (nuevoCardan != cardanAcoplado && motorState == 0) {
    cardanAcoplado = nuevoCardan;
  }

  // 2. Si cardán acoplado, motor de tracción bloqueado
  if (cardanAcoplado) nuevoMotorState = 0;

  motorState = nuevoMotorState;

  // 3. Comandos de tolva solo con cardán acoplado y motor detenido
  if ((tCmd == 'U' || tCmd == 'D') && cardanAcoplado && motorState == 0
      && tolvaEstado == T_LISTO) {
    if (tCmd == 'U') {
      stepper.moveTo(0);
      tolvaEstado = T_SUBIENDO;
    } else {
      stepper.moveTo(-PASOS_MAXIMOS);
      tolvaEstado = T_BAJANDO;
    }
  }

  // --- Aplicar dirección al servo ---
  int servoPos;
  if (d_visual < 0) servoPos = map((int)d_visual, -360, 0, MAX_IZQ, OFFSET_RECTO);
  else              servoPos = map((int)d_visual, 0, 360, OFFSET_RECTO, MAX_DER);
  direccion.write(servoPos);

  // --- Aplicar PWM al motor de tracción ---
  if (motorState == 1)      { analogWrite(pinMotorA, 255); analogWrite(pinMotorB, 0); }
  else if (motorState == 2) { analogWrite(pinMotorA, 0);   analogWrite(pinMotorB, 255); }
  else                      { analogWrite(pinMotorA, 0);   analogWrite(pinMotorB, 0); }

  // --- Responder con estado de tolva ---
  const char* estado;
  switch (tolvaEstado) {
    case T_SUBIENDO: estado = "SUBIENDO"; break;
    case T_BAJANDO:  estado = "BAJANDO";  break;
    default:         estado = "LISTO";    break;
  }
  server.send(200, "text/plain", estado);
}

// ============================================================
// LUCES
// ============================================================
void updateLights() {
  // --- Luz de freno con timer (1500ms) ---
  if (motorState == 0 && lastMotorState != 0) {
    brakeStartTime = millis();
    brakeActive = true;
  }
  if (brakeActive && (millis() - brakeStartTime >= BRAKE_DURATION_MS)) {
    brakeActive = false;
  }
  lastMotorState = motorState;

  digitalWrite(pinBajas,   lowLights ? HIGH : LOW);
  digitalWrite(pinFreno,   brakeActive ? HIGH : LOW);
  digitalWrite(pinReversa, (motorState == 2) ? HIGH : LOW);

  // --- Blink global ---
  if (millis() - lastBlink > BLINK_PERIOD_MS) {
    lastBlink = millis();
    blinkState = !blinkState;
  }

  // --- Giros / Balizas ---
  if (hazard) {
    digitalWrite(pinGiroIzq, blinkState ? HIGH : LOW);
    digitalWrite(pinGiroDer, blinkState ? HIGH : LOW);
  } else {
    digitalWrite(pinGiroIzq, (d_visual < -45 && blinkState) ? HIGH : LOW);
    digitalWrite(pinGiroDer, (d_visual >  45 && blinkState) ? HIGH : LOW);
  }
}

// ============================================================
// FAILSAFE
// ============================================================
void checkFailsafe() {
  if (millis() - lastCommandTime > FAILSAFE_TIMEOUT_MS) {
    if (!failsafeActive) {
      failsafeActive = true;
      // Cortar motor de tracción
      motorState = 0;
      analogWrite(pinMotorA, 0);
      analogWrite(pinMotorB, 0);
      // Apagar luces (excepto balizas, siguen parpadeando)
      lowLights = false;
      // Detener tolva si estaba moviéndose
      stepper.stop();
      // Servo queda en su última posición (no tocamos direccion.write())
    }
  }
}

// ============================================================
// TOLVA: detectar fin de movimiento
// ============================================================
void updateTolva() {
  stepper.run();
  if (tolvaEstado != T_LISTO && stepper.distanceToGo() == 0) {
    tolvaEstado = T_LISTO;
  }
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  // Servo
  direccion.setPeriodHertz(50);
  direccion.attach(pinServo, 500, 2400);
  direccion.write(OFFSET_RECTO);

  // Pines de salida
  pinMode(pinMotorA, OUTPUT);
  pinMode(pinMotorB, OUTPUT);
  pinMode(pinBajas, OUTPUT);
  pinMode(pinFreno, OUTPUT);
  pinMode(pinReversa, OUTPUT);
  pinMode(pinGiroIzq, OUTPUT);
  pinMode(pinGiroDer, OUTPUT);

  // Stepper
  stepper.setMaxSpeed(STEPPER_VMAX);
  stepper.setAcceleration(STEPPER_ACEL);
  stepper.setCurrentPosition(0);

  // WiFi AP
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  // Web server
  server.on("/", []() { server.send_P(200, "text/html", html); });
  server.on("/c", handleControl);
  server.begin();

  lastCommandTime = millis();
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  server.handleClient();
  updateLights();
  updateTolva();
  checkFailsafe();
}