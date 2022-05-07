#include "SparkControlSpoofer.h"

#define HELTEC
#define CLASSIC

#ifdef HELTEC
#include "heltec.h"
#endif

bool spark_box;

void setup() {
// general setup
#ifdef HELTEC
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
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
