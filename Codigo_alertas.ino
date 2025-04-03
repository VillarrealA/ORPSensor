#ifdef USE_PULSE_OUT
  #include "orp_iso_grav.h"       
  Gravity_ORP_Isolated ORP = Gravity_ORP_Isolated(A0);         
#else
  #include "orp_grav.h"
  Gravity_ORP ORP = Gravity_ORP(A0);
#endif

#include <WiFiS3.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char SSID[]     = "LAB";
const char PASS[]     = "";
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

// variable a enviar
float orpValue;

// Definir el rango de ORP
const int ORP_MIN = 450;
const int ORP_MAX = 650;
const int ORP_ALERT_THRESHOLD = 50; // Umbral de variación inusual

int previousOrpValue = 650; // Valor inicial para detección de variaciones

void onOrpValueChange();

// Función para convertir una cadena a mayúsculas
void strToUpper(char* str) {
  while (*str) {
    *str = toupper(*str);
    str++;
  }
}

void parse_cmd(char* string) {
  strToUpper(string);
  String cmd = String(string);
  if (cmd.startsWith("CAL")) {
    int index = cmd.indexOf(',');
    if (index != -1) {
      String param = cmd.substring(index + 1, cmd.length());
      if (param.equals("CLEAR")) {
        ORP.cal_clear();
        Serial.println("CALIBRATION CLEARED");
      } else {
        int cal_param = param.toInt();
        ORP.cal(cal_param);
        Serial.println("ORP CALIBRATED");
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println(F("Use command \"CAL,xxx\" to calibrate the circuit to the value xxx \n\"CAL,CLEAR\" clears the calibration"));
  
  if (ORP.begin()) {
    Serial.println("Loaded EEPROM");
  }
  
  // Configurar conexión a la nube
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  ArduinoCloud.addProperty(orpValue, READ, ON_CHANGE, onOrpValueChange);
}

void loop() {
  ArduinoCloud.update();
  
  if (Serial.available() > 0) {
    char user_data[32];
    Serial.readBytesUntil(13, user_data, sizeof(user_data));
    parse_cmd(user_data);
  }
  
  int currentOrp = (int)ORP.read_orp();
  Serial.println(currentOrp);
  orpValue = currentOrp;
  
  // Verificar alertas
  if (abs(currentOrp - previousOrpValue) >= ORP_ALERT_THRESHOLD) {
    Serial.println("ALERTA: Cambio inusual en la lectura de ORP");
  }
  if (currentOrp <= ORP_MIN + 20 && currentOrp > ORP_MIN) {
    Serial.println("PRECAUCIÓN: ORP acercándose al límite mínimo");
  }
  if (currentOrp < ORP_MIN) {
    Serial.println("ALERTA CRÍTICA: ORP por debajo del mínimo recomendado");
  }
  
  previousOrpValue = currentOrp;
  delay(1000);
}

void onOrpValueChange() {
  Serial.println("Datos enviados a la Nube de Arduino");
}
