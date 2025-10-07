#include <ArduinoJson.hpp>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>  // Para enviar los datos en JSON

// ---------------- CONFIGURACIÓN DE PINES ----------------
#define PIN_SENSOR_HUMEDAD 34
#define PIN_SENSOR_CONDUCTIVIDAD 35
#define PIN_SENSOR_TEMPERATURA 32
#define PIN_SENSOR_PH 33

// Pines LoRa (SX1276/SX1278)
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26
#define LORA_BAND 915E6  // Frecuencia 433 MHz

// ---------------- ESTRUCTURA DE DATOS ----------------
struct DatosSensores {
  float humedad;
  float conductividad;
  float temperatura;
  float ph;
};

// ---------------- CLASE SENSOR BASE ----------------
class Sensor {
public:
  virtual float leer() = 0;
};

// ---------------- CLASES DE SENSORES ----------------
class SensorHumedad : public Sensor {
private:
  int pin;
public:
  SensorHumedad(int p)
    : pin(p) {}
  float leer() override {
    int val = analogRead(pin);
    return map(val, 0, 4095, 0, 100);  // Escalar a 0-100%
  }
};

class SensorConductividad : public Sensor {
private:
  int pin;
public:
  SensorConductividad(int p)
    : pin(p) {}
  float leer() override {
    int val = analogRead(pin);
    return (float)val * 0.1;  // Ejemplo de conversión
  }
};

class SensorTemperatura : public Sensor {
private:
  int pin;
public:
  SensorTemperatura(int p)
    : pin(p) {}
  float leer() override {
    int val = analogRead(pin);
    return (val / 4095.0) * 100.0;  // Escalar a 0-100°C (ejemplo)
  }
};

class SensorPH : public Sensor {
private:
  int pin;
public:
  SensorPH(int p)
    : pin(p) {}
  float leer() override {
    int val = analogRead(pin);
    return (val / 4095.0) * 14.0;  // Escalar a 0-14 (pH)
  }
};

// ---------------- OBJETOS DE SENSORES ----------------
SensorHumedad sensorHumedad(PIN_SENSOR_HUMEDAD);
SensorConductividad sensorCE(PIN_SENSOR_CONDUCTIVIDAD);
SensorTemperatura sensorTemp(PIN_SENSOR_TEMPERATURA);
SensorPH sensorPh(PIN_SENSOR_PH);

// ---------------- FUNCIONES ----------------
DatosSensores leerSensores() {
  DatosSensores datos;
  datos.humedad = sensorHumedad.leer();
  datos.conductividad = sensorCE.leer();
  datos.temperatura = sensorTemp.leer();
  datos.ph = sensorPh.leer();
  return datos;
}

void enviarPorLoRa(DatosSensores datos) {
  // Convertir a JSON
  StaticJsonDocument<200> doc;
  doc["humedad"] = datos.humedad;
  doc["conductividad"] = datos.conductividad;
  doc["temperatura"] = datos.temperatura;
  doc["ph"] = datos.ph;

  char buffer[200];
  serializeJson(doc, buffer);

  LoRa.beginPacket();
  LoRa.print(buffer);
  LoRa.endPacket();
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("Inicio de LoRa");

  // Inicializar LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("Error iniciando LoRa!");
    while (1)
      ;
  }
  Serial.println("LoRa iniciado correctamente.");
}

// ---------------- LOOP ----------------
void loop() {
  DatosSensores datos = leerSensores();

  // Mostrar en serial
  Serial.print("Humedad: ");
  Serial.println(datos.humedad);
  Serial.print("Conductividad: ");
  Serial.println(datos.conductividad);
  Serial.print("Temperatura: ");
  Serial.println(datos.temperatura);
  Serial.print("pH: ");
  Serial.println(datos.ph);

  // Enviar por LoRa
  enviarPorLoRa(datos);

  delay(5000);  // cada 5 segundos
}
