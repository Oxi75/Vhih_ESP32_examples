#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "virtualHomee.hpp"
#include "wifi_defines.h"

// Version und Konstanten
const double FIRMWARE_VERSION_d = 0.10;
const String FIRMWARE_VERSION = String(FIRMWARE_VERSION_d, 1);

// homee Attribute IDs
const uint32_t ID_SHUTTER = 1;
const uint32_t ID_3rd_POSITION = 2;
const uint32_t ID_ENABLE = 3;
const uint32_t ID_SW_VER = 4;


// Definitions for virtual homee
const char* vhih_name = "my_vhih";                   // vhih_name (must be unique in the homee network)
const char *virtualDeviceName = "vhih Test-Rolladen";  // virtualDeviceName (must be unique on this vhih)
const uint32_t VHIH_NODE_ID = 88;                    // Homee node ID (must be unique in the homee network)


virtualHomee vhih(vhih_name);       // Homee instance

// Constants for virtual devices
const double HW_REV = 1.0;         // Hardware revision (can be used for future updates)
const double VERSION_f = 1.0;      // Software version (can be used for future updates)


// Global variables for virtual devices
double shutterPos = NAN;
bool ID_updated = false;

// Timing variables
uint32_t lastCheckTime = 0;
const uint32_t CHECK_INTERVAL = 20000;  // Interval to check sensor values [ms]

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
          shutterPos = 2; //open
          Serial.println("open fully");
          break;
        }
        if (buffer == 1.0)
        {
          shutterPos = 0; //closed
          Serial.println("close completely");
          break;
        }
        if (buffer == 2.0)
        {
          shutterPos = -1; //stop moving
          Serial.println("stop moving");          
          break;
        }
        shutterPos = -2; //neutral value to indicate no action
        Serial.println("do nothing");
        break;
      }

      case ID_3rd_POSITION:
      {
        double buffer = a->getCurrentValue();
        ID_updated = true;

        if (buffer == 0.0) shutterPos = 0.0;       //close
        else if (buffer == 1.0) shutterPos = 1.0; //middle position
        else if (buffer == 2.0) shutterPos = 2.0; //open
        else shutterPos = -2.0; //neutral value to indicate no action

        Serial.print("Received new Shutter action (" + String(shutterPos) + "): ");
        if (shutterPos == 0.0)
        {
          Serial.println("close completely");
          break;
        }
        if (shutterPos == 1.0)
        {
          Serial.println("move to middle position");
          break;
        }
        if (shutterPos == 2.0)
        {
          Serial.println("open fully");
          break;
        }
        Serial.println("do nothing");

        break;
      }

      default: ;
    }
}


void homee_setup()
{
  Serial.println("Setup homee (ID" + String(VHIH_NODE_ID) +  ")");
  delay(1000);

//  homeeMutex = xSemaphoreCreateMutex();

  node *n1;
  nodeAttributes *attr;
  

  // New Device
  n1 = new node(VHIH_NODE_ID, 2002, virtualDeviceName);  // 2002 - Rolladensteuerung

  // Attribut: Rolladen hoch
  attr = new nodeAttributes(135, ID_SHUTTER);
  attr->setEditable(true);
  attr->setCallback(callBack_homeeReceiveValue);
  n1->AddAttributes(attr);


  //Attribut for (3rd) position
  //if you know how fast your roller shutter moves, you can calculate the exact position value (0..100%)
  //or you can set it to a fixed value like 0 = close, 1 = middel position, 2 = open
  const uint32_t CAAttributeTypePosition = 15; // On/Off attribute type
  attr = new nodeAttributes(CAAttributeTypePosition, ID_3rd_POSITION);  //open / closed state
  attr->setName("3rd Position");
  attr->setMaximumValue(2.0);
  attr->setMinimumValue(0.0);
  attr->setEditable(true);
  attr->setCallback(callBack_homeeReceiveValue);
  n1->AddAttributes(attr);       //set attribute to node


  // Attribut: OnOff
  attr = new nodeAttributes(1, ID_ENABLE);
  attr->setName("enabled");
  attr->setUnit("");
  attr->setCurrentValue(1.0);
  attr->setMaximumValue(1.0);
  attr->setMinimumValue(0.0);
  attr->setEditable(true);
  attr->setCallback(callBack_homeeReceiveValue);
  n1->AddAttributes(attr);
    
  // Attribut: Firmware-Version
  attr = new nodeAttributes(44, ID_SW_VER);
  attr->setName("Firmware Version");
  attr->setUnit("");
  attr->setCurrentValue(FIRMWARE_VERSION_d);
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


void homee_updateValues()
{  
  nodeAttributes *na;
  Serial.println("Update homee values (sending value from ESP32 to homee)");
//  Serial.printf("Stack High Water Mark: %d\n", uxTaskGetStackHighWaterMark(NULL));

/*
  // Update target temperature
  na = vhih.getAttributeById(ID_ROOM_TEMP_DST);
  if (na)
  {
  //  Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, tempDest);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homee attribute updated: tempDest");
  } 
*/


  Serial.println("homee update done");
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

void WiFi_check(bool restart = true)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WLAN off -> restart");
    delay(3000);   // Short delay to output the error message
    ESP.restart(); // Restart ESP32
  }
}



/*************************************************************************************
/** from here, you'll find the functions for the standard FBH control mode mode   **
/*************************************************************************************/
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

    
    // Button processing
    uint32_t currentTime = millis();


    //check for new (sensor) values every CHECK_INTERVAL milliseconds and update homee
    if (millis() - lastCheckTime > CHECK_INTERVAL)
    {
      lastCheckTime = millis();

      // Serial.println("Check sensor values");
      // Here you can insert code to check temperature and humidity

      homee_updateValues();

      Serial.println("Back from homee update");
    }

    ID_updated = false; //you can use ID_updated flag in your main loop to process the new value

}