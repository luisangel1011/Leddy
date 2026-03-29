#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <FastLED.h>

// ---------------- LED ----------------
#define NUM_LEDS 60        
#define DATA_PIN1 5        
#define COLOR_ORDER GRB    
#define LED_TYPE WS2811    

CRGB leds[NUM_LEDS];

// ---------------- CLASE ----------------
class procesos{
  private:
    int R,G,B;
    String Funcion;
  public:
    procesos(int r=0, int g=0, int b=0, String funcion="") {
      R=r; G=g; B=b; Funcion=funcion;
    }
    int getRed(){return R;}
    int getGreen(){return G;}
    int getBlue(){return B;}
    String getFun(){return Funcion;}
};

// ---------------- BLE ----------------
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

String Arreglo[4];
int tam=0;

procesos P;
String modoActual = "";

unsigned long contador;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// ---------------- CALLBACKS ----------------
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Cliente BLE conectado");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Cliente BLE desconectado");
    BLEDevice::startAdvertising();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {

  void onWrite(BLECharacteristic *pCharacteristic) {
    
    tam = 0;
    String rxValue = pCharacteristic->getValue().c_str();

    if (rxValue.length() > 0) {
      Serial.print("Dato recibido: ");
      Serial.println(rxValue);

      int start = 0;
      int end = rxValue.indexOf(',');

      while (end > -1 && tam < 4) {
        Arreglo[tam++] = rxValue.substring(start, end);
        start = end + 1;
        end = rxValue.indexOf(',', start);
      }

      if (tam < 4) {
        Arreglo[tam++] = rxValue.substring(start);
      }

      if (tam >= 4) {
        P = procesos(
          Arreglo[0].toInt(),
          Arreglo[1].toInt(),
          Arreglo[2].toInt(),
          Arreglo[3]
        );

        modoActual = Arreglo[3]; // 🔥 clave
      }
    }
  }
};

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, DATA_PIN1, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  contador = millis();

  BLEDevice::init("ESP32C3_BT");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEDevice::startAdvertising();

  Serial.println("ESP listo");
}

// ---------------- LOOP ----------------
void loop() {

  if (!deviceConnected) {
    delay(200);
    return;
  }

  if (modoActual == "aleatoriedad") {
    aleatoriedad();
  }
  else if (modoActual == "solido") {
    solido(P.getRed(), P.getGreen(), P.getBlue());
  }
  else if (modoActual == "desvanecer") {
    desvanecimiento(P.getRed(), P.getGreen(), P.getBlue());
  }
}

// ---------------- FUNCIONES ----------------

void aleatoriedad() {

  if (millis() - contador >= 600) {
    contador = millis();

    fill_solid(leds, NUM_LEDS, CRGB(
      random(0,256),
      random(0,256),
      random(0,256)
    ));

    FastLED.show();
  }
}

void solido(int r,int g,int b) {
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  FastLED.show();
}

void desvanecimiento(int r,int g,int b) {

  // color base
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));

  if (millis() - contador >= 500) {
    contador = millis();

    for(int i=0; i<NUM_LEDS; i++){
      leds[i].fadeToBlackBy(10); // 🔥 efecto fade real
    }

    FastLED.show();
  }
}