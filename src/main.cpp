#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "virtualHomee.hpp"
#include "virtualHomee/homee_defines.h"
#include "virtualHomee/homee_icons.h"
#include "wifi_defines.h"

// Version and constants
const double FIRMWARE_VERSION_d = 0.10;
const String FIRMWARE_VERSION = String(FIRMWARE_VERSION_d, 1);

// homee attribute IDs
const uint32_t ID_UP_DOWN = 1;             // CAAttributeTypeUpDown: 0 = open, 1 = close, 2 = stop
const uint32_t ID_VENTILATE_IMPULSE = 2;   // CAAttributeTypeVentilateImpulse: 0 = idle, 1 = move to 3rd position
const uint32_t ID_SW_VER = 3;
const uint32_t ID_HW_REV = 4;

// Values for the UpDown attribute
const double UP_DOWN_OPEN = 0.0;
const double UP_DOWN_CLOSE = 1.0;
const double UP_DOWN_STOP = 2.0;

// Values for the VentilateImpulse attribute
const double VENTILATE_IDLE = 0.0;
const double VENTILATE_3RD_POSITION = 1.0;


// Definitions for virtual homee
const char* vhih_name = "my_vhih";                     // vhih_name (must be unique in the homee network)
const char *virtualDeviceName = "vhih Test-Rolladen";   // virtualDeviceName (must be unique on this vhih)
const uint32_t VHIH_NODE_ID = 88;                       // Homee node ID (must be unique in the homee network)

// No documented homee node profile renders all four attributes below as controls in the WebUI
// at once (CANodeProfileShutterPositionSwitch, for example, hides the VentilateImpulse/3rd-position
// button). An unmapped profile value makes homee fall back to rendering every editable attribute
// generically, which is the only way found so far to get all four controls to show up.
// TODO: add a proper named profile constant for this to the homee-api-esp32 library.
const uint32_t NODE_PROFILE_GENERIC = (uint32_t)(-1);


virtualHomee vhih(vhih_name);       // Homee instance

// Constants for virtual devices
const double HW_REV = 1.0;         // Hardware revision, reported via the "Hardware Revision" attribute
const double VERSION_f = 1.0;      // Software version (can be used for future updates)


// Global variables for virtual devices
bool ID_updated = false;

uint32_t ID = 0xFFFFFFFF;  // Attribute ID of this device

nodeAttributes* attrUpDown = nullptr;         // set once in homee_setup(), used by the 3rd-position impulse for testing
nodeAttributes* activeImpulseAttr = nullptr; // impulse attribute currently waiting to be reset to 0
uint32_t impulseResetTime = 0;
bool impulseActive = false;
const uint32_t IMPULSE_RESET_DELAY = 500;    // ms - how long the impulse attribute stays at 1


// Function Declarations
void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a);
void scheduleImpulseReset(nodeAttributes* a);


// Marks an impulse attribute to be reset back to 0 after IMPULSE_RESET_DELAY (see loop())
void scheduleImpulseReset(nodeAttributes* a)
{
  activeImpulseAttr = a;
  impulseResetTime = millis();
  impulseActive = true;
}

void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a)
{
  if (ID_updated == true) return;  // Not ready to receive new values

  a->setCurrentValue(a->getTargetValue());
  vhih.updateAttribute(a);
  ID = a->getId();

  Serial.println("Callback received with ID " + String(ID) + " means ESP32 got new value from homee.");

  switch (ID)
  {
    case ID_UP_DOWN:
      ID_updated = true;
      if (a->getTargetValue() == UP_DOWN_OPEN)       Serial.println("Received command: Open");
      else if (a->getTargetValue() == UP_DOWN_CLOSE) Serial.println("Received command: Close");
      else if (a->getTargetValue() == UP_DOWN_STOP)  Serial.println("Received command: Stop");
      break;

    case ID_VENTILATE_IMPULSE:
      if (a->getTargetValue() >= 0.5)  // only act on the press (value 1), not on the auto-reset to 0
      {
        ID_updated = true;
        Serial.println("Received impulse: Move to 3rd position");

        // Test: push an out-of-range value to the UpDown attribute and see how homee reacts
        if (attrUpDown != nullptr)
        {
          attrUpDown->setCurrentValue(UP_DOWN_STOP);
          vhih.updateAttribute(attrUpDown);
        }

        scheduleImpulseReset(a);
      }
      break;

    default: ;
  }
}


