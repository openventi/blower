#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <RCSwitch.h>
#include "config.h"

const char * ssid = STASSID;
const char * password = STAPSK;

const int pin_led = LED_BUILTIN;              // pin al cual se conecta el LED
int estado_led = HIGH;
int t_muestreo = 5000;        

const char *mqtt_server = "pccontrol.ml";
const int mqtt_port = 1883;
const char *mqtt_user = "web_client112";
const char *mqtt_pass = "123456";
String clientId = "esp32_B";
char * variable_id = "led1";

WiFiClient espClient;
PubSubClient client(espClient);

long last_msg = 0;
char msg[25];

// ------------------------- SE DECLARAN LAS FUNCIONES -------------------------
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();


// ----------------------------------- SETUP -----------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(pin_led, OUTPUT);                   // se inicializa el led del esp 
  digitalWrite(pin_led, estado_led);
  randomSeed(micros());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);   // se define el servidor mqtt
  client.setCallback(callback);               // se define el callback que se levanta al recibir informacion del servidor

  ArduinoOTA.setHostname("PCCONTROL-IOT");
  ArduinoOTA.setPassword("CLAVEPARAOTA");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { 
      type = "filesystem";
    }
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


// ------------------------------ CICLO PRINCIPAL ------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  long now = millis();
  if (now - last_msg > t_muestreo){
    last_msg = now;
    String to_send = String(estado_led);
    to_send.toCharArray(msg, 25);
    Serial.print("Publicamos mensaje: ");
    Serial.println(msg);
    client.publish("values", msg);
  }
}


// --------------------------------- FUNCIONES ---------------------------------
void callback(char* topic, byte* payload, unsigned int length){
  String incoming = "";
  Serial.print("Mensaje recibido desde: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++) {      // se itera cada byte y se lo convierte en char y posteriormente se concatena
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje: " + incoming);
  if ( incoming == "on") {
    estado_led = LOW;
    digitalWrite(pin_led, estado_led); 
  } else {
    estado_led = HIGH;
    digitalWrite(pin_led, estado_led); 
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando reconectar con el servidor MQTT...");
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      client.subscribe(variable_id);
    } else {
      Serial.print("Algo ha pasado, error: ");
      Serial.print(client.state());
      Serial.println("Reintentamos en 5 segundos");
      delay(2000);
    }
  }
}
