#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "virtualHomee.hpp"
#include "wifi_defines.h"


//homee definitionen
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

//definitions for virtual homee
virtualHomee vhih("my_vhih");      //homee instance
const uint32_t VHIH_NODE_ID = 23;  //homee node ID (must be unique in the homee network)

//constants for virtual devices
const double HW_REV = 1.0;         //hardware revision (can be used for future updates)
const double VERSION_f = 1.0;      //software version (can be used for future updates)
const unsigned long CHECK_INTERVAL = 10000; // 10 Sekunden
const unsigned long BattCheckInterval = 150000; // 5 min für Batteriestatus / -alarm

//global variables for virtual devices
double tempDest = 0.0;       
double tempRoom = 0.0;          //default value after restart
double humidityRoom = 0.0;                //current room humidity from BT device - 0.0 if not available
double tempReturn = 0.0;      //return-water temperature (measured by DS18B20)
double currentBattery = 0.0;
double battAlarm = 0.0;  //battery alarm (0 = no alarm, 1 = alarm)
double currentSignalStrength = 0.0;
bool pumpState = false;  //pump is off by default

bool ID_updated = false;
unsigned long lastCheckTime;
unsigned long lastBattCheckTime;

uint32_t ID = 0xFFFFFFFF;  //attribute ID of this device


//Function Declarations
void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a);