void homee_setup()
{
  Serial.println("Setup homee (ID " + String(VHIH_NODE_ID) +  ")");
  delay(1000);

  node *n1;
  nodeAttributes *attr;

  // New virtual device: roller shutter with Up/Down/Stop control and a 3rd-position impulse
  n1 = new node(VHIH_NODE_ID, NODE_PROFILE_GENERIC, virtualDeviceName);
  n1->setImage(NodeIconShutter);

  // Attribute: Up/Down/Stop control (0 = open, 1 = close, 2 = stop)
  attr = new nodeAttributes(CAAttributeTypeUpDown, ID_UP_DOWN);
  attr->setName("Up/Down");
  attr->setMinimumValue(UP_DOWN_OPEN);
  attr->setMaximumValue(UP_DOWN_STOP);
  attr->setCurrentValue(UP_DOWN_STOP);
  attr->setStepValue(1.0);
  attr->setEditable(true);
  attr->setCallback(callBack_homeeReceiveValue);
  n1->AddAttributes(attr);
  attrUpDown = attr;

  // Attribute: Ventilate impulse, used to trigger the 3rd position (0 = idle, 1 = move to 3rd position)
  attr = new nodeAttributes(CAAttributeTypeOpenPartialImpulse, ID_VENTILATE_IMPULSE);
  attr->setName("3rd Position");
  attr->setMinimumValue(VENTILATE_IDLE);
  attr->setMaximumValue(VENTILATE_3RD_POSITION);
  attr->setCurrentValue(VENTILATE_IDLE);
  attr->setStepValue(1.0);
  attr->setEditable(true);
  attr->setCallback(callBack_homeeReceiveValue);
  n1->AddAttributes(attr);

  // Attribute: Firmware revision
  attr = new nodeAttributes(CAAttributeTypeFirmwareRevision, ID_SW_VER);
  attr->setName("Firmware Revision");
  attr->setUnit("");
  attr->setCurrentValue(FIRMWARE_VERSION_d);
  attr->setEditable(false);
  attr->setCallback(nullptr);
  n1->AddAttributes(attr);

  // Attribute: Hardware revision
  attr = new nodeAttributes(CAAttributeTypeHardwareRevision, ID_HW_REV);
  attr->setName("Hardware Revision");
  attr->setUnit("");
  attr->setCurrentValue(HW_REV);
  attr->setEditable(false);
  attr->setCallback(nullptr);
  n1->AddAttributes(attr);

  // Add node to homee
  vhih.addNode(n1);

  vhih.start();

  Serial.println("Homee configured");
  Serial.println("");
  delay(1000);
}


/***********************************************************************
/** System Setup
/***********************************************************************/


void WiFi_setup()
{
  Serial.println("connecting to:");
  Serial.println("  WiFi-name: " + ssid);
  Serial.println("  Password: " + password);
  Serial.println("  Gateway-IP: " + gateway.toString());
  Serial.println("  subnet-Mask: " + subnet.toString());

  if (!WiFi.config(client_ip, gateway, subnet))
  {
    Serial.println("... Error: Could not configure static IP address");
  }

  if (ssid.length() > 32)
  {
    Serial.println("... SSID too long, using first 32 characters only.");
    ssid = ssid.substring(0, 32);
  }
  if (password.length() > 64)
  {
    Serial.println("... Password too long, using first 64 characters only.");
    password = password.substring(0, 64);
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  uint32_t i = 0;

  Serial.print("..");

  while (WiFi.status() != WL_CONNECTED)
  {
    if (i > 40)
    {
      Serial.println(" failed");
      delay(3000);
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
    i++;
  }

  Serial.println(" done");

  // Output final connection details
  Serial.println("");
  Serial.print("New virtual homee (""" + String(vhih_name) + """) has IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Example: virtual homee roller shutter node (Up/Down/Stop, 3rd-position impulse, plus Firmware- and Hardware-Revision).");
  Serial.println("");

  delay(1000);
}


void setup()
{
    // Serial Monitor
    Serial.begin(115200);
    Serial.println();
    Serial.println("*******************************************");
    Serial.println("VHIH Test Program" + String(", V") + String(VERSION_f));

    WiFi_setup();
    homee_setup();
}

void WiFi_check()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WLAN off -> restart");
    delay(3000);   // Short delay to output the error message
    ESP.restart(); // Restart ESP32
  }
}



static bool firstCall = true;
void loop()
{
    if (firstCall)
    {
      Serial.println("Enter control loop for the first time.");
      firstCall = false;
    }
    yield();  // Delay is not allowed here, because homee connection would become unstable

    WiFi_check();

    // Reset a pressed impulse button back to 0 once IMPULSE_RESET_DELAY has passed
    if (impulseActive && (millis() - impulseResetTime >= IMPULSE_RESET_DELAY))
    {
      impulseActive = false;
      if (activeImpulseAttr != nullptr)
      {
        activeImpulseAttr->setCurrentValue(0.0);
        vhih.updateAttribute(activeImpulseAttr);
        activeImpulseAttr = nullptr;
      }
    }

    ID_updated = false; //you can use ID_updated flag in your main loop to process the new value
}
