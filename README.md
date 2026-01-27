# VHIH ESP32 Example – Roller Shutter Control

This repository contains **multiple PlatformIO-based ESP32 test applications**, each located on a separate branch.

The currently selected branch (**`rollershutter`**) contains the example described below.  

The project runs a **virtual homee instance on an ESP32** and exposes a **roller shutter (roller blind) device with multiple positions**.  
It is intended as a **reference and learning example** and cannot control real hardware.

---

## Development Environment

- PlatformIO  
- Framework: Arduino  
- Target: ESP32 (`esp32dev`)  
  - Additional targets can be added to `platformio.ini`
  - ESP8266 is not tested and not supported by this setup - but is most likely possible by modifying the `platformio.ini`

---

## Purpose of the Example

The example application:

- Creates a **virtual homee (node)** running on an ESP32
- Exposes a **roller shutter device** with **multiple position-related attributes**
- Demonstrates typical roller shutter functionality:
  - Absolute positioning (closed, fully opened)
  - Intermediate position
  - Movement commands from homee to the ESP32
- Demonstrates bidirectional communication:
  - Position and state values sent from the ESP32 to homee
  - Control commands and target position changes sent from homee to the ESP32

This makes the example suitable as a **template for real roller shutter or blind controllers**.

---

## WiFi Configuration

WiFi settings are stored in a separate configuration file.

1. Rename  
   `wifi_defines.h.txt` → `wifi_defines.h`
2. Open the file and configure:
   - Static IP address
   - Gateway
   - Subnet mask
   - WiFi SSID
   - WiFi password

The application will not compile without this file.

---

## Connecting a New Homee Device

Once the ESP32 is powered on and connected to your WiFi, the virtual homee device is automatically available.

To add it to your homee system:

1. Open the homee app or the homee Web UI as an administrator
2. Add a new device
3. Select  
   **Choose device → Miscellaneous → homee in homee**
4. In step **2a – Connect homee**:
   - As *homee ID*, enter either the virtual homee name or its IP address
   - Username and password can be set to arbitrary values (e.g. `xxx`)

---

## Compatibility, usablity and general nodes

This example is only prepared for beeing used on an ESP32 MH-ET Live board.
By adjusting the platformio.ini you can adapt it to other ESP32 and ESP8266 system.

This example runs without a real roller-shutter hardware. You just need your homee-system, 
access to your WiFi and of course an ESP32 board connected to your computer.


### Used Libraries

- `ESPAsyncWebServer` @ 3.7.2  
  https://github.com/ESP32Async/ESPAsyncWebServer  

- `AsyncTCP` @ 3.3.6  
  https://github.com/ESP32Async/AsyncTCP  

- `bblanchon/ArduinoJson` @ 7.3.1  

- `homee-api-esp32` (fork with support for new ESPAsyncWebServer)  
  https://github.com/Oxi75/homee-api-esp32#feature/new_ESPAsyncWebServer

  **note: in the platformio.ini, section lib_deps, you can switch to older library version**

---

## Disclaimer

This project is **not affiliated** with homee GmbH or codeatelier GmbH.
