#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "virtualHomee.hpp"
#include "wifi_defines.h"


// Homee definitions
#define ID_ROOM_TEMP_DST 1
#define ID_ROOM_TEMP_CUR 2
#define ID_RETURN_TEMP 3
#define ID_PUMP_STATE 4
#define ID_SW_VER 5
#define ID_HW_REV 6
#define ID_SIGNAL_LEVEL 7
#define ID_BATT_LEVEL 8
#define ID_BATT_ALARM 9
#define ID_ADDRESS 10

#define CAAttributeTypeBatteryLowAlarm 69
#define CAAttributeTypeNone 0


// Definitions for virtual homee
const char* vhih_name = "my_vhih";                   // vhih_name (must be unique in the homee network)
const char *virtualDeviceName = "vhih Test Device";  // virtualDeviceName (must be unique on this vhih)
const uint32_t VHIH_NODE_ID = 88;                    // Homee node ID (must be unique in the homee network)


virtualHomee vhih(vhih_name);       // Homee instance

// Constants for virtual devices
const double HW_REV = 1.0;         // Hardware revision (can be used for future updates)
const double VERSION_f = 1.0;      // Software version (can be used for future updates)
const unsigned long CHECK_INTERVAL = 10000; // 10 seconds
const unsigned long BattCheckInterval = 150000; // 5 min for battery status / alarm

// Global variables for virtual devices
double tempDest = 0.0;       
double tempRoom = 0.0;          // Default value after restart
double humidityRoom = 0.0;      // Current room humidity from BT device - 0.0 if not available
double tempReturn = 0.0;        // Return-water temperature (measured by DS18B20)
double currentBattery = 0.0;
double battAlarm = 0.0;         // Battery alarm (0 = no alarm, 1 = alarm)
double currentSignalStrength = 0.0;
bool pumpState = false;         // Pump is off by default

bool ID_updated = false;
unsigned long lastCheckTime;
unsigned long lastBattCheckTime;

uint32_t ID = 0xFFFFFFFF;  // Attribute ID of this device


// Function Declarations
void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a);


void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a)
{
  if (ID_updated == true) return;  // Not ready to receive new values

    a->setCurrentValue(a->getTargetValue());
    vhih.updateAttribute(a);
    ID = a->getId();

    Serial.println("Callback received with ID " + String(ID));

    switch (ID)
    {
      case ID_ROOM_TEMP_DST:
      {
        tempDest = a->getCurrentValue();
        ID_updated = true;
        Serial.println("Received new destination temperature " + String(tempDest)+ "°C");
        break;
      }
/* case ID_ROOM_TEMP_CUR:
      {
        tempRoom = a->getCurrentValue();
        Serial.println("Received new room temperature " + String(tempRoom) + "°C");
        break;
      }
*/      
      default: ;
    }
}


