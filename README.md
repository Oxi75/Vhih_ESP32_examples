# VHIH ESP32 Example

This repository contains a **PlatformIO-based example application** for an ESP32 that demonstrates how to use the **vhih (virtual homee interface)** library.

The project runs a **virtual homee instance on an ESP32** and exposes a test device with multiple attributes.  
It is intended as a **reference and learning example**, not as the vhih library itself.

---

## Development Environment

- PlatformIO  
- Framework: Arduino  
- Target: ESP32 (`esp32dev`)  
  - Additional targets can be added to `platformio.ini`
  - ESP8266 is not tested, but should work when using the appropriate networking, JSON, and web server libraries

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

This example intentionally uses **specific (older) library versions** to maintain **backward compatibility with the original vhih implementation**.

For using newer libraries, select a different branch of this repository.

The goal is that this example works:

- With the **original vhih library**
- Without requiring API changes or behavioral adjustments

Using newer library versions may work, but is **not the target of this example**.

---

### Used Libraries

- `bblanchon/ArduinoJson`@6.17.3

- `ESPAsyncWebServer` 
  https://github.com/DanielKnoop/ESPAsyncWebServer@1.2.7

- `ESPAsyncTCP` V 1.2.2
  https://github.com/earlephilhower/ESPAsyncTCP


- `homee-api-esp32`:

  https://github.com/Oxi75/homee-api-esp32#feature/stabilityImprovements

  or alternatively the original:  
   
  https://github.com/DanielKnoop/homee-api-esp32@0.2.7


---

## Notes

- This project is meant as a **minimal and understandable reference implementation**
- It focuses on **API usage and data flow**, not on production readiness
- Only **one virtual node** is created by default

---

## Disclaimer

This project is **not affiliated** with homee GmbH or codeatelier GmbH.
