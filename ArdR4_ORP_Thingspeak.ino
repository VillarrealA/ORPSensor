/*  ORP → ThingSpeak cada 30 minutos
    Placa: Arduino UNO R4 WiFi
    Sensor: Atlas Scientific Surveyor™ Analog ORP a A0
    Aislador por pulsos (opcional): descomenta USE_PULSE_OUT y conecta a un pin digital.

    Calibración por Serial (9600 baud):
      CAL,225     // calibra con buffer de 225 mV
      CAL,CLEAR   // limpia calibración

    Envío HTTP GET: https://api.thingspeak.com/update?api_key=WRITE_KEY&field1=valor
*/

#include <WiFiS3.h>

// ───── Config Wi-Fi ─────────────────────────────────────────────────────────────
const char* WIFI_SSID     = "Totalplay-4C61";
const char* WIFI_PASSWORD = "4C617F426us926TT";
const int orpMin = -650;
const int orpMax = 800;

// ───── Config ThingSpeak ────────────────────────────────────────────────────────
const char* TS_HOST   = "api.thingspeak.com";
const int   TS_PORT   = 80;
const char* TS_WRITE_API_KEY = "N3CMWQBVF3CCD8YG";   // p.ej. ABCDEFGHIJK12345
// Opcional: canal/campos adicionales -> agrega &field2=... etc.

// ───── Lectura ORP (Atlas Surveyor) ────────────────────────────────────────────
// Para usar aislador con salida por pulsos, descomenta la línea siguiente:
//// #define USE_PULSE_OUT

#ifdef USE_PULSE_OUT
  #include "orp_iso_surveyor.h"
  // Si usas aislador por pulsos, puedes usar un pin digital (aquí A0 por facilidad).
  Surveyor_ORP_Isolated ORP(A0);
#else
  #include "orp_surveyor.h"
  // Entrada analógica estándar del Surveyor ORP
  Surveyor_ORP ORP(A0);
#endif

// ───── Temporización ───────────────────────────────────────────────────────────
const unsigned long MEASUREMENT_PERIOD_MS = 5UL * 60UL * 1000UL; // 30 minutos
unsigned long lastSend = 0;

// ───── Buffer para comandos de calibración por Serial ──────────────────────────
uint8_t user_bytes_received = 0;
const uint8_t bufferlen = 32;
char user_data[bufferlen];

// ───── Utilidades ──────────────────────────────────────────────────────────────
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Conectando a Wi-Fi: ");
  Serial.print(WIFI_SSID);
  int status;
  unsigned long t0 = millis();
  while ((status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD)) != WL_CONNECTED) {
    Serial.print(".");
    delay(1500);
    if (millis() - t0 > 30000) { // 30 s
      Serial.println("\nReintentando Wi-Fi...");
      t0 = millis();
    }
  }
  Serial.print("\nWi-Fi OK. IP: ");
  Serial.println(WiFi.localIP());
}

bool sendToThingSpeak(long orp_mV, int alarmFlag) {
  // Construye la URL
  String url = "/update?api_key=" + String(TS_WRITE_API_KEY) +
             "&field1=" + String(orp_mV) +
             "&field2=" + String(alarmFlag);   // 0=OK, 1=ALERTA
  WiFiClient client;
  if (!client.connect(TS_HOST, TS_PORT)) {
    Serial.println("Error conectando a ThingSpeak");
    return false;
  }

  // Petición HTTP 1.1
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + TS_HOST + "\r\n" +
               "Connection: close\r\n\r\n");

  // Lee breve respuesta (opcional)
  unsigned long t0 = millis();
  while (client.connected() && millis() - t0 < 3000) {
    while (client.available()) {
      String line = client.readStringUntil('\n');
      // Puedes imprimirla si quieres depurar:
      // Serial.println(line);
    }
  }
  client.stop();
  Serial.println("ThingSpeak: enviado " + String(orp_mV) + " mV");
  return true;
}

void parse_cmd(char* string) {
  // Igual que el ejemplo oficial de Atlas (CAL,xxx / CAL,CLEAR)
  // Ver: orp_surveyor_example.ino
  // Convierte a MAYÚSCULAS
  for (char* p = string; *p; ++p) *p = toupper(*p);

  String cmd = String(string);
  if (cmd.startsWith("CAL")) {
    int index = cmd.indexOf(',');
    if (index != -1) {
      String param = cmd.substring(index + 1);
      param.trim();
      if (param.equals("CLEAR")) {
        ORP.cal_clear();
        Serial.println("CALIBRACION LIMPIADA");
      } else {
        int cal_param = param.toInt();
        ORP.cal(cal_param);
        Serial.println("ORP CALIBRADO a " + String(cal_param) + " mV");
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(300);

  Serial.println(F("ORP → ThingSpeak (UNO R4 WiFi)"));
  Serial.println(F("Comandos Serial: CAL,225  |  CAL,CLEAR"));
  // Intenta cargar calibración desde EEPROM (si existe)
  if (ORP.begin()) {
    Serial.println("Calibracion cargada de EEPROM.");
  } else {
    Serial.println("Sin calibracion previa (EEPROM vacia).");
  }

  connectWiFi();
  lastSend = millis() - MEASUREMENT_PERIOD_MS; // fuerza 1er envío inmediato
}

void loop() {
  // Escucha comandos por Serial (calibración)
  if (Serial.available() > 0) {
    user_bytes_received = Serial.readBytesUntil(13, user_data, sizeof(user_data));
  }
  if (user_bytes_received) {
    parse_cmd(user_data);
    user_bytes_received = 0;
    memset(user_data, 0, sizeof(user_data));
  }

  // Cada 30 minutos
  if (millis() - lastSend >= MEASUREMENT_PERIOD_MS) {
    connectWiFi();  // Garantiza conexión
    // Lee ORP (mV). La librería promedia internamente y aplica: ORP = V_mV - 1500 - offset
    long orp_mV = (long)ORP.read_orp();
    Serial.print("ORP = ");
    Serial.print(orp_mV);
    Serial.println(" mV");

    // 2) Calcular bandera de alarma (0=OK, 1=ALERTA)
    int alarmFlag = (orp_mV < orpMin || orp_mV > orpMax) ? 1 : 0;

    // Envía a ThingSpeak
    if (sendToThingSpeak(orp_mV, alarmFlag)) {
      lastSend = millis();
    } else {
      // En caso de fallo, reintenta pronto (2 minutos)
      lastSend = millis() - (MEASUREMENT_PERIOD_MS - 120000UL);
    }
  }

  // Pequeño respiro (el R4 no duerme profundo aquí; si quieres ultra-bajo consumo,
  // luego integramos RTC + modo standby entre muestras)
  delay(250);
}

