# ESP32-C3 Climate Reader
This is a small project, where I build a climate monitoring device based on an ESP32-C3 Mini and a BME680. The system measures environmental conditions such as temperature, humidity and air pressure, displaying the collected data in real time on an OLED screen.

### Current Status
- [x] Firmware implementation completed
- [x] Sensor driver implemented
- [x] OLED interface implemented
- [x] Environmental data acquisition working
- [x] 3D printed enclosure
- [ ] Final hardware assembly
- [ ] Project photos
- [ ] Wiring schematic

---

## Features

* Real-time temperature monitoring (°C)
* Realative humidity measurement (%)
* Atmospheric pressure measurement (hPa)
* Live Data visualization on an OLED display
* Periodic sensor updates
* Error handling for sensor initialization failures

---

## Hardware

|Component                      |Description                               |
|-------------------------------|------------------------------------------|
|ESP32-C3 Mini                  |Main controller                           |
|BME680                         |Temperature, humidity ang pressure sensor |
|SH1107 OLED Display            |128x128 display                           |
|4x AA battery holder           |Power supply                              |
|DC-DC Step-Down Buck Converter |Voltage regulator                         |
|Casing                         |Custom 3D printed casing                  |

---

## Software

- ESP_IDF 6.0.1
- PlatformIO as the build environment
- Firmware written in C/C++

### Sensor System
- Custom BME680 driver
- Communication via the ESP-IDF 6 'i2c_master' driver
- Temperature, humidity and pressure measurements
- Temperature offset compensation to reduce self-heating effects

### Display System
- SH1107 OLED driver provided by U8g2
- Custom U8g2 I2C backend for compatibility
- Communication via ESP_IDF 6 'i2c_master' driver
- Custom startup screen with a sun icon

### Logging and Debugging
- ESP_LOG for runtime diagnostics
- Serial output via UART (115200 baud)
- Error handling using 'ESP_ERROR_CHECK()'

### Libraries Used
- U8g2 (OLED graphics libary)
- FreeRTOS (provided by ESP-IDF)
- esp_driver_i2c
- esp_log

### Custom software components
- Custom BME680 driver
- Custom U8g2 to ESP-IDF 6 I2C adapter
- Startup screen implementation
- Sensor data processing and compensation

---

## Schematic


---

## Photos


---

## Installation

1. Clone this repository:

```bash
git clone https://github.com/A-Kananathan/esp32-climate-monitor.git
```

2. Open the project in PlatformIO.

3. Build and upload the firmware to the ESP32-C3 Mini.

4. Connect the hardware as shown in the wiring diagram.

5. Power on the device.
