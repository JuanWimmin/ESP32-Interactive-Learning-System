// =============================================
// Proyecto Integrado ESP32 - Módulo 1 y Módulo 2
// Módulo 1: Secuencia de colores con LED RGB
// Módulo 2: Conteo de manzanas con pantalla OLED y DIP Switch
// Interfaz Web con registro de nombre, edad, selección de módulo y registro del último intento
// =============================================

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ======================== CONFIGURACIÓN ========================

// Pines LED RGB (ánodo común)
const int ledR = 33;
const int ledG = 25;
const int ledB = 26;

// Buzzer
const int buzzerPin = 14;

// Sensores capacitivos (Módulo 1)
const int sensorAmarillo = 36;
const int sensorRojo     = 32;
const int sensorAzul     = 34;
const int sensorVerde    = 35;
const int sensorNaranja  = 39;

// sensor sonico

const int trigPin = 13;
const int echoPin = 12;

// DIP Switch y botón (Módulo 2)
const int dipSwitch[6] = {23, 4, 2, 19, 18, 5};
const int botonVerificar = 27;

// OLED SH1106
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PWM canales LED
const int canalR = 0, canalG = 1, canalB = 2;

// WiFi
const char* ssid = "vivosis";
const char* password = "Dante1019";

// =================== ENUMS Y ESTRUCTURAS ===================

enum ModuloActivo { NINGUNO, COLORES, NUMEROS };
ModuloActivo moduloActivo = NINGUNO;

enum Color {AMARILLO, ROJO, AZUL, VERDE, NARANJA};

struct Registro {
  String nombre;
  int edad;
  String secuenciaColores;
  int aciertosColores;
  int fallosColores;
  int aciertosNumeros;
  int fallosNumeros;
};

Registro registroActual;

// =================== VARIABLES JUEGO ===================

const int longitudSecuencia = 5;
Color secuencia[longitudSecuencia];
int indiceUsuario = 0;
int aciertos = 0;
int fallos = 0;
bool esperandoRespuesta = false;
bool pruebaIniciada = false;

// Módulo 2
const int TOTAL_RONDAS = 5;
int rondaActual = 0;
int cantidadManzanas = 0;
bool esperandoVerificacion = true;

// =================== RED Y WEBSOCKET ===================

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
String logSerial = "";

void log(String mensaje) {
  Serial.println(mensaje);
  logSerial += mensaje + "\n";
  ws.textAll(mensaje);
}

// =================== FUNCIONES DE UTILIDAD ===================

long medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
  long distancia = duracion * 0.034 / 2;
  return distancia; // en centímetros
}


void apagarLED() {
  ledcWrite(canalR, 255);
  ledcWrite(canalG, 255);
  ledcWrite(canalB, 255);
}

void beepCorto() {
  digitalWrite(buzzerPin, HIGH);
  delay(150);
  digitalWrite(buzzerPin, LOW);
}

void beepError() {
  digitalWrite(buzzerPin, HIGH);
  delay(50);
  digitalWrite(buzzerPin, LOW);
  delay(50);
  digitalWrite(buzzerPin, HIGH);
  delay(50);
  digitalWrite(buzzerPin, LOW);
}

// =================== MÓDULO 1 - COLORES ===================

String nombreColor(Color c) {
  switch (c) {
    case AMARILLO: return "Amarillo";
    case ROJO:     return "Rojo";
    case AZUL:     return "Azul";
    case VERDE:    return "Verde";
    case NARANJA:  return "Naranja";
    default:       return "Desconocido";
  }
}

Color leerSensor() {
  if (digitalRead(sensorAmarillo) == HIGH) return AMARILLO;
  if (digitalRead(sensorRojo)     == HIGH) return ROJO;
  if (digitalRead(sensorAzul)     == HIGH) return AZUL;
  if (digitalRead(sensorVerde)    == HIGH) return VERDE;
  if (digitalRead(sensorNaranja)  == HIGH) return NARANJA;
  return (Color)(-1);
}

