#include "matrixleiste.h"
#include <MD_MAX72xx.h>
#include <ctype.h>
#include <Arduino.h>
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


bool sommerzeit = false;


// ================================
//  Font-Daten (Columns je Zeichen)
// ================================
// Hinweis: Für AVR kannst du das später in PROGMEM legen, hier erst mal simpel im RAM.
static const byte FONT_AZ[26][8] = {
  // A
  { B00111110, B01001000, B01001000, B00111110, B00000000, B00000000, B00000000, B00000000 },
  // B
  { B01111110, B01010010, B01010010, B00101100, B00000000, B00000000, B00000000, B00000000 },
  // C
  { B00111100, B01000010, B01000010, B00100100, B00000000, B00000000, B00000000, B00000000 },
  // D
  { B01111110, B01000010, B01000010, B00111100, B00000000, B00000000, B00000000, B00000000 },
  // E
  { B01111110, B01010010, B01010010, B01010010, B00000000, B00000000, B00000000, B00000000 },
  // F
  { B01111110, B01010000, B01010000, B01010000, B00000000, B00000000, B00000000, B00000000 },
  // G
  { B00111100, B01000010, B01001010, B00101100, B00000000, B00000000, B00000000, B00000000 },
  // H
  { B01111110, B00010000, B00010000, B01111110, B00000000, B00000000, B00000000, B00000000 },
  // I
  { B01111110, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000 },
  // J
  { B01000100, B01000010, B01000010, B01111100, B00000000, B00000000, B00000000, B00000000 },
  // K
  { B01111110, B00001000, B00010100, B01100010, B00000000, B00000000, B00000000, B00000000 },
  // L
  { B01111110, B00000010, B00000010, B00000010, B00000000, B00000000, B00000000, B00000000 },
  // M
  { B01111110, B00100000, B00010000, B00100000, B01111110, B00000000, B00000000, B00000000 },
  // N
  { B01111110, B00010000, B00001000, B01111110, B00000000, B00000000, B00000000, B00000000 },
  // O
  { B00111100, B01000010, B01000010, B00111100, B00000000, B00000000, B00000000, B00000000 },
  // P
  { B01111110, B01001000, B01001000, B00110000, B00000000, B00000000, B00000000, B00000000 },
  // Q
  { B00111100, B01000010, B01001010, B00110100, B00000000, B00000000, B00000000, B00000000 },
  // R
  { B01111110, B01001000, B01001000, B00110110, B00000000, B00000000, B00000000, B00000000 },
  // S
  { B00110100, B01010010, B01010010, B00101100, B00000000, B00000000, B00000000, B00000000 },
  // T
  { B01000000, B01000000, B01111110, B01000000, B01000000, B00000000, B00000000, B00000000 },
  // U
  { B01111100, B00000010, B00000010, B01111100, B00000000, B00000000, B00000000, B00000000 },
  // V
  { B01111000, B00000100, B00000010, B00000100, B01111000, B00000000, B00000000, B00000000 },
  // W
  { B01111110, B00000100, B00001000, B00000100, B01111110, B00000000, B00000000, B00000000 },
  // X
  { B01100010, B00010100, B00001000, B00010100, B01100010, B00000000, B00000000, B00000000 },
  // Y
  { B01100100, B00010010, B00010010, B01111100, B00000000, B00000000, B00000000, B00000000 },
  // Z
  { B01000110, B01001010, B01010010, B01100010, B00000000, B00000000, B00000000, B00000000 }
};

static const byte FONT_09[10][8] = {
  // 0
  { B00111100, B01010010, B01001010, B00111100, B00000000, B00000000, B00000000, B00000000 },
  // 1
  { B01000010, B01111110, B00000010, B00000000, B00000000, B00000000, B00000000, B00000000 },
  // 2
  { B00100110, B01001010, B01001010, B00110010, B00000000, B00000000, B00000000, B00000000 },
  // 3
  { B01000010, B01001010, B01001010, B00110100, B00000000, B00000000, B00000000, B00000000 },
  // 4
  { B00001000, B00011000, B00101000, B01111110, B00001000, B00000000, B00000000, B00000000 },
  // 5
  { B01110010, B01010010, B01010010, B01001100, B00000000, B00000000, B00000000, B00000000 },
  // 6
  { B00111100, B01010010, B01010010, B01001100, B00000000, B00000000, B00000000, B00000000 },
  // 7
  { B01000000, B01000110, B01001000, B01110000, B00000000, B00000000, B00000000, B00000000 },
  // 8
  { B00101100, B01010010, B01010010, B00101100, B00000000, B00000000, B00000000, B00000000 },
  // 9
  { B00110100, B01010010, B01010010, B00111100, B00000000, B00000000, B00000000, B00000000 }
};

