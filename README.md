# VHIH ESP32 Example

This repository contains **multiple PlatformIO-based ESP32 test applications**, each located on a separate branch.

The currently selected branch (**`general_vhih_new_libs`**) contains the example described below and uses newer versions of the required libraries.
The branch **`general_vhih_classic`** uses the original vhih library and its historical dependencies.

The project runs a **virtual homee instance on an ESP32** and exposes a test device with multiple attributes.  
It is intended as a **reference and learning example**, not as the vhih library itself.

---

## Development Environment

- PlatformIO  
- Framework: Arduino  
- Target: ESP32 (`esp32dev`)  
  - Additional targets can be added to `platformio.ini`
  - ESP8266 is not tested and not supported by this setup

The project is configured and tested using the versions defined in `platformio.ini`.

---

## Purpose of the Example

The example application:

- Creates a **virtual homee (node)** running on an ESP32
- Exposes a **test device** with several attributes
- Demonstrates bidirectional communication:
  - Values sent from the ESP32 to homee
  - Commands and value changes sent from homee to the ESP32

The virtual homee uses the following default settings, which can be adjusted **at the beginning of `main.cpp`**:

- Homee name: **`my_vhih`**
- Homee ID: **88**
- Device (node) name: **`vhih Test Device`**

All values can be changed to match your setup.  
Make sure that **all IDs and names are unique within your homee network**.

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

## Dependencies

### Compatibility Note

This example requires **relatively new (but not the latest)** versions of the networking and JSON libraries.

Due to these updated dependencies, the project is **no longer compatible with the original vhih library (v0.2.7)**.

Instead, it uses a **vhih fork adapted for newer ESPAsyncWebServer versions**.

The goal is a working setup with current ESP32 async networking libraries,  
not backward compatibility with the historical vhih implementation.

---

### Used Libraries

- `ESPAsyncWebServer` @ 3.7.2  
  https://github.com/ESP32Async/ESPAsyncWebServer  

- `AsyncTCP` @ 3.3.6  
  https://github.com/ESP32Async/AsyncTCP  

- `bblanchon/ArduinoJson` @ 7.3.1  

- `homee-api-esp32` (fork with support for new ESPAsyncWebServer)  
  https://github.com/Oxi75/homee-api-esp32#feature/new_ESPAsyncWebServer

---

## Notes

- This project is meant as a **minimal and understandable reference implementation**
- It focuses on **API usage and data flow**, not on production readiness
- Only **one virtual node** is created by default

---

## Disclaimer

This project is **not affiliated** with homee GmbH or codeatelier GmbH.
