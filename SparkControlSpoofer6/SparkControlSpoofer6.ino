// USES SPARK BOX TO EMULATE A SPARK CONTROL


// Edit NimBLEDevice.cpp

/*
  //uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_PUBLIC;
  uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_RANDOM;
*/



// UPDATE 
//    ACTIVE_HIGH define 
//    uint8_t switchPins[]{33,14,27,26}; 
//    uint8_t SCswitchPins[]{33,27,14,26}; 
// for your GPIO setup
// note SCswitchPins is in a strange order - [0][2][1][3]
      
//#define ACTIVE_HIGH

//#define HELTEC
//#define M5CORE
#define M5CORE2
//#define M5STICK

//#define BLUEDROID
#define FULL_SERVICES

#ifdef HELTEC
#include "heltec.h"
#endif

#ifdef M5CORE2
#include <M5Core2.h>
#endif

#ifdef M5CORE
#include <M5Stack.h>
#endif

#ifdef M5STICK
#include <M5StickC.h>
#endif


void SparkControlStart();
void InitialiseGPIO();
void SparkControlLoop();
bool spark_box;

void setup() {
// general setup
#ifdef HELTEC
  Heltec.begin(true , false , true );
  Heltec.display->setFont(ArialMT_Plain_24); // 10, 16 and 24
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Spk Control");
  #ifdef BLUEDROID
  Heltec.display->drawString(0, 30, "BLUEDR");
  #else
  Heltec.display->drawString(0, 30, "NimBLE");
  #endif
  #ifdef FULL_SERVICES
  Heltec.display->drawString(100, 30, "F");
  #else
  Heltec.display->drawString(100, 30, "P");
  #endif
  Heltec.display->display();
#elif defined M5CORE2 || defined M5CORE || defined M5STICK
  M5.begin();
#ifdef M5CORE
  M5.Power.begin();
#endif
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.println("Spk Control");
  #ifdef BLUEDROID
  M5.Lcd.println("BLUEDROID");
  #else
  M5.Lcd.println("NimBLE");
  #endif
  #ifdef FULL_SERVICES
  M5.Lcd.println("Full");
  #else
  M5.Lcd.println("Partial");
  #endif
#else
  Serial.begin(115200);
#endif
  
  Serial.println("Starting BLE Server");
  spark_box = false;
  InitialiseGPIO();
  if (spark_box) {
  }
  else {
    SparkControlStart();
    Serial.println("Ready");
  }
}

void loop() {
  if (spark_box) {
  }
  else { 
    SparkControlLoop();
  }
}
