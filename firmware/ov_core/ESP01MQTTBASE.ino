  //#include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <RCSwitch.h>

#ifndef STASSID
#define STASSID "SSD WIFI"
#define STAPSK  "CLAVE WIFI"
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

/*
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*/

//int sensor = 13;  // Digital pin D7
//#define GPIO2 2
  const int pinPulsador = 0;         // pin al cual se conecta el pulsador
const int pinLed = 2;              // pin al cual se conecta el LED


int a;
int b;

int contadorPulsaciones = 0;        // Contador: variable que acumula el numero de pulsaciones
int estadoPulsador = 0;             // Variable que almacena el estado actual del pulsador
int estadoAnteriorPulsador = 0;     // Variable que almacena el estado anterior del pulsador


 const char *mqtt_server = "pccontrol.ml";
 // const char *mqtt_server = "18.229.245.154";
  const int mqtt_port = 1883;
  const char *mqtt_user = "web_client112";
  const char *mqtt_pass = "123456";

  WiFiClient espClient;
  PubSubClient client(espClient);

  long lastMsg = 0;
  char msg[25];


  char msg1[25];
  char msg2[25];
  
  int temp1 = 0;
  int temp2 = 1;
  int volts = 2;

  //*****************************
  //*** DECLARACION FUNCIONES ***
  //*****************************
  void setup_wifi();
  void callback(char* topic, byte* payload, unsigned int length);
  void reconnect();
 
  void setup() {
   
   //  mySwitch.enableReceive(0);  // pin #2 de arduino como RX

 //    pinMode(sensor, INPUT);   // declare sensor as input
 //    pinMode(GPIO2, OUTPUT);   // declare sensor as input
 pinMode(pinPulsador, INPUT);       // Inicializa el pin del pulsador como entrada
  pinMode(pinLed, OUTPUT);           // Inicializa el pin del LED como salida

 // pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  randomSeed(micros());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  a=0;

   

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("PCCONTROL-IOT");
  // No authentication by default
   ArduinoOTA.setPassword("CLAVEPARAOTA");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
if (!client.connected()) {
reconnect();
}

client.loop();

long now = millis();
if (now - lastMsg > 5000){
lastMsg = now;
temp1++;
temp2++; 
volts++;

 


String to_send = String(temp1) + "," + String(temp2) + "," + String(volts);
to_send.toCharArray(msg, 25);
Serial.print("Publicamos mensaje -> ");

Serial.println(msg);
client.publish("values", msg);

 




}

 estadoPulsador = digitalRead(pinPulsador);      // Se lee y almacena el estado del pulsador

  if (estadoPulsador != estadoAnteriorPulsador) { // Se compara la variable estadoPulsador 
                                                  //// con su valor anterior
    if (estadoPulsador == HIGH) {                 // Si el estado cambió, se incrementa el contador
                                                  // Si el estado actual es HIGH entonces el pulsador
      contadorPulsaciones++;                      //// pasó de un estado OFF a ON
      Serial.println("Encendido");
      Serial.print("numero de pulsaciones:  ");
      Serial.println(contadorPulsaciones);
      String to_send = String("ACTIVADO");
to_send.toCharArray(msg1, 25);
Serial.print("Publicamos mensaje -> ");
Serial.println(msg1);
client.publish("boton", msg1);

    } 
    else {                                        // Si el estado actual es LOW entonces el pulsador
      Serial.println("Apagado");                  //// paso de un estado ON a OFF
      String to_send = String("DESACTIVADO");
to_send.toCharArray(msg2, 25);
Serial.print("Publicamos mensaje -> ");
Serial.println(msg2);
client.publish("boton", msg2);

    }
  }
                                                  // Se guarda el estado actual como ultimo estado
  estadoAnteriorPulsador = estadoPulsador;        //// para la siguiente vuelta de la función loop     
  
  if (contadorPulsaciones % 4 == 0) {             // Enciende el LED una vez cada 4 pulsaciones 
    digitalWrite(pinLed, HIGH);                   //// calculando el módulo del contador de pulsaciones.
  } else {                                        // La función módulo calcula el resto de una división 
   digitalWrite(pinLed, LOW);                     //// entre dos números. Si el resto es cero, significa
  }                                               //// que el dividendo es múltiplo del divisor.


}



//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
delay(10);
// Nos conectamos a nuestra red Wifi
Serial.println();
Serial.print("Conectando a ");
Serial.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.waitForConnectResult()  != WL_CONNECTED) {
delay(500);
Serial.print(".");

}

Serial.println("");
Serial.println("Conectado a red WiFi!");
Serial.println("Dirección IP: ");
Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* payload, unsigned int length){
String incoming = "";
Serial.print("Mensaje recibido desde -> ");
Serial.print(topic);
Serial.println("");
for (int i = 0; i < length; i++) {
incoming += (char)payload[i];
}
incoming.trim();
Serial.println("Mensaje -> " + incoming);

if ( incoming == "on") {
//digitalWrite(GPIO2, HIGH);
  digitalWrite(pinLed, HIGH); 
} else {
//digitalWrite(GPIO2, LOW);
  digitalWrite(pinLed, LOW); 
}
}

void reconnect() {

while (!client.connected()) {
Serial.print("Intentando conexión Mqtt...");
// Creamos un cliente ID
String clientId = "esp32_";

//clientId += String(random(0xffff), HEX);
clientId += String("B");
// Intentamos conectar
if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
Serial.println("Conectado!");
// Nos suscribimos
client.subscribe("led1");
client.subscribe("led2");
} else {
Serial.print("falló :( con error -> ");
Serial.print(client.state());
Serial.println(" Intentamos de nuevo en 5 segundos");

delay(2000);
}
}
}
