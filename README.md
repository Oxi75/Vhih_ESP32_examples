# VHIH ESP32 Example – Roller Shutter Control

This repository contains **multiple PlatformIO-based ESP32 test applications**, each located on a separate branch.

The currently selected branch (**`rollershutter`**) contains the example described below.

The project runs a **virtual homee instance (vhih) on an ESP32** and exposes a single **roller shutter (roller blind) node**.
It is intended as a **reference and learning example** and cannot control real hardware.

---

## What this example does

- Creates one virtual homee node of type "Rolladensteuerung" (`CANodeProfileShutterPositionSwitch`).
- Exposes exactly three attributes on that node:
  - **Zustand** – Auf / Stopp / Zu (`CAAttributeTypeUpDown`). Editable: homee sends a command
    (0 = open, 1 = close, 2 = stop), the ESP32 acknowledges it and logs the received action to
    the serial console.
  - **Firmware Version** (`CAAttributeTypeFirmwareRevision`) – static, read-only.
  - **Hardware Revision** (`CAAttributeTypeHardwareRevision`) – static, read-only.
- Logs to the serial console (RS232/USB):
  - The WiFi connection status, the vhih name and its IP address, plus a one-line description
    of the example, once WiFi is connected.
  - Every value received from homee (attribute ID and the resulting action), via
    `callBack_homeeReceiveValue()`.

This makes the example a minimal, easy-to-read starting point for building your own
virtual homee node with a control attribute plus version reporting.

---

## Development Environment

- PlatformIO
- Framework: Arduino
- Two build targets are defined in `platformio.ini`:
  - `esp32-c3-supermini` (default) – ESP32-C3-SuperMini board (`esp32-c3-devkitm-1`, native USB-CDC serial)
  - `esp32dev` – classic ESP32 dev boards (e.g. MH-ET Live)
  - Additional targets can be added the same way.
  - ESP8266 is not tested and not supported by this setup - but is most likely possible by modifying the `platformio.ini`
- Select a target explicitly with `pio run -e esp32dev` / `pio run -e esp32-c3-supermini`, or
  just flash with no `-e` flag to use the default (`esp32-c3-supermini`).

---

## WiFi Configuration

WiFi settings are stored in a separate, git-ignored configuration file so credentials never
get committed.

1. Copy or rename `include/wifi_defines.h.txt` to `include/wifi_defines.h`.
2. Open `wifi_defines.h` and fill in your own values:
   - `client_ip` – static IP address the ESP32 should use
   - `gateway` – your router's IP address
   - `subnet` – your subnet mask
   - `ssid` – your WiFi network name
   - `password` – your WiFi password

The application will not compile without this file (it is referenced by `src/main.cpp` and
excluded from git via `include/.gitignore`).

---

## Connecting the ESP32 to homee

Once the ESP32 is powered on and connected to your WiFi, the virtual homee device is
automatically available on the network (check the serial monitor for its IP address).

To add it to your homee system:

1. Open the homee app or the homee Web UI as an administrator.
2. Add a new device.
3. Select **Choose device → Miscellaneous → homee in homee**.
4. In step **2a – Connect homee**:
   - As *homee ID*, enter either the virtual homee name (`vhih_name` in `src/main.cpp`,
     `"my_vhih"` by default) or its IP address.
   - Username and password are not checked by the vhih, so any values work
     (e.g. `xxx` / `xxx`). Recommendation: always use the same username and password for all
     your vhih devices - since they're not actually verified, a consistent pair avoids
     confusion without any downside.

---

## Compatibility, usability and general notes

This example runs without any real roller-shutter hardware. You just need your homee system,
access to your WiFi and an ESP32 (or ESP32-C3) board connected to your computer.

### Used Libraries

- `ESPAsyncWebServer` @ 3.7.2
  https://github.com/ESP32Async/ESPAsyncWebServer

- `AsyncTCP` @ 3.3.6
  https://github.com/ESP32Async/AsyncTCP

- `bblanchon/ArduinoJson` @ 7.3.1

- `homee-api-esp32` (fork with support for new ESPAsyncWebServer)
  https://github.com/Oxi75/homee-api-esp32


---

## Disclaimer

This project is **not affiliated** with homee GmbH or codeatelier GmbH.