// ----------------- interne Helfer -----------------
static int letterIndex(char c) {
  char ch = toupper((unsigned char)c);
  return (ch >= 'A' && ch <= 'Z') ? (ch - 'A') : -1;
}

// ----------------- öffentliche API -----------------
int createMessage(const String& message, byte result[], int resultMax, int spacer) {
  if (message.length() == 0 || result == nullptr || resultMax <= 0) return 0;

  int index = 0;
  const int maxlen = message.length();

  for (int pos = 0; pos < maxlen; ++pos) {
    char ch = toupper((unsigned char)message[pos]);

    // Leerzeichen
    if (ch == ' ') {
      const int spacewidth = 6;
      for (int i = 0; i < spacewidth && index < resultMax; ++i) result[index++] = 0x00;
      continue;
    }

    // Buchstaben A-Z
    int li = letterIndex(ch);
    if (li >= 0) {
      for (int i = 0; i < 8 && index < resultMax; ++i) {
        if (FONT_AZ[li][i] == 0x00) {
          for (int s = 0; s < spacer && index < resultMax; ++s) result[index++] = 0x00;
          break;
        }
        result[index++] = FONT_AZ[li][i];
      }
      continue;
    }

    // Ziffern 0-9
    if (isdigit((unsigned char)ch)) {
      int di = ch - '0';
      for (int i = 0; i < 8 && index < resultMax; ++i) {
        if (FONT_09[di][i] == 0x00) {
          for (int s = 0; s < spacer && index < resultMax; ++s) result[index++] = 0x00;
          break;
        }
        result[index++] = FONT_09[di][i];
      }
      continue;
    }

    // Einfache Satzzeichen & Umlaute (Beispiele)
    auto pushCol = [&](uint8_t c) {
      if (index < resultMax) result[index++] = c;
    };

    if (index + 5 < resultMax) {
      if      (ch == ':') {
        pushCol(B00010100);
      }
      else if (ch == '.') {
        pushCol(B00000010);
      }
      else if (ch == ',') {
        pushCol(B00000001);
        pushCol(B00000010);
      }
      else if (ch == '!') {
        pushCol(B01111010);
      }
      else if (ch == '-') {
        pushCol(B00001000);
        pushCol(B00001000);
        pushCol(B00001000);
      }
      else if (ch == '%') {
        pushCol(B00100100);
        pushCol(B00001000);
        pushCol(B00010000);
        pushCol(B00100100);
      }
      else if (ch == '°') {
        pushCol(B00101000);
        pushCol(B01010000);
        pushCol(B00100000);
      }
      else if (ch == '*') {
        pushCol(B00101000);
        pushCol(B00010000);
        pushCol(B00101000);
      }
      else if (ch == 'Ä') {
        pushCol(B10111110);
        pushCol(B01001000);
        pushCol(B01001000);
        pushCol(B10111110);
      }
      else if (ch == 'Ö') {
        pushCol(B10111100);
        pushCol(B01000010);
        pushCol(B01000010);
        pushCol(B10111100);
      }
      else if (ch == 'Ü') {
        pushCol(B10111100);
        pushCol(B00000010);
        pushCol(B00000010);
        pushCol(B10111100);
      }
      else                {
        pushCol(B00000010);
        pushCol(B00000010);
        pushCol(B00000010);
        pushCol(B00000010);
      }

      for (int s = 0; s < spacer && index < resultMax; ++s) result[index++] = 0x00;
    }
  }

  return index;
}