void homee_setup()
{
  Serial.println("Setup homee (ID" + String(VHIH_NODE_ID) +  ")");
  delay(1000);

//  homeeMutex = xSemaphoreCreateMutex();

  node *n1;
  nodeAttributes *na;
  

  // New Device
  n1 = new node(VHIH_NODE_ID, 3006, virtualDeviceName);  // 1001 - Bulb, 3001 Thermometer

  // Attribute Room Setpoint Temperature
  na = new nodeAttributes(6, ID_ROOM_TEMP_DST);
  na->setName("Room Setpoint Temp");
  na->setUnit("°C");  
  na->setMinimumValue(5);
  na->setMaximumValue(28); 
  na->setCurrentValue(tempDest);
  na->setCallback(callBack_homeeReceiveValue);
  na->setEditable(true);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Room Current Temperature
  na = new nodeAttributes(5, ID_ROOM_TEMP_CUR);
  na->setName("Room Current Temp");
  na->setUnit("°C");  
  na->setMinimumValue(-10);
  na->setMaximumValue(50); 
  na->setCurrentValue(tempRoom);
  na->setCallback(callBack_homeeReceiveValue);
  na->setEditable(false);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Return Temperature
  na = new nodeAttributes(5, ID_RETURN_TEMP);
  na->setName("Return Temperature");
  na->setUnit("°C");  
  na->setMinimumValue(-20);
  na->setMaximumValue(50); 
  na->setCurrentValue(tempReturn);
  na->setCallback(nullptr);
  na->setEditable(false);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Pump State
  na = new nodeAttributes(1);  // CAAttributeTypeOnOff (1); CAAttributeTypeLEDState (46)
  na->setName("Pump State");
  na->setId(ID_PUMP_STATE);
  na->setUnit(pumpState ? "off" : "on");
  na->setUnit("");
  na->setMinimumValue(0);
  na->setMaximumValue(1); 
  na->setCurrentValue(pumpState ? 0.0 : 1.1); 
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node  

  // Attribute Hardware Revision
  na = new nodeAttributes(43);  // CAAttributeTypeHardwareRevision
  na->setName("Hardware Revision");
  na->setId(ID_HW_REV);
  na->setUnit("");
  na->setMinimumValue(0);
  na->setMaximumValue(1); 
  na->setCurrentValue(HW_REV);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Firmware Version
  na = new nodeAttributes(44);  // CAAttributeTypeFirmwareRevision
  na->setName("Firmware Version");
  na->setId(ID_SW_VER);
  na->setUnit("");  
  na->setMinimumValue(0);
  na->setMaximumValue(100); 
  na->setCurrentValue(VERSION_f);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Battery Level
  na = new nodeAttributes(8);  // CAAttributeTypeBatteryLevel
  na->setName("Battery Level");
  na->setId(ID_BATT_LEVEL);
  na->setUnit("%");  
  na->setMinimumValue(0);
  na->setMaximumValue(100); 
  na->setCurrentValue(currentBattery);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Battery Low Alarm
  na = new nodeAttributes(CAAttributeTypeBatteryLowAlarm);
  na->setName("Battery Low Alarm");
  na->setId(ID_BATT_ALARM);
  na->setUnit("");  
  na->setMinimumValue(0);
  na->setMaximumValue(1); 
  na->setCurrentValue(battAlarm);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Sensor Address
  na = new nodeAttributes(CAAttributeTypeNone);
  na->setName("Sensor Address");
  na->setId(ID_ADDRESS);
  na->setUnit("0xFFFF FFFF");  
  na->setMinimumValue(0);
  na->setMaximumValue(999999999999); 
  na->setCurrentValue(32234234);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node

  // Attribute Signal Strength
  na = new nodeAttributes(2);  // CAAttributeTypeDimmingLevel
  na->setName("Signal Strength");
  na->setId(ID_SIGNAL_LEVEL);
  na->setUnit("%");  
  na->setMinimumValue(0);
  na->setMaximumValue(100); 
  na->setCurrentValue(currentSignalStrength);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       // Set attribute to node

  // Add Device
  vhih.addNode(n1);

  vhih.start();

  Serial.println("Homee configured");
  Serial.println("");
  delay(1000);
}


void homee_updateValues()
{  
  nodeAttributes *na;
  Serial.println("Update homee values");
//  Serial.printf("Stack High Water Mark: %d\n", uxTaskGetStackHighWaterMark(NULL));

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

  // Update Room-Temperature
  na = vhih.getAttributeById(ID_ROOM_TEMP_CUR);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, tempRoom);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homee attribute updated: tempRoom");
  } 

  // Update Return Temperature
  na = vhih.getAttributeById(ID_RETURN_TEMP);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, tempReturn);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homee attribute updated: tempReturn");
  } 

  // Update Pump State (0 = off, 1 = on)
  na = vhih.getAttributeById(ID_PUMP_STATE);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
//    na->setUnit(pumpState ? "off" : "on");
    vhih.updateAttributeValue(na, pumpState ? 0.0 : 1.0);  // Those values show nothing but are needed to update the unit value
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homee attribute updated: Unit");
  }

  // Update Signal Strength
  na = vhih.getAttributeById(ID_SIGNAL_LEVEL);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, currentSignalStrength);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homee attribute updated: SignalStrength");
  }

  // Update Battery Level
  na = vhih.getAttributeById(ID_BATT_LEVEL);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, currentBattery);  
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homee attribute updated: Battery");
  }


  // Update Battery Alarm
  na = vhih.getAttributeById(ID_BATT_ALARM);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, battAlarm);  
    delay(200);
    yield();
    Serial.println("homee attribute updated: Battery Alarm");
  }

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


    // Temperature check (except Bluetooth)
    if (millis() - lastCheckTime > CHECK_INTERVAL)
    {
      lastCheckTime = millis();

      Serial.println("Check temperature and humidity values");
      // Here you can insert code to check temperature and humidity

      homee_updateValues();

        Serial.println("Back from homee update");
    }


    // Check battery status
    if (millis() - lastBattCheckTime > BattCheckInterval)
    {
      lastBattCheckTime = millis();

      if (battAlarm == 0.0)
      {
        Serial.println("Set battery alarm");
        battAlarm = 1.0;
      }
      else
      {
        Serial.println("Reset battery alarm");
        battAlarm = 0.0;
      } 

      homee_updateValues();
      Serial.println("Back from homee update");
    }
}