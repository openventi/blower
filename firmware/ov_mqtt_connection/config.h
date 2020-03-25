#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "nombre_de_la_red_wifi"
#define STAPSK  "contrasena_de_la_red"
#endif

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(STASSID);
  WiFi.begin(STASSID, STAPSK);
  
  while (WiFi.waitForConnectResult()  != WL_CONNECTED) {    // se espera indefinidamente hasta que exista coneccion
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}
