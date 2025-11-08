# 🌡️ **BME280 Weather Display with LED Matrix & EEPROM Storage**

An Arduino project that displays and stores weather data (temperature, humidity, and air pressure) on an 8×32 LED matrix.
The readings come from a **BME280 sensor**, while the **DS3231 RTC module** provides accurate timekeeping.
Hourly pressure values are stored persistently in the **EEPROM**, allowing the system to restore and visualize pressure trends after restart.

---

## 🔧 **Hardware Components**

| Component                            | Function                          | Connection                  |
| ------------------------------------ | --------------------------------- | --------------------------- |
| **Arduino Uno**                      | Main controller                   | —                           |
| **BME280 Sensor**                    | Temperature, pressure, humidity   | I²C (SDA=A4, SCL=A5)        |
| **DS3231 RTC Module**                | Real-time clock with DST handling | I²C (SDA=A4, SCL=A5)        |
| **LED Matrix (4× 8×8 FC16 modules)** | Text and graph display            | SPI (DIN=11, CLK=13, CS=10) |
| *(optional)* Push buttons            | User input or mode switching      | configurable                |

---

## 💡 **Features**

* **Automatic sensor initialization**
  Detects BME280 and RTC at startup, restores or sets time after power loss.

* **Live data display on LED matrix**

  * Current time and date
  * Temperature, humidity, and air pressure
  * Optional: 24-hour barometric trend graph

* **Hourly EEPROM data storage**

  * Saves one pressure value per hour (`hPa × 10` as `int16_t`)
  * Automatically loads stored data at startup
  * Includes **checksum** and **magic number** validation
  * Uses `EEPROM.put()` with byte-level wear protection

* **Automatic daylight saving time detection (EU)**

  * Detects the last Sunday in March/October
  * Shifts clock forward/backward automatically
  * Prevents repeated adjustments on the same day

* **Robust operation**

  * Handles uninitialized EEPROM gracefully
  * Recovers from power loss without losing long-term data

---

## 🧠 **Project Structure**

```
├── main.ino              # Main sketch (loop and logic)
├── matrixleiste.h        # Declarations, constants, public functions
└── matrixleiste.cpp      # Implementations: display, EEPROM, BME, RTC, DST
```

**Key modules:**

* `saveBaroToEEPROM()` / `loadBaroFromEEPROM()` – handle persistent pressure data
* `printbaro()` – visualizes hourly pressure trend on the LED matrix
* `readBME()` – averages multiple readings for stability
* `CheckZeitumstellung()` – automatic daylight-saving adjustment

---

## ⚙️ **Dependencies**

Install via the **Arduino Library Manager**:

* [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
* [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
* [RTClib (by Adafruit)](https://github.com/adafruit/RTClib)
* [MD_MAX72xx](https://github.com/MajicDesigns/MD_MAX72xx)
* [EEPROM (built-in)](https://www.arduino.cc/en/Reference/EEPROM)

---

## 🚀 **How to Use**

1. Connect the hardware according to the table above.
2. Upload the sketch to your Arduino Uno.
3. On the first run:

   * The clock is set to the compile time.
   * The EEPROM is empty, so all pressure values initialize as `-1`.
4. After one hour, the first valid pressure value is stored in EEPROM.
5. The LED matrix shows live sensor data and gradually builds the barometric trend.

---

## 💾 **EEPROM Layout**

| Field         | Type          | Description                         |
| ------------- | ------------- | ----------------------------------- |
| `magic`       | `uint16_t`    | Identifier (`0xBADA`)               |
| `version`     | `uint8_t`     | Data structure version              |
| `hPa_x10[24]` | `int16_t[24]` | Hourly pressure values (`hPa × 10`) |
| `checksum`    | `uint8_t`     | Simple byte-based checksum          |

**Total size:** ~52 bytes (fits easily into the Uno’s 1 KB EEPROM).

---

## 🧩 **Example Serial Output**

```
Init I2C...
Init RTC...
Init BME280...
BME280  T: 23.45 °C   P: 1008.21 hPa   H: 47.8 %RH   Alt: 541.2 m
Init MD_MAX72XX...
RTC_DS3231_H detected
creating initial message...
```

---

## 🛠️ **Future Improvements**

* Temperature or humidity trend graphs
* Additional EEPROM storage for historical data
* ESP8266/ESP32 web dashboard
* Configurable display brightness and night mode
* Optional external calibration or reference pressure sync

---

## 📜 **License**

MIT License

Copyright (c) 2025 Julian Kampitsch

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
