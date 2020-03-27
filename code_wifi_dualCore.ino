#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <WiFi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <HTTPClient.h>
#include "IIS3DHHC.h"
#include <math.h>

//POR FAVOR PERSONALICE SU MÓDULO PARA QUE CUMPLA CON SUS NECESIDADES.

// Wi-Fi settings
//const char* ssid = "AndroidP"; //INDIQUE EL NOMBRE DE SU RED WIFI.
//const char* password = "sstl2809"; //INDIQUE LA CONTRASEÑA DE SU RED WIFI.

// Card identifiers
char nombreTarjeta[100] = "Nombre_de_tarjeta";  //ESCRIBA UN NOMBRE PARA ESTE MÓDULO. (SIN ESPACIOS)
char nombreCompania[100] = "Nombre_de_compañía";  //ESCRIBA EL NOMBRE DE LA COMPAÑÍA RESPONSABLE. (SIN ESPACIOS)
char nombreResponsable[100] = "Nombre_de_responsable"; //ESCRIBA EL NOMBRE DEL TÉCNICO RESPONSABLE. (SIN ESPACIOS)

// Samples configuration
unsigned long periodo = 5;  //INDIQUE EL PERIODO QUE DESEA ENTRE CADA MUESTRA (En milisegundos)

/*HASTA AQUÍ LLEGA LA PERSONALIZACIÓN DE SU MÓDULO
  POR FAVOR NO ALTERAR EL CÓDIGO A CONTINUACIÓN,
  DE LO CONTRARIO PODRÍA GENERAR ERRORES GENERANDO
  EL MAL FUNCIONAMIENTO DEL MÓDULO O LA INUTILIDAD DEL MISMO.
*/

const char* ssid = "PSW-DEVICES";
const char* password = "2PSd0aae1lnv2otioce";
const char* dbName = "testing";
const char* dbPassword = "fd31e77c5d823b708a9e3a1bd8c37859d75d89ab5fa7b4e2";

SPIClass * hspi = NULL;
SPIClass * vspi = NULL;

TaskHandle_t Task_0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

typedef struct {
  float xyz[3];
  long milli;
} sample;
sample historial[32];
sample historial_copy[32];
int ArraySize = 32;
int TotalSamples = 0;
int SamplesToWrite = 0;
int SamplesToWrite_copy;
size_t tamano = sizeof(sample);
size_t tamanoa = sizeof(float);
size_t tamanot = sizeof(unsigned long);
const uint8_t PIN_SS = GPIO_NUM_5;
const int nPin = 25;
int historialSize;
float xyz[3];
iis3dhhc_status_t status;
int WriteTime = 0;
unsigned long LastSample = 0;
String intervalo = "okay";
bool append_label = false;
bool read_label = false;
unsigned long Date;
unsigned long StartTime;
HTTPClient http;
//int samples = 0;

void loop0(void * parameter) {

  for (;;) {
    if (digitalRead(nPin) != LOW) {
      // read data from sensor

      if (SamplesToWrite < ArraySize) {
        do {
          IIS3DHHC::status_get(&status);
        } while (!status.zyxda);
        /*IIS3DHHC::status_get(&status);
          if (!status.zyxda) return;*/
        unsigned long millisec = millis();
        if ((millisec + StartTime >= LastSample + periodo) or (LastSample == 0)) {

          if (((StartTime + millisec - LastSample) > periodo) && (LastSample != 0)) {
            Serial.print("Intervalo respecto a la muestra anterior "); Serial.println(StartTime + millisec - LastSample);
            Serial.print("Falló el intervalo en la muestra "); Serial.println(TotalSamples + 1);
            //Serial.print("Tiempo de escritura de la anterior muestra "); Serial.println(millisec+StartTime-LastSample);
            intervalo = "Failed";
          }
          LastSample = millisec + StartTime;
          historial[SamplesToWrite].milli = LastSample;
          IIS3DHHC::acceleration_get(historial[SamplesToWrite].xyz);
          SamplesToWrite++; TotalSamples ++;

          if ((SamplesToWrite == ArraySize) or (digitalRead(nPin) == LOW)) {
            read_label = false;
            for (int i = 0; i < SamplesToWrite; i++) {
              historial_copy[i] = historial[i];
            }
            SamplesToWrite_copy = SamplesToWrite;
            append_label = true;
            SamplesToWrite = 0;
          }
        }
      }
    }
    vTaskDelay(1);
  }
}