void mostrarColor(String nombreColor, int r, int g, int b, int duracion = 500) {
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
  Serial.println("🎨 " + nombreColor);
  log("🎨 " + nombreColor);
  ledcWrite(canalR, r);
  ledcWrite(canalG, g);
  ledcWrite(canalB, b);
  delay(duracion);
  apagarLED();
  delay(100);
}

void mostrarColorSecuencia(Color c, int duracion = 500) {
  if (moduloActivo != COLORES) return;
  switch (c) {
    case AMARILLO: mostrarColor("Amarillo", 0, 0, 255, duracion); break;
    case ROJO:     mostrarColor("Rojo", 0, 255, 255, duracion); break;
    case AZUL:     mostrarColor("Azul", 255, 255, 0, duracion); break;
    case VERDE:    mostrarColor("Verde", 255, 0, 255, duracion); break;
    case NARANJA:  mostrarColor("Naranja", 0, 127, 255, duracion); break;
  }
}

void generarSecuencia() {
  for (int i = 0; i < longitudSecuencia; i++) {
    secuencia[i] = (Color) random(0, 5);
  }
}

void mostrarSecuencia() {
  registroActual.secuenciaColores = "";
  log("🔔 Mostrando secuencia...");
  for (int i = 0; i < longitudSecuencia; i++) {
    mostrarColorSecuencia(secuencia[i]);
    registroActual.secuenciaColores += nombreColor(secuencia[i]);
    if (i < longitudSecuencia - 1) registroActual.secuenciaColores += ", ";
  }
}

// =================== MÓDULO 2 - NÚMEROS ===================

int leerDIP() {
  for (int i = 0; i < 6; i++) {
    if (digitalRead(dipSwitch[i]) == HIGH) return i + 1;
  }
  return 0;
}

void dibujarManzanas(int cantidad) {
  display.clearDisplay();
  for (int i = 0; i < cantidad; i++) {
    int x = 10 + (i % 3) * 40;
    int y = 20 + (i / 3) * 30;
    display.drawCircle(x, y, 10, SH110X_WHITE);
    display.drawLine(x, y - 10, x, y - 15, SH110X_WHITE);
  }
  display.display();
}

void nuevaRonda() {
  cantidadManzanas = random(1, 6);
  rondaActual++;
  esperandoVerificacion = true;
  log("🔁 Ronda " + String(rondaActual) + " - 🍎 Manzanas: " + String(cantidadManzanas));
  dibujarManzanas(cantidadManzanas);
}

void resumenFinalNumeros() {
  registroActual.aciertosNumeros = aciertos;
  registroActual.fallosNumeros = fallos;
  log("📊 Módulo Números:");
  log("✅ Aciertos: " + String(aciertos));
  log("❌ Fallos: " + String(fallos));

  String resumen = "REGISTRO|" + registroActual.nombre + "|" +
                 String(registroActual.edad) + "|" +
                 registroActual.secuenciaColores + "|" +
                 String(registroActual.aciertosColores) + "|" +
                 String(registroActual.fallosColores) + "|" +
                 String(registroActual.aciertosNumeros) + "|" +
                 String(registroActual.fallosNumeros);
ws.textAll(resumen);
}

void retroalimentacionFinal() {
    registroActual.aciertosColores = aciertos;
    registroActual.fallosColores = fallos;
  log("\n📊 RESULTADOS Colores:");
  log("✅ Aciertos: " + String(aciertos));
  log("❌ Fallos: " + String(fallos));
  log("----------------------");

  // Enviar como mensaje tipo REGISTRO al frontend
String resumen = "REGISTRO|" + registroActual.nombre + "|" +
                 String(registroActual.edad) + "|" +
                 registroActual.secuenciaColores + "|" +
                 String(registroActual.aciertosColores) + "|" +
                 String(registroActual.fallosColores) + "|" +
                 String(registroActual.aciertosNumeros) + "|" +
                 String(registroActual.fallosNumeros);
ws.textAll(resumen);
}


