#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// ====== CONFIGURACIÓN BLE ======
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// ====== PINES Y VARIABLES DE SENSORES ======
const int pinSensor  = A0;
const int pinSensor1 = A1;

int valorSensor  = 0;
int valorSensor1 = 0;
int porcentajeHumedad = 0;

// ====== CALLBACKS BLE ======
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("Dispositivo BLE conectado");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("Dispositivo BLE desconectado");
    BLEDevice::startAdvertising();
    Serial.println("Reanunciando BLE...");
  }
};

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando sensores y BLE...");

  // --- Inicialización BLE ---
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT); // Solo BLE
  BLEDevice::init("ESP32S3 BLE Humedad");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ   |
    BLECharacteristic::PROPERTY_WRITE  |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->setValue("Esperando datos del sensor...");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE listo y anunciando");
}

// ====== LOOP ======
void loop() {
  // Lectura de sensores
  valorSensor  = analogRead(pinSensor);
  valorSensor1 = analogRead(pinSensor1);
  porcentajeHumedad = map(valorSensor, 1023, 0, 0, 100);

  // Texto de salida
  String mensaje = "Sensor1: " + String(valorSensor) + 
                   " | Sensor2: " + String(valorSensor1) + 
                   " | Humedad: " + String(porcentajeHumedad) + "%";

  // Mostrar por Serial
  Serial.println(mensaje);

  // Enviar por BLE (si está conectado)
  if (deviceConnected) {
    pCharacteristic->setValue(mensaje.c_str());
    pCharacteristic->notify();  //Notifica al celular
  }

  delay(1000);
}
