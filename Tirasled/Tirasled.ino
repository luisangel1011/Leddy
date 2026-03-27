
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

class procesos{
  private:
  int R,G,B;
  String Funcion;
  public:
   procesos(int r, int g, int b, String funcion) 
      : R(r), G(g), B(b), Funcion(funcion) {}
  int getRed(){return R;}
  int getGreen(){return G;}
  int getBlue(){return B;}
  String getFun(){return Funcion;}
};

// UUIDs de servicio c:\Users\luisa\Desktop\Tirasledy característica (puedes generar otros en uuidgenerator.net)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"
String Arreglo[4];
procesos* P= NULL;
int tam=0;
int cont=0;
unsigned long contador=millis();

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
    BLEDevice::startAdvertising();
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
      while (end > -1) {
        Arreglo[tam++] = rxValue.substring(start, end);
        start = end + 1;
        end = rxValue.indexOf(',', start);
      }
      Arreglo[tam++] = rxValue.substring(start);
      if(Arreglo[0]!="" && Arreglo[1]!="" && Arreglo[2]!=""){
        P= new procesos(Arreglo[0].toInt(), Arreglo[1].toInt(),Arreglo[2].toInt(),Arreglo[3]);
      }
      else 
        P= new procesos(0, 0,0,Arreglo[3]);
      
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
  
  if(P!=NULL){
    String comando = P->getFun();  // Último valor recibido

    if (comando == "aleatoriedad") {
      
      aleatoriedad();
    } 
    else if (comando == "solido") {
      solido( ((P->getRed()*100)/255), ((P->getGreen()*100)/255), ((P->getBlue()*100)/255) );
      P=NULL;
    } 
    else if (comando == "desvanecimiento") {
      desvanecimiento(((P->getRed()*100)/255), ((P->getGreen()*100)/255), ((P->getBlue()*100)/255));
    }
    else if (comando == "rastro") {
      rastro(((P->getRed()*100)/255), ((P->getGreen()*100)/255), ((P->getBlue()*100)/255));
    }
    else {
      Serial.println("Comando no reconocido");
    }
    
  
  }
  
      


}

 void aleatoriedad(){
    

    if(millis()-contador>=20){
      FastLED.clear();
      FastLED.show();
      // 🔹 Intentar leer como formato R,G,B
      fill_solid(leds, NUM_LEDS, CRGB(random(0,101), random(0,101), random(0,101)));
      FastLED.show();
      contador=millis();
    }
    P= new procesos(0,0,0,"aleatoriedad");
    
  }

  void solido(int r,int g,int b){
    // 🔹 Intentar leer como formato R,G,B
          
          fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
          FastLED.show();
        
  }

  void desvanecimiento(int r,int g,int b) {
    
    FastLED.clear();
    FastLED.show();
    // 🔹 Intentar leer como formato R,G,B
    Serial.printf("Color recibido -> R:%d, G:%d, B:%d\n", r, g, b);
    fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
    FastLED.show();
    
    if(millis()-contador>=20 && (r>=0 && g>=0 && b>0)){
      contador=millis();
      P= new procesos(((r/100)*255)-10, ((g/100)*255)-10,((b/100)*255)-10,"desvanecimiento");
    }
    else if (r>=0 && g>=0 && b>0){
      P= new procesos(random(0,256), random(0,256), random(0,256),"desvanecimiento");
      
    }
    else
    P= new procesos(((r/100)*255), ((g/100)*255),((b/100)*255),"desvanecimiento");
    
    
  }