void writeMessage(byte* messageToPrint, int lenOfMessage, MD_MAX72XX& objToWrite, int progressOfMovement) {
  if (messageToPrint == nullptr || lenOfMessage <= 0) return;

  const int view = objToWrite.getColumnCount();

  for (int i = 0; i < view; ++i) {
    // Entspricht deiner bisherigen Logik, aber ohne „32“-Magie:
    // früher: src = progress + (view - 32 - i)  (bei view=32 → progress - i)
    int src = progressOfMovement - i;

    uint8_t col = 0x00;
    if (src >= 0 && src < lenOfMessage) col = messageToPrint[src];

    // FC16-Module sind oft horizontal gespiegelt → (view-1-i)
    objToWrite.setColumn(view - 1 - i, col);
  }
  // objToWrite.update(); // nur nötig, wenn UPDATE=OFF
}






void newmessage(String& mes, DateTime& dt, float T , float P, float H, float alt, float sea_lev) {
  mes = String(dt.hour());
  mes += ":";
  if ((int)dt.minute() < 10) {
    mes += "0";
  }
  mes += String(dt.minute());
  mes += " ";
  mes += String(dt.day());
  mes += ".";
  mes += String(dt.month());
  mes += ".";
  mes += String(dt.year());
  mes += " ";
  mes += "T:";
  mes += String(T, 1);
  mes += "C P:";
  mes += String(P, 1);
  mes += " H:";
  mes += String(H, 1);
  mes += "% ";
  mes += "Baro:";
  



  /*
    // „mm:ss“ aus millis()
    const unsigned long s = millis() / 1000;
    const unsigned long mm = s / 60;
    const unsigned long ss = s % 60;

    mes  = String((int)mm);
    mes += ":";
    if (ss < 10) mes += "0";
    mes += String((int)ss);
  */



}



// Nützliche Hilfsfunktion: formatiert DateTime schön
void printDateTime(const DateTime& now) {
  static const char* wd[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
  char buf[40];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d  %s",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second(),
           wd[now.dayOfTheWeek()]);
  Serial.println(buf);
}

