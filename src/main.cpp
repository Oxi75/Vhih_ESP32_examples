#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "virtualHomee.hpp"
#include "virtualHomee/homee_defines.h"
#include "wifi_defines.h"

// Version und Konstanten
const double FIRMWARE_VERSION_d = 0.10;
const String FIRMWARE_VERSION = String(FIRMWARE_VERSION_d, 1);

// homee Attribute IDs
const uint32_t ID_SHUTTER = 1;
const uint32_t ID_SW_VER = 2;
const uint32_t ID_HW_REV = 3;


// Definitions for virtual homee
const char* vhih_name = "my_vhih";                   // vhih_name (must be unique in the homee network)
const char *virtualDeviceName = "vhih Test-Rolladen";  // virtualDeviceName (must be unique on this vhih)
const uint32_t VHIH_NODE_ID = 88;                    // Homee node ID (must be unique in the homee network)


virtualHomee vhih(vhih_name);       // Homee instance

// Constants for virtual devices
const double HW_REV = 1.0;         // Hardware revision, reported via the "Hardware Revision" attribute
const double VERSION_f = 1.0;      // Software version (can be used for future updates)


// Global variables for virtual devices
bool ID_updated = false;

uint32_t ID = 0xFFFFFFFF;  // Attribute ID of this device


// Function Declarations
void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a);


void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a)
{
  if (ID_updated == true) return;  // Not ready to receive new values

    a->setCurrentValue(a->getTargetValue());
    vhih.updateAttribute(a);
    ID = a->getId();

    Serial.println("Callback received with ID " + String(ID) + " means ESP32 got new value from homee.");

    switch (ID)
    {
      case ID_SHUTTER:
      {
        double buffer = a->getCurrentValue();
        ID_updated = true;
        Serial.print("Received new Shutter action (" + String(buffer) + "): ");
        if (buffer == 0.0)
        {
          Serial.println("open fully");
        }
        else if (buffer == 1.0)
        {
          Serial.println("close completely");
        }
        else if (buffer == 2.0)
        {
          Serial.println("stop moving");
        }
        else
        {
          Serial.println("do nothing");
        }
        break;
      }

      default: ;
    }
}


void homee_setup()
{
  Serial.println("Setup homee (ID" + String(VHIH_NODE_ID) +  ")");
  delay(1000);

  node *n1;
  nodeAttributes *attr;

  // New Device
  n1 = new node(VHIH_NODE_ID, CANodeProfileShutterPositionSwitch, virtualDeviceName);

  // Attribut: Zustand (Rolladen Auf/Stopp/Zu)
  attr = new nodeAttributes(CAAttributeTypeUpDown, ID_SHUTTER);
  attr->setName("Zustand");
  attr->setEditable(true);
  attr->setCallback(callBack_homeeReceiveValue);
  n1->AddAttributes(attr);

  // Attribut: Firmware-Version
  attr = new nodeAttributes(CAAttributeTypeFirmwareRevision, ID_SW_VER);
  attr->setName("Firmware Version");
  attr->setUnit("");
  attr->setCurrentValue(FIRMWARE_VERSION_d);
  attr->setEditable(false);
  attr->setCallback(nullptr);
  n1->AddAttributes(attr);

  // Attribut: Hardware-Revision
  attr = new nodeAttributes(CAAttributeTypeHardwareRevision, ID_HW_REV);
  attr->setName("Hardware Revision");
  attr->setUnit("");
  attr->setCurrentValue(HW_REV);
  attr->setEditable(false);
  attr->setCallback(nullptr);
  n1->AddAttributes(attr);

  // Node zur homee hinzufügen
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
  Serial.println("Example: virtual homee roller shutter node (Zustand: Auf/Stopp/Zu, plus Firmware- and Hardware-Revision).");
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

    ID_updated = false; //you can use ID_updated flag in your main loop to process the new value
}