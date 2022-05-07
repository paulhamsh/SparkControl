// USES SPARK BOX TO EMULATE A SPARK CONTROL

// UPDATE 
//    ACTIVE_HIGH define 
//    uint8_t switchPins[]{33,14,27,26}; 
//    uint8_t SCswitchPins[]{33,27,14,26}; 
// for your GPIO setup
// notSCswitchPins is the same as switchPins but with items [1] and [2] swapped around -so  [0][1][2][3] and [0][2][1][3]
      
//#define ACTIVE_HIGH
#define HELTEC
//#define CLASSIC


#ifdef HELTEC
#include "heltec.h"
#endif

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
  Heltec.display->display();
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