// Einmalig (zum Setzen der Uhr): hochladen, kurz laufen lassen, dann wieder AUSkommentieren
void setTimeOnce(RTC_DS3231& obj) {
  // Beispiel: 2025-09-18 12:00:00
  // obj.adjust(DateTime(2025, 9, 18, 12, 0, 0));
  // Oder auf Kompilierzeit setzen:
  obj.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

// Umrechnung auf Meereshöhe
float pressureToSeaLevel(float measuredPressure_hPa, float altitude_m) {
  return measuredPressure_hPa / pow(1.0 - (altitude_m / 44330.0), 5.255);
}

// Manuelle Kalibrierung gegen Referenzstation
void calibratePressure(float reference_hPa, float measured_hPa) {
  float pressureOffset = reference_hPa - measured_hPa;
  Serial.print("Kalibrierung aktiv. Offset = ");
  Serial.print(pressureOffset);
  Serial.println(" hPa");
}


void print_bme_stats(float T, float P, float H, float alt) {
  Serial.print(F("BME280  T: "));
  Serial.print(T, 2);
  Serial.print(F(" °C   P: "));
  Serial.print(P, 2);
  Serial.print(F(" hPa   H: "));
  Serial.print(H, 1);
  Serial.print(F(" %RH   Alt: "));
  Serial.print(alt, 1);
  Serial.println(F(" m"));
}


// ---------- „Meereshöhe“ kalibrieren ----------
// Wenn du deine echte Hoehe (z. B. 580 m) kennst, kannst du damit den
// seaLevel_hPa-Wert feinjustieren:
float sea_level_hPa(float P, float height) {
  float seaLevel_hPa = P / pow(1.0 - (height / 44330.0), 5.255);
  return seaLevel_hPa;
}


void startUp(RTC_DS3231& rtc_var, Adafruit_BME280& bme_var, MD_MAX72XX& mx_var) {
  Serial.println(F("Init I2C..."));
  Wire.begin(); // Beim Uno nutzt das intern A4/A5

  // ---------- RTC starten ----------
  Serial.println(F("Init RTC..."));
  if (!rtc_var.begin()) {
    Serial.println(F("RTC nicht gefunden! Prüfe SDA/SCL, VCC, GND."));
    while (1) delay(10);
  }

  // Falls die RTC „stehen blieb“ (z. B. nach Batteriewechsel), Zeit setzen:
  if (rtc_var.lostPower()) {
    Serial.println(F("RTC hat Strom verloren – setze auf Kompilierzeit."));
    setTimeOnce(rtc_var); // Einmalig Uhr setzen (siehe Funktion oben)
  }

  // ---------- BME280 starten ----------
  Serial.println(F("Init BME280..."));
  bool ok = bme_var.begin(0x76);           // erst 0x76 versuchen
  if (!ok) ok = bme_var.begin(0x77);       // dann 0x77
  if (!ok) {
    Serial.println(F("BME280 nicht gefunden (0x76/0x77)."));
    Serial.println(F("Pruefe Adresse/Jumper/Verkabelung/Spannung."));
    while (1) delay(10);
  }

  // ---------- Einfache „Konsistenz“-Auswertung ----------
  // Idee: Wenn DS3231-Temp und BME280-Temp sehr weit auseinander liegen,
  // Hinweis ausgeben (z. B. wenn ein Sensor vom Luftstrom/Heizung beeinflusst ist).
#ifdef RTC_DS3231_H
  Serial.println(F("RTC_DS3231_H erkannt"));
  if (!isnan(T) && !isnan(rtc_var.getTemperature())) {
    float dT = fabsf(T - rtc_var.getTemperature());
    if (dT > 3.0f) {
      Serial.println(F("Hinweis: RTC-Temp und BME280-Temp unterscheiden sich >3°C."));
      Serial.println(F("-> BME280 nicht neben Spannungsregler/MCU platzieren;"));
      Serial.println(F("-> ggf. Luftzug vermeiden oder Gehaeuse optimieren."));
    }
  }
#endif

  Serial.println(F("Init MD_MAX72XX..."));
  mx_var.begin();                                     // Start der Lib
  Serial.println(F("MD_MAX72XX INTENSITY 1"));
  mx_var.control(MD_MAX72XX::INTENSITY, 1);           // Helligkeit 0..15
  mx_var.clear();                                     // alles aus
  Serial.print(F("creating initial message"));
  for (int i = 0; i < 3; ++i) {
    delay(10);
    Serial.println(F("."));
  }
}


void writebaroentry(float* baros, const int currenthour, float pressure){
  baros[currenthour] = pressure;
}

void printbaro(MD_MAX72XX& mx_var, const float* baros, const int baro_len, const DateTime& rtc_var) {
  mx_var.clear();
  
  // aktuelle Stunde (z. B. für Markierung)
  int current_hour = (int)rtc_var.hour();
  
  // Array für Matrix-Höhen (0–8)
  int baroToMatrix[baro_len];
  
  // Min/Max bestimmen
  float baroMax = -9999, baroMin = 9999;

  for (int i = 0; i < baro_len; ++i){
    float actual_value = baros[i];
    if (actual_value == -1) continue;
    baroMax = actual_value>baroMax? actual_value : baroMax;
    baroMin = actual_value<baroMin? actual_value : baroMin;
  }

  // Bereich in 8 Stufen aufteilen
  float baro_diff = baroMax-baroMin;
  float baro_step = baro_diff>0 ? baro_diff/8.0f : 1.0f; // Division durch 0 vermeiden
  if (baro_step < 0.5) baro_step = 0.5;
  /*
   * wenn weiterhin der "fehler" besteht, dass das Baro ganz wild auf und abgeht, aber es 0er
   * steps sind, dann kann man versuchen, hier baro_diff>0.5 zuprobieren. Dann gib es keine 
   * Steps die <0 sind und die Linie wird etwas gerader.
   * else ginge auch
   * if (baro_step < 0.5) baro_step = 0.5;
   */

  /*
  Serial.print("baroMax:\t");
  Serial.println(baroMax);
  Serial.print("baroMin:\t");
  Serial.println(baroMin);
  Serial.print("baro_diff:\t");
  Serial.println(baro_diff);
  Serial.print("baro_step:\t");
  Serial.println(baro_step);  
   */
  // Float-Werte in Integer (0–8) umrechnen
  for (int i = 0; i < baro_len; ++i){
    if (baros[i] == -1){
      baroToMatrix[i] = -1;
      continue;
    }
    baroToMatrix[i] = (int)((baros[i]-baroMin)/baro_step);

    // Werte ggf. begrenzen (Sicherheitsmaßnahme)
    if (baroToMatrix[i] < 0) baroToMatrix[i] = 0;
    if (baroToMatrix[i] >= 8) baroToMatrix[i] = 7;
  }

  for (int i = 0; i < 24; i++ ) {
    current_hour++;
    if (current_hour >= 24) current_hour = 0;

    //y, x von links unten
    if (baroToMatrix[current_hour] == -1) continue;
    mx_var.setPoint( baroToMatrix[current_hour],i, true);
    delay(200);
  }
  
  for (int i = 24+2, j=0; i<24+8; ++i, ++j){
    mx_var.setColumn(i,FONT_09[(int)baro_step][j]);
  }
}

void readBME(float& T, float& H, float& P, Adafruit_BME280& bme_var){
  int number_of_measurements = 5;
  T=0; 
  H=0;
  P=0;
  
  for (int i=0; i<number_of_measurements; ++i) {
    T += bme_var.readTemperature();       // °C
    H += bme_var.readHumidity();          // %RH
    P += bme_var.readPressure() / 100.0F; // in hPa
    delay(100);
  }

  T /= number_of_measurements;
  H /= number_of_measurements;
  P /= number_of_measurements;
}

static uint16_t lastAdjustYday = 65535; // Marker gegen Mehrfach-Adjust (unmöglicher Startwert)

void CheckZeitumstellung(RTC_DS3231& rtc_ref, const DateTime& dt) {
  // "Tag-im-Jahr" – reicht als Einmal-Sperre pro Kalendertag
  uint16_t yday = DateTime(dt.year(), dt.month(), dt.day()).unixtime() / 86400;

  // --- Umstellung auf Winterzeit (CEST -> CET): letzter Sonntag im Oktober, 03:00 -> 02:00 ---
  if (sommerzeit &&
      isLastSundayOfOctober(dt) &&
      dt.hour() == 3 && dt.minute() == 0 && dt.second() == 0 &&
      yday != lastAdjustYday) {

    rtc_ref.adjust(DateTime(dt.year(), dt.month(), dt.day(),
                            2, dt.minute(), dt.second())); // eine Stunde zurück
    sommerzeit = false;
    lastAdjustYday = yday;
    return; // defensiv: an diesem Tick nichts weiter
  }

  

  // --- Umstellung auf Sommerzeit (CET -> CEST): letzter Sonntag im März, 02:00 -> 03:00 ---
  if (!sommerzeit &&
      isLastSundayOfMarch(dt) &&
      dt.hour() == 2 && dt.minute() == 0 && dt.second() == 0 &&
      yday != lastAdjustYday) {

    rtc_ref.adjust(DateTime(dt.year(), dt.month(), dt.day(),
                            3, dt.minute(), dt.second())); // eine Stunde vor
    sommerzeit = true;
    lastAdjustYday = yday;
    return;
  }

  // Kein expliziter Reset nötig: am Folgetag ist yday != lastAdjustYday automatisch erfüllt.
}

bool isLastSundayOfOctober(const DateTime& dt) {
  if (dt.month() != 10) return false;
  // letzter Sonntag: wenn zum Datum innerhalb der nächsten 7 Tage der Monatswechsel käme
  // dayOfTheWeek(): 0=So,1=Mo,...6=Sa
  return dt.dayOfTheWeek() == 0 && (dt.day() + 7) > 31;
}

bool isLastSundayOfMarch(const DateTime& dt) {
  if (dt.month() != 3) return false;
  // letzter Sonntag: wenn zum Datum innerhalb der nächsten 7 Tage der Monatswechsel käme
  // dayOfTheWeek(): 0=So,1=Mo,...6=Sa
  return dt.dayOfTheWeek() == 0 && (dt.day() + 7) > 31;
}