void IRAM_ATTR callBack_homeeReceiveValue(nodeAttributes* a)
{
  if (ID_updated == true) return;  //not ready to reiceive new values

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
/*      
      case ID_ROOM_TEMP_CUR:
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
  Serial.println("setup homee (ID" + String(VHIH_NODE_ID) +  ")");
  delay(1000);

//  homeeMutex = xSemaphoreCreateMutex();

  node *n1;
  nodeAttributes *na;
  

  //Neues Gerät
  n1 = new node(VHIH_NODE_ID, 3006, "vhih Testgeraet");  //1001 - Glühbirne, 3001 Thermomether

  //Attribut Raum-Soll-Temperatur
  na = new nodeAttributes(6, ID_ROOM_TEMP_DST);  //Raum-Soll-Temperatur
  na->setName("Raum-Soll-Temperatur");
  na->setUnit("°C");  
  na->setMinimumValue(5);
  na->setMaximumValue(28); 
  na->setCurrentValue(tempDest);
  na->setCallback(callBack_homeeReceiveValue);
  na->setEditable(true);
  n1->AddAttributes(na);       //set attribute to node

  //Attribut Raum-Ist-Temperatur
  na = new nodeAttributes(5, ID_ROOM_TEMP_CUR);  //Raum-Ist-Temperatur
  na->setName("Raum-Ist-Temperatur");
  na->setUnit("°C");  
  na->setMinimumValue(-10);
  na->setMaximumValue(50); 
  na->setCurrentValue(tempRoom);
  na->setCallback(callBack_homeeReceiveValue);
  na->setEditable(false);
  n1->AddAttributes(na);       //set attribute to node

  //Attribut Rücklauftemperatur
  na = new nodeAttributes(5, ID_RETURN_TEMP);  //Rücklauftemperatur
  na->setName("Rücklauftemperatur");
  na->setUnit("°C");  
  na->setMinimumValue(-20);
  na->setMaximumValue(50); 
  na->setCurrentValue(tempReturn);
  na->setCallback(nullptr);
  na->setEditable(false);
  n1->AddAttributes(na);       //set attribute to node

  //Attribut Pumpenstatus
  na = new nodeAttributes(1);  //CAAttributeTypeOnOff (1); CAAttributeTypeLEDState (46)
  na->setName("Pumpenstatus");
  na->setId(ID_PUMP_STATE);
  na->setUnit(pumpState ? "aus" : "ein");
  na->setUnit("");
  na->setMinimumValue(0);
  na->setMaximumValue(1); 
  na->setCurrentValue(pumpState ? 0.0 : 1.1); 
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node  

  na = new nodeAttributes(43);  //CAAttributeTypeHardwareRevision
  na->setName("Hardware Revision");
  na->setId(ID_HW_REV);
  na->setUnit("");
  na->setMinimumValue(0);
  na->setMaximumValue(1); 
  na->setCurrentValue(HW_REV);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node

  na = new nodeAttributes(44);  //CAAttributeTypeFirmwareRevision
  na->setName("Firmware Version");
  na->setId(ID_SW_VER);
  na->setUnit("");  
  na->setMinimumValue(0);
  na->setMaximumValue(100); 
  na->setCurrentValue(VERSION_f);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node


  na = new nodeAttributes(8);  //CAAttributeTypeBatteryLevel
  na->setName("Battery Level");
  na->setId(ID_BATT_LEVEL);
  na->setUnit("%");  
  na->setMinimumValue(0);
  na->setMaximumValue(100); 
  na->setCurrentValue(currentBattery);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node


  na = new nodeAttributes(CAAttributeTypeBatteryLowAlarm);  //CAAttributeTypeBatteryLevel
  na->setName("Battery Low Alarm");
  na->setId(ID_BATT_ALARM);
  na->setUnit("");  
  na->setMinimumValue(0);
  na->setMaximumValue(1); 
  na->setCurrentValue(battAlarm);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node


  na = new nodeAttributes(CAAttributeTypeNone);  //
  na->setName("Sensor Address");
  na->setId(ID_ADDRESS);
  na->setUnit("0xFFFF FFFF");  
  na->setMinimumValue(0);
  na->setMaximumValue(999999999999); 
  na->setCurrentValue(32234234);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node

 

  na = new nodeAttributes(2);  //CAAttributeTypeDimmingLevel
  na->setName("Signal Strength");
  na->setId(ID_SIGNAL_LEVEL);
  na->setUnit("%");  
  na->setMinimumValue(0);
  na->setMaximumValue(100); 
  na->setCurrentValue(currentSignalStrength);
  na->setEditable(false);
  na->setCallback(nullptr);
  n1->AddAttributes(na);       //set attribute to node

  //Gerät hinzufügen
  vhih.addNode(n1);

  vhih.start();

  Serial.println("Homee configured");
  delay(1000);
}


void homee_updateValues()
{  
  nodeAttributes *na;
  Serial.println("update homee values");
//  Serial.printf("Stack High Water Mark: %d\n", uxTaskGetStackHighWaterMark(NULL));

  // Aktualisierung der Zieltemperatur
  na = vhih.getAttributeById(ID_ROOM_TEMP_DST);
  if (na)
  {
  //  Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, tempDest);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homme attribute updated: tempDest");
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
    Serial.println("homme attribute updated: tempRoom");
  } 

  // Aktualisierung der Rücklauftemperatur
  na = vhih.getAttributeById(ID_RETURN_TEMP);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, tempReturn);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homme attribute updated: tempReturn");
  } 

  // Aktualisierung des Pumpenzustands (0 = aus, 1 = ein)
  na = vhih.getAttributeById(ID_PUMP_STATE);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
//    na->setUnit(pumpState ? "aus" : "ein");
    vhih.updateAttributeValue(na, pumpState ? 0.0 : 1.0);  //Those values shows nothing but are needed to update the unit value
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homme attribute updated: Unit");
  }

  // Update Signal-Strength
  na = vhih.getAttributeById(ID_SIGNAL_LEVEL);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, currentSignalStrength);
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homme attribute updated: SignalStrength");
  }

  //Update Battery-Level
  na = vhih.getAttributeById(ID_BATT_LEVEL);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, currentBattery);  
    delay(200);
    yield();
//    Serial.printf(", after: %u", ESP.getFreeHeap());
    Serial.println("homme attribute updated: Battery");
  }


  //Update Battery-Alarm
  na = vhih.getAttributeById(ID_BATT_ALARM);
  if (na)
  {
//    Serial.printf("FreeHeap before: %u", ESP.getFreeHeap());
    vhih.updateAttributeValue(na, battAlarm);  
    delay(200);
    yield();
    Serial.println("homme attribute updated: Battery Alarm");
  }

  Serial.println("homee Update done");
}




/***********************************************************************
/** System Setup
/***********************************************************************/


void WiFi_setup()
{
    Serial.println("Connect to WLAN");
    
    if (!WiFi.config(client_ip, gateway, subnet)) 
    {
        Serial.println("Error: Could not configure static IP address");
    }

  if (ssid.length() > 32)
  {
    Serial.println("SSID too long, using first 32 characters only.");
    ssid = ssid.substring(0, 32);
  }
  if (password.length() > 64)
  {
    Serial.println("Password too long, using first 64 characters only.");
    password = password.substring(0, 64);
  }
  
  WiFi.begin(ssid.c_str(), password.c_str());
  uint32_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (i > 40)
    {
      Serial.println("WiFi failed!");
      delay(3000);
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
    i++;
  }

  Serial.println("WLAN connected");
  delay(1000);
}


void setup()
{
    // Serieller Monitor
    Serial.begin(115200);
    Serial.println();
    Serial.println("*******************************************");
    Serial.println("VHIH Testprogramm" + String(", V") + String(VERSION_f));

    WiFi_setup();
    homee_setup();
}

void WiFi_check(bool restart = true)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WLAN off -> restart");
    delay(3000);   // Kurze Verzögerung zur Ausgabe der Fehlermeldung  
    ESP.restart(); // ESP32 neu starten
  }
}



/*************************************************************************************
/**   from here, you'll find the functions for the standard FBH control mode mode   **
/*************************************************************************************/
static bool firstCall = true;
void loop()
{
    if (firstCall)
    {
      Serial.println("Enter control loop for the first time.");
      firstCall = false;
    }
    yield();  //delay is not allowed here, because homee connection would become unstable
    
    WiFi_check();

    
    // Taster-Verarbeitung
    uint32_t currentTime = millis();


    // Temperaturprüfung (außer Bluetooth)
    if (millis() - lastCheckTime > CHECK_INTERVAL)
    {
      lastCheckTime = millis();

      Serial.println("Check temperature and humidity values");
      // Hier können Sie den Code zur Überprüfung der Temperatur und Luftfeuchtigkeit einfügen

      homee_updateValues();

        Serial.println("back from homee update");
    }


    // Batteriestatus "prüfen"
    if (millis() - lastBattCheckTime > BattCheckInterval)
    {
      lastBattCheckTime = millis();

      if (battAlarm == 0.0)
      {
        Serial.println("set battery alarm");
        battAlarm = 1.0;
      }
      else
      {
        Serial.println("reset battery alarm");
        battAlarm = 0.0;
      } 

      homee_updateValues();
      Serial.println("back from homee update");
    }

    
}
