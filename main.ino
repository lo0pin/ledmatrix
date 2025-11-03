#include <Arduino.h>
#include <SPI.h>
#include <MD_MAX72xx.h>
#include "matrixleiste.h"
#include <math.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// --- RTC (RTClib) ---
#include "RTClib.h"

int mode = 0;
/*
 * 0 = laufschrift
 * 1 = einblenden, der reihe nach
 */

/*
  Verdrahtung (Arduino Uno):
    SDA -> A4 (oder SDA-Pin)
    SCL -> A5 (oder SCL-Pin)
    VCC -> 5V (RTC) / 3.3V (BME280) *oder* 5V falls das BME280-Board Pegelwandler/Regler hat
    GND -> GND
*/


// >>> Passe diesen Hardware-Typ an dein Modul an (häufig FC16_HW). <<<
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW   // z.B. FC16_HW, PAROLA_HW, GENERIC_HW
#define MAX_DEVICES   4                     // Anzahl kaskadierter 8x8-Module
#define CS_PIN        10                    // Chip-Select (LOAD)

// Globale Display-Instanz (Hardware-SPI)
MD_MAX72XX mx(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


// I2C-Instanz RTC:
RTC_DS3231 rtc;     // Für DS3231 (genauer & mit Temperatur)


// I2C-Instanz Sensor
Adafruit_BME280 bme;
// Hinweis: BME280 hat meist I2C-Adresse 0x76 ODER 0x77

const float ALTITUDE = 353.0;               // Höhe über NN in Metern
//const float SEA_LEVEL_REFERENCE = 1013.25;  // hPa Referenz (optional)
float pressureOffset = 0.0;                 // manuelle Kalibrierung in hPa
//const int hoehe_m = 353;                  //Meereshöhe von Graz
float T =0 , H = 0, P = 0;


// --- Konfiguration ---
float seaLevel_hPa = 1013.25; // lokaler Luftdruck auf Meereshöhe (zur Altitude-Berechnung!)
// Tipp: Einmal auf deinen Ort kalibrieren (Wetterdienst), dann passt die Höhenanzeige super.

// Sichtbreite in Spalten
const uint8_t VIEW_COLS = MAX_DEVICES * 8;

// Animations-Parameter (non-blocking)
const uint16_t STEP_MS = 60;    // Zeit zwischen Frames
unsigned long lastTick = 0;
const uint16_t STEP_MS_MEASSURE = 500;
unsigned long lastTick_meassure = 0;

String message = "booting...";
const int maxleng = 512;
byte buf[maxleng];
const int spacerint = 2;
int columns = 0;
int iterations = 0;

// -------- Barograph --------
float baro_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
/*
  float baro_messungen[24] = {
  991,  992,  993,  994,  995,  996,
  997,  998,  999,  1000, 1001, 1002,
  1003, 1004, 1005, 1006, 1007, 1008,
  1009, 1010, 1011, 1012, 1013, 990
  };
*/

const int baro_len = 24;
bool firstrun = 1;
int currenthour = -1;

// -------- Setup --------
void setup()
{
  Serial.begin(9600);
  while (!Serial) {}
  startUp(rtc, bme, mx);
  columns = createMessage(message, buf, maxleng, spacerint);
  Serial.println(F("Setup abgeschlossen.\n"));
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}


// -------- Loop (non-blocking) --------
void loop()
{
  //wenn eine Message zu Ende geschrieben ist
  if (iterations > columns + (VIEW_COLS - 1)) {
    iterations = 0; //Reset des Displayvorgangs

    // ---------- RTC lesen ----------
    DateTime now = rtc.now();
    //Serial.print(F("RTC: "));
    //printDateTime(now);
    CheckZeitumstellung(rtc, now);



    //Luftdruck über die letzten 24h ausgeben
    if (!firstrun) {
      printbaro(mx, baro_messungen, baro_len, now);
      delay(1000);
    }
    firstrun = 0;


    // ---------- BME280 lesen ----------
    readBME(T, H, P, bme);
    
  
    //float alt = bme.readAltitude(seaLevel_hPa); // m, abhängig von seaLevel_hPa
    //print_bme_stats( T, P, H, alt);

    // ---------- Umwandlung des stationären Luftdrucks in Luftdruck auf Seehöhe ----------
    //Serial.print("Seelevel hPa:\t");
    //float sea_lvl = sea_level_hPa(P, ALTITUDE);
    //Serial.println(sea_lvl);

    // ---------- Aktuellen Luftdruck 1x pro Stunde ins Array schreiben ----------
    int hour_now = now.hour();
    if (currenthour != hour_now){
      currenthour = hour_now;
      writebaroentry(baro_messungen, currenthour, P);
      for (int i = 0; i< baro_len; ++i){
        Serial.println(baro_messungen[i]);
      }
    }

    newmessage(message, now, T, P, H, -1, -1); //2 Variablen sind Höhenmessung und Umgewandelten Druckwert für Meereshöhe
    columns = createMessage(message, buf, maxleng, spacerint);
  }

  const unsigned long now_timer = millis();
  if (now_timer - lastTick < STEP_MS) return;           // noch nicht Zeit für den nächsten Frame
  lastTick = now_timer;
  writeMessage(buf, columns, mx, iterations++);






}
