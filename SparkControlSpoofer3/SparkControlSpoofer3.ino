// USES SPARK BOX TO EMULATE A SPARK CONTROL

// UPDATE 
//    ACTIVE_HIGH define 
//    uint8_t switchPins[]{33,14,27,26}; 
//    uint8_t SCswitchPins[]{33,27,14,26}; 
// for your GPIO setup
// notSCswitchPins is the same as switchPins but with items [1] and [2] swapped around -so  [0][1][2][3] and [0][2][1][3]
      
//#define ACTIVE_HIGH

//#define HELTEC
//#define M5CORE2
#define M5CORE
//#define CLASSIC

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
  #ifdef CLASSIC
  Heltec.display->drawString(0, 30, "CLASSIC");
  #else
  Heltec.display->drawString(0, 30, "NimBLE");
  #endif
  #ifdef FULL_SERVICES
  Heltec.display->drawString(100, 30, "F");
  #else
  Heltec.display->drawString(100, 30, "P");
  #endif
  Heltec.display->display();
#elif defined M5CORE2 || defined M5CORE
  M5.begin();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.println("Spk Control");
  #ifdef CLASSIC
  M5.Lcd.println("CLASSIC");
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
