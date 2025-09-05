
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <FastLED.h>
#include <Preferences.h>

// Parámetros de la tira
#define NUM_LEDS 60        
#define DATA_PIN1 5        // Pin conectado a "Data"
#define COLOR_ORDER GRB    
#define LED_TYPE WS2811    

CRGB leds[NUM_LEDS];

// UUIDs de servicio c:\Users\luisa\Desktop\Tirasledy característica (puedes generar otros en uuidgenerator.net)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"
String Arreglo[10];
int tam=0;
int cont=0;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callbacks para conexión BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Cliente BLE conectado");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Cliente BLE desconectado");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
    


  void onWrite(BLECharacteristic *pCharacteristic) {
    
    tam=0;
    String rxValue = pCharacteristic->getValue().c_str();
    
    if (rxValue.length() > 0) {
      Serial.print("Dato recibido por BLE: ");
      Serial.println(rxValue);
      int start = 0;
      int end = rxValue.indexOf(',');
      while (end != -1) {
        Arreglo[tam++] = rxValue.substring(start, end);
        start = end + 1;
        end = rxValue.indexOf(',', start);
      }
      Arreglo[tam++] = rxValue.substring(start);

     

      
      
      
    }
  }

 
};



void setup() {
  Serial.begin(115200);

  // Configura FastLED
  FastLED.addLeds<LED_TYPE, DATA_PIN1, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // Inicializa BLE
  BLEDevice::init("ESP32C3_BT");  // Nombre que verás en tu celular
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Esperando datos...");
  pService->start();

  // Empezar publicidad
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("ESP32-C3 listo. Busca 'ESP32C3_BT'.");
}

void loop() {
  if (!deviceConnected) {
    Serial.println("Esperando conexión...");
    delay(2000);
  } 
 
  if(tam>0){
      String comando = Arreglo[tam - 1];  // Último valor recibido

      if (comando == "arcoiris") {
        
        
        arcoiris();
      } 
      else if (comando == "solido") {
        solido( ((Arreglo[0].toInt()*100)/255), ((Arreglo[1].toInt()*100)/255), ((Arreglo[2].toInt()*100)/255) );
      } 
      else if (comando == "desvanecimiento") {
        desvanecimiento(((Arreglo[0].toInt()*100)/255), ((Arreglo[1].toInt()*100)/255), ((Arreglo[2].toInt()*100)/255));
      }
      else if (comando == "rastro") {
        rastro(((Arreglo[0].toInt()*100)/255), ((Arreglo[1].toInt()*100)/255), ((Arreglo[2].toInt()*100)/255));
      }
      else {
        Serial.println("Comando no reconocido");
      }
  }
      


}

 void arcoiris(){
    EVERY_N_MILLISECONDS(20){
      if (cont>255)
        cont=0;


      fill_rainbow(leds, NUM_LEDS, cont++);
      FastLED.show();
    }
    
  }

  void solido(int r,int g,int b){
    // 🔹 Intentar leer como formato R,G,B
          Serial.printf("Color recibido -> R:%d, G:%d, B:%d\n", r, g, b);
          fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
          FastLED.show();
        
  }
  void rastro(int r,int g,int b){
    FastLED.clear();
    FastLED.show();
    EVERY_N_MILLISECONDS(200){
      if (cont>255)
        cont=0;
      for(int j=cont; j< cont+3; j++)
        leds[j] = CRGB(r, g, b);
      FastLED.show();
      cont++;
    }
   
    
  }

  void desvanecimiento(int r,int g,int b) {
    for (int i = 0; i < NUM_LEDS; i++) 
      leds[i] = CRGB(r, g, b);
    FastLED.show();
    

    EVERY_N_MILLISECONDS(200){
      fadeToBlackBy(leds, NUM_LEDS, 10);
      FastLED.show();
    }
     
    
  }


