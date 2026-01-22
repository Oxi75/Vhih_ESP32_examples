# VHIH ESP32 Examples

This repository contains **example and test projects** for using the **vhih (virtual homee interface)** on ESP32 devices.

The `develop` branch itself does **not** contain a runnable example.  
It serves as an **entry point and overview** for the repository.

All actual example applications are located on **separate branches**.

---

## Repository Structure

Each branch in this repository contains a **self-contained PlatformIO project** with a specific focus or library setup.

To work with an example, switch to the corresponding branch and follow the instructions in its `README.md`.

---

## Available Example Branches

### `general_vhih_new_libs`

- ESP32 example project using **newer (but not latest)** versions of:
  - ESPAsyncWebServer
  - AsyncTCP
  - ArduinoJson
- Uses a **vhih fork adapted to the newer async networking stack**
- Not compatible with the original vhih library (v0.2.7)
- Demonstrates:
  - Running a virtual homee on an ESP32
  - Bidirectional communication between ESP32 and homee

---

### `general_vhih_classic`

- ESP32 example project using the **original vhih library**
- Uses the **historical dependency versions** as recommended by the original author
- Maintains **full backward compatibility** with vhih v0.2.7
- Demonstrates the same virtual homee concept using the classic setup

---

## How to Use

1. Clone the repository
2. Switch to the desired branch:
   ```bash
   git checkout <branch-name>
