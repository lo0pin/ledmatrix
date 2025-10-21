#pragma once
#include <Arduino.h>



// Vorwärtsdeklaration reicht für die Signaturen
class MD_MAX72XX;
class DateTime; 
class RTC_DS3231; 
class Adafruit_BME280;


// Baut aus Text einen Spaltenpuffer (Columns) für das Matrix-Display.
// result: Zielpuffer (byte-Array)
// resultMax: maximale Anzahl Spalten
// spacer: Anzahl Nullspalten zwischen Zeichen
int  createMessage(const String& message, byte result[], int resultMax, int spacer);

// Schreibt ein Fenster (sichtbare Spalten) des Puffers auf die Matrix (non-blocking).
// progressOfMovement: wie weit bereits gescrollt wurde (in Spalten)
void writeMessage(byte* messageToPrint, int lenOfMessage, MD_MAX72XX& objToWrite, int progressOfMovement);

// Erzeugt eine neue Beispiel-Nachricht (mm:ss) in 'mes'.
void newmessage(String& mes, DateTime& dt, float T , float P, float H, float alt, float sea_lev);

// Nützliche Hilfsfunktion: formatiert DateTime schön
void printDateTime(const DateTime& now);

// Einmalig (zum Setzen der Uhr): hochladen, kurz laufen lassen, dann wieder AUSkommentieren
void setTimeOnce(RTC_DS3231& obj);

// Umrechnung auf Meereshöhe
float pressureToSeaLevel(float measuredPressure_hPa, float altitude_m);

// Manuelle Kalibrierung gegen Referenzstation
void calibratePressure(float reference_hPa, float measured_hPa);

//BME Messung Drucken
void print_bme_stats(float T, float P, float H, float alt);


// ---------- „Meereshöhe“ kalibrieren ----------
// Wenn du deine echte Hoehe (z. B. 580 m) kennst, kannst du damit den
// seaLevel_hPa-Wert feinjustieren:
float sea_level_hPa(float P, float height);

void startUp(RTC_DS3231& rtc_var, Adafruit_BME280& bme_var, MD_MAX72XX& mx_var);

void printbaro(MD_MAX72XX& mx_var, const float* baros, const int baro_len, const DateTime& rtc_var);

void writebaroentry(float* baros, const int currenthour, float pressure);

void readBME(float& T, float& H, float& P, Adafruit_BME280& bme_var);