// === continua en el siguiente bloque ===


// =================== SETUP Y WEBSOCKET ===================

void setup() {

  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);
  randomSeed(analogRead(0));

  // PWM para LED RGB
  ledcSetup(canalR, 5000, 8); ledcAttachPin(ledR, canalR);
  ledcSetup(canalG, 5000, 8); ledcAttachPin(ledG, canalG);
  ledcSetup(canalB, 5000, 8); ledcAttachPin(ledB, canalB);

  // Pines
  pinMode(buzzerPin, OUTPUT);
  pinMode(botonVerificar, INPUT);
  pinMode(sensorAmarillo, INPUT);
  pinMode(sensorRojo, INPUT);
  pinMode(sensorAzul, INPUT);
  pinMode(sensorVerde, INPUT);
  pinMode(sensorNaranja, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  for (int i = 0; i < 6; i++) 
  pinMode(dipSwitch[i], INPUT);

  apagarLED();
  display.begin(OLED_ADDR, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(":) Sistema listo");
  display.display();

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  display.setCursor(0, 1);
  display.println(":) Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
    display.println(":");
  }
  Serial.println("\n✅ Conectado: " + WiFi.localIP().toString());
  display.println(" Conectado " + WiFi.localIP().toString());

  // WebSocket
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
      String msg = String((char*)data);
       
      if (msg == "start_colores") {
        long distancia = medirDistancia();
            if (distancia > 30 || distancia == 0) {
            log("🚫 No se detecta presencia cercana. No se puede iniciar la prueba.");
        return;
        }

        moduloActivo = COLORES;
        pruebaIniciada = true;
        esperandoRespuesta = true;
        aciertos = 0; fallos = 0; indiceUsuario = 0;
        generarSecuencia();
        mostrarSecuencia();
        log("🟢 Inició módulo de Colores.");
      } else if (msg == "start_numeros") {
        long distancia = medirDistancia();
            if (distancia >30 || distancia == 0) {
            log("🚫 No se detecta presencia cercana. No se puede iniciar la prueba.");
        return;
        }
        moduloActivo = NUMEROS;
        pruebaIniciada = true;
        esperandoVerificacion = true;
        aciertos = 0; fallos = 0; rondaActual = 0;
        nuevaRonda();
        log("🟢 Inició módulo de Números.");
      }
    }
  });
  server.addHandler(&ws);

  // Página HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>Interfaz Educativa</title>