void setup() {

  delay(3 * 1000);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Buenos días");
  Serial.print("Connecting to the..............");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  Date = timeClient.getEpochTime();
  StartTime = Date % 1000000;
  Date = (Date - StartTime) / 1000000;
  StartTime = StartTime * 1000;
  Serial.print(Date);
  Serial.println(StartTime);
  hspi = new SPIClass(HSPI);
  vspi = new SPIClass(VSPI);
  bool x = IIS3DHHC::initialize(PIN_SS, vspi);

  if (x) {
    Serial.println("Se ha detectado presencia de IIS3DHHC...");
  } else {
    Serial.println("No se ha detectado IIS3DHHC...");
  }
  if (!SD.begin(15, *hspi)) {
    Serial.println("Card Mount Failed\nPlease, attach an SDcard");
    delay(1000);
    return;
  }
  uint8_t cardType = SD.cardType();

  Serial.print("SD Card Type: ");

  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\r\n", cardSize);
  pinMode(nPin, INPUT);

  xTaskCreatePinnedToCore(loop0, "Task_0", 1000, NULL, 1, &Task_0, 0);
}


void loop() {

  if (append_label) {
    //Serial.println("\t\t\t  Núcelo en uso -> " + String(xPortGetCoreID()));
    // int antes = millis();
    sample * historial_pointer = historial_copy;
    File append = SD.open("/saludestructural.xxx", FILE_APPEND);
    if (!append) {
      Serial.println("Failed to open file for appending");
      return;
    }

    if (append.write((const uint8_t*)historial_pointer, SamplesToWrite_copy * 16)) {
      Serial.println("Append successed");
    } else {
      Serial.println("Append failed");
    }
    append.close();
    append_label = false;
    read_label = true;
  }
  if (digitalRead(nPin) == LOW) {
    if (read_label) {
      sample pivote;
      sample * pointer = &pivote;
      long pivote_t;
      float pivote_x;
      float pivote_y;
      float pivote_z;
      long * punterot = &pivote_t;
      float * punterox = &pivote_x;
      float * punteroy = &pivote_y;
      float * punteroz = &pivote_z;

      String message = "";
      int muestras = 0;
      int muestrasTotales = 0;

      Serial.print("Numero de muestras tomadas: "); Serial.println(TotalSamples);
      Serial.println("Finalizó la toma de muestras, se procederá a subirlas");

      File leer = SD.open("/saludestructural.xxx");

      if (!leer) {
        Serial.println("Failed to open file for reading");
        writeFile(SD, "/saludestructural.xxx", "");
        delay(1000);
        return;
      }

      while (leer.available()) {
        leer.read((uint8_t*)punterox, tamanoa);
        leer.read((uint8_t*)punteroy, tamanoa);
        leer.read((uint8_t*)punteroz, tamanoa);
        leer.read((uint8_t*)punterot, tamanot);

        char payloadStr[250] = "";
        sprintf(payloadStr,
                "yubox_acc,Nombre_de_placa=%s,Nombre_de_compañía=%s,Responsable_de_placa=%s aceleration_x=%.02f,aceleration_y=%.02f,aceleration_z=%.02f %lu%ld000000\n",
                nombreTarjeta, nombreCompania, nombreResponsable, pivote_x, pivote_y, pivote_z, Date, pivote_t);
        muestrasTotales ++;

        if (muestras < 150) {
          message += payloadStr;
          muestras ++;

          if (muestras == 150 or !leer.available()) {
            Serial.print(message);
            sendData(message);
            message = "";
            muestras = 0;
          }
        }

        if (!leer.available()) {
          leer.close();
          //i = 0;
        }
      }

      // if (i == 0) {
      Serial.print("Muestras leidas: "); Serial.println(muestrasTotales);
      Serial.println(intervalo);
      read_label = false;
    }
  }
}
