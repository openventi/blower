// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2020
// MIT License
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
// https://arduinojson.org/v6/example/generator/

#include <ArduinoJson.h>
ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);
 


 
  long lastMsg = 0;
uint8_t pin_led = 2;
char* ssid = "RED-WIFI";
char* password = "clavered1";

  int temp1 = 0;
  int temp2 = 1;
  int volts = 2;
  
char webpage[] PROGMEM = R"=====(
<html>
<head>
  <script>
    var Socket;
    function init() {
      Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      Socket.onmessage = function(event){
        document.getElementById("rxConsole").value += event.data;
      }
    }
    function sendText(){
      Socket.send(document.getElementById("txBar").value);
      document.getElementById("txBar").value = "";
    }
    function sendBrightness(){
      Socket.send("#"+document.getElementById("brightness").value);
    }    
  </script>
</head>
<body onload="javascript:init()">
  <div>
    <textarea id="rxConsole"></textarea>
  </div>
  <hr/>
  <div>
    <input type="text" id="txBar" onkeydown="if(event.keyCode == 13) sendText();" />
  </div>
  <hr/>
  <div>
    <input type="range" min="0" max="1023" value="512" id="brightness" oninput="sendBrightness()" />
  </div>  
</body>
</html>
)=====";

void setup()
{
     // Allocate the JSON document
  //
  // Inside the brackets, 200 is the RAM allocated to this document.
  // Don't forget to change this value to match your requirement.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<200> doc;
  // StaticJsonObject allocates memory on the stack, it can be
  // replaced by DynamicJsonDocument which allocates in the heap.
  //
  // DynamicJsonDocument  doc(200);

  // Add values in the document
  //
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;
    // Add an array.
  //
  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  // Generate the minified JSON and send it to the Serial port.
  //
  serializeJson(doc, Serial);
  // The above line prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

  // Start a new line
    Serial.println();

    // Generate the prettified JSON and send it to the Serial port.
  //
  serializeJsonPretty(doc, Serial);
  // The above line prints:
  // {
  //   "sensor": "gps",
  //   "time": 1351824120,
  //   "data": [
  //     48.756080,
  //     2.302038
  //   ]
  // }
  pinMode(pin_led, OUTPUT);
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",[](){
    server.send_P(200, "text/html", webpage);  
  });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
   long now = millis();
  webSocket.loop();
  server.handleClient();
  if(Serial.available() > 0){
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }

 
if (now - lastMsg > 5000){
lastMsg = now;
temp1++;
temp2++; 
volts++;

   StaticJsonDocument<200> doc;
  // StaticJsonObject allocates memory on the stack, it can be
  // replaced by DynamicJsonDocument which allocates in the heap.
  //
  // DynamicJsonDocument  doc(200);

  // Add values in the document
  //
  doc["sensor"] = temp1;
  doc["time"] = volts;
    // Add an array.
  //
  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  // Generate the minified JSON and send it to the Serial port.
  //
 // serializeJson(doc, Serial);
  // The above line prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

  // Start a new line
//    Serial.println();

    // Generate the prettified JSON and send it to the Serial port.
  //
  serializeJsonPretty(doc, Serial);
 //char c[] = {(char)temp1};
// char cadena[] = "{hola}";
String cadena="";
 serializeJson(doc, cadena);
 //char *c = cadena.c_str();
 // char *c = cadena.c_str();
  webSocket.broadcastTXT(cadena.c_str());

}

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT){
    if(payload[0] == '#'){
      uint16_t brightness = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
      brightness = 1024 - brightness;
      analogWrite(pin_led, brightness);
      Serial.print("brightness= ");
      Serial.println(brightness);
    }

    else{
      for(int i = 0; i < length; i++)
        Serial.print((char) payload[i]);
      Serial.println();
    }
  }
  
}