<style>
body { font-family: sans-serif; background: #f0f0f0; padding: 20px; }
.card { background: white; border-radius: 12px; padding: 20px; box-shadow: 0 0 10px #ccc; max-width: 500px; margin: auto; }
input, button { padding: 10px; margin: 5px 0; width: 100%; border-radius: 6px; border: 1px solid #ccc; }
button { background: #007BFF; color: white; border: none; }
pre { background: #111; color: #0f0; padding: 10px; height: 200px; overflow-y: auto; }
</style></head>
<body>
<div class="card">
  <h2>Registro</h2>
  <input id="nombre" placeholder="Nombre del estudiante" />
  <input id="edad" type="number" placeholder="Edad" />
  <button onclick="iniciarModulo('colores')">Iniciar módulo de Colores</button>
  <button onclick="iniciarModulo('numeros')">Iniciar módulo de Números</button>
  <h3>🖥️ Consola:</h3>
  <pre id="output"></pre>
  <h3>📋 Último Registro:</h3>
  <ul id="registro">
    <li><b>Nombre:</b> -</li>
    <li><b>Edad:</b> -</li>
    <li><b>Secuencia de colores:</b> -</li>
    <li><b>Aciertos colores:</b> -</li>
    <li><b>Fallos colores:</b> -</li>
    <li><b>Aciertos números:</b> -</li>
    <li><b>Fallos números:</b> -</li>
  </ul>
</div>
<script>
let socket = new WebSocket("ws://" + location.host + "/ws");
socket.onmessage = e => {
  const out = document.getElementById("output");
  out.textContent += e.data + "\n";
  out.scrollTop = out.scrollHeight;
  if (event.data.startsWith("REGISTRO|")) {
  const partes = event.data.split("|");
  const [_, nombre, edad, secuencia, acCol, faCol, acNum, faNum] = partes;
  document.getElementById("registro").innerHTML = `
    <li><b>Nombre:</b> ${nombre}</li>
    <li><b>Edad:</b> ${edad}</li>
    <li><b>Secuencia de colores:</b> ${secuencia}</li>
    <li><b>Aciertos colores:</b> ${acCol}</li>
    <li><b>Fallos colores:</b> ${faCol}</li>
    <li><b>Aciertos números:</b> ${acNum}</li>
    <li><b>Fallos números:</b> ${faNum}</li>
  `;
}

};

function iniciarModulo(mod) {
  const nombre = document.getElementById("nombre").value;
  const edad = document.getElementById("edad").value;
  if (!nombre || !edad) return alert("Completa los datos.");
  fetch("/registro?nombre=" + nombre + "&edad=" + edad);
  socket.send("start_" + mod);
}
</script>
</body>
</html>)rawliteral");
  });

  // Registro
  server.on("/registro", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("nombre") && request->hasParam("edad")) {
      registroActual.nombre = request->getParam("nombre")->value();
      registroActual.edad = request->getParam("edad")->value().toInt();
      log("👤 Estudiante: " + registroActual.nombre + " (" + String(registroActual.edad) + ")");
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

// =================== LOOP GENERAL ===================

unsigned long lastTouchTime = 0;
const int debounceDelay = 300;

void loop() {

   //  Verificación continua de presencia
  static unsigned long alejamientoInicio = 0;

  if (pruebaIniciada) {
    long dist = medirDistancia();

    if (dist > 100 || dist == 0) {
      if (alejamientoInicio == 0) {
        alejamientoInicio = millis();
      } else if (millis() - alejamientoInicio > 3000) {
        log("⚠️ Usuario se alejó. Cancelando prueba.");
        pruebaIniciada = false;
        esperandoRespuesta = false;
        esperandoVerificacion = false;
        moduloActivo = NINGUNO;
        alejamientoInicio = 0;
      }
    } else {
      alejamientoInicio = 0;
    }
  }

  if (!pruebaIniciada) return;

  if (moduloActivo == COLORES && esperandoRespuesta) {
    unsigned long ahora = millis();
    if (ahora - lastTouchTime > debounceDelay) {
      Color tocado = leerSensor();
      if (tocado != (Color)(-1)) {
        lastTouchTime = ahora;
        mostrarColorSecuencia(tocado, 300);
        log("👆 Intento " + String(indiceUsuario + 1) + ": Tocó " + nombreColor(tocado));
        if (tocado == secuencia[indiceUsuario]) {
          aciertos++; beepCorto();
        } else {
          fallos++; beepError();
        }
        indiceUsuario++;
        if (indiceUsuario >= longitudSecuencia) {
          esperandoRespuesta = false;
          pruebaIniciada = false;

          log("✅ Módulo Colores terminado.");
          retroalimentacionFinal();
        }
      }
    }
  }

  if (moduloActivo == NUMEROS && pruebaIniciada && esperandoVerificacion) {
    if (digitalRead(botonVerificar) == HIGH) {
      delay(300);
      int seleccion = leerDIP();
      log("📥 Selección del usuario: " + String(seleccion));
      if (seleccion == cantidadManzanas) {
        log("✅ Correcto");
        beepCorto(); aciertos++;
      } else {
        log("❌ Incorrecto");
        beepError(); fallos++;
      }
      esperandoVerificacion = false;

      if (rondaActual < TOTAL_RONDAS) {
        delay(500);
        nuevaRonda();
      } else {
        pruebaIniciada = false;
        resumenFinalNumeros();
        log("✅ Módulo Números terminado.");
      }
    }
  }
}
