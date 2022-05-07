/////////////////////////////////////////

#ifdef CLASSIC
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>
  #define CHAR_READ      BLECharacteristic::PROPERTY_READ 
  #define CHAR_WRITE     BLECharacteristic::PROPERTY_WRITE
  #define CHAR_NOTIFY    BLECharacteristic::PROPERTY_NOTIFY
  #define CHAR_INDICATE  BLECharacteristic::PROPERTY_INDICATE
#else
  #include <NimBLEDevice.h>  
  #define CHAR_READ      NIMBLE_PROPERTY::READ 
  #define CHAR_WRITE     NIMBLE_PROPERTY::WRITE
  #define CHAR_NOTIFY    NIMBLE_PROPERTY::NOTIFY
  #define CHAR_INDICATE  NIMBLE_PROPERTY::INDICATE
#endif

// Had to put these her to make the compiler work but not really sure why

BLEService* newService(BLEServer *server, const char *service_UUID, int num_chars);
BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties);
BLECharacteristic* newChar(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len);


// Server callbacks
class SCServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected");
    BLEDevice::startAdvertising();
  };
  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected - start advertising");
    BLEDevice::startAdvertising();
  };
};

// Characteristic callbacks
class SCCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic){
    Serial.print("<<<< ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(": onRead(), value: ");

    int j, l;
    const char *p;
    byte b;
    l = pCharacteristic->getValue().length();
    p = pCharacteristic->getValue().c_str();
    for (j = 0; j < l; j++) {
      b = p[j];
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();
  };
  void onWrite(BLECharacteristic* pCharacteristic) {
    Serial.print(">>>> ");
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(": onWrite(), value: ");

    int j, l;
    const char *p;
    byte b;
    l = pCharacteristic->getValue().length();
    p = pCharacteristic->getValue().c_str();
    for (j = 0; j < l; j++) {
      b = p[j];
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();
  };
};

static SCCharacteristicCallbacks SCchrCallbacks;

// define here so loop() can see it
BLECharacteristic* pcData1;

BLEService* newService(BLEServer *server, const char *service_UUID, int num_chars) {
  #ifdef CLASSIC
    return server->createService(BLEUUID(service_UUID), num_chars);
  #else
    return server->createService(service_UUID);
  #endif
}

BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties) {
  BLECharacteristic *pChar;

  pChar = pService->createCharacteristic(char_UUID, properties);
    pChar->setCallbacks(&SCchrCallbacks);
  #ifdef CLASSIC
    if (properties & CHAR_NOTIFY)
       pChar->addDescriptor(new BLE2902());
  #endif 
  return pChar;
}

BLECharacteristic* newChar(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len) {
  BLECharacteristic *pChar;

  pChar = newCharNoVal(pService, char_UUID, properties);
  pChar->setValue(data, data_len); 
  return pChar;
}


void SparkControlStart() {
    uint8_t dat[1];
    uint8_t val[1];
    Serial.println("Spark Control emulation");
    static BLEServer* pSCServer;
        
    BLEDevice::init("SKC50S-4 v3.0.1 9E02");
    BLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    pSCServer = BLEDevice::createServer();
    pSCServer->setCallbacks(new SCServerCallbacks());
    // Device Information
    BLEService* psDevInf = newService(pSCServer, "180a", 40);        
    uint8_t manuf[7]={'A', 'i', 'r', 'T', 'u', 'r', 'n'};
    BLECharacteristic* pcManufName = newChar(   psDevInf, "2a29", CHAR_READ, manuf, 7); 
    uint8_t model[8]={'B','T','5','0','0','S','-','4'};
    BLECharacteristic* pcModelNumber = newChar( psDevInf, "2a24", CHAR_READ, model, 8);
    uint8_t hwrev[8]={'H','7','.','0','.','0'};
    BLECharacteristic* pcHwRev = newChar(       psDevInf, "2a27", CHAR_READ, hwrev, 6);
    uint8_t fwrev[6]={'F','3','.','0','.','1'};    
    BLECharacteristic* pcFwRev = newChar(       psDevInf, "2a26", CHAR_READ, fwrev, 6);
    uint8_t serial[16]={'7','8','5','0','3','5','4','0','C','6','6','9','9','E','0','2'};   
    BLECharacteristic* pcSerialNumber = newChar(psDevInf, "2a25", CHAR_READ, serial, 16);
    uint8_t systemid[8] = {0x5c, 0x2f, 0xe0, 0xfe, 0xff, 0x7f, 0xab, 0xdb};
    BLECharacteristic* pcSystemID = newChar(    psDevInf, "2a23", CHAR_READ, systemid, 8);
    uint8_t pnpid[7] = {0x01, 0x22, 0x01, 0x0b, 0x00, 0x01, 0x00};
    BLECharacteristic* pcPnPID = newChar(       psDevInf, "2a50", CHAR_READ, pnpid, 7);

    // Battery Service
    BLEService* psBat = newService(pSCServer, "180f", 40);
    val[0] = 80;    
    BLECharacteristic* pcBatLvl = newChar(psBat, "2a19", CHAR_READ | CHAR_NOTIFY, val, 1);

    // Service A
    BLEService* psA = newService(pSCServer, "34452F38-9E44-46AB-B171-0CC578FEB928", 60);
    uint8_t A1[14] = {0x04, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x83, 0x00};    
    BLECharacteristic* pcA1 = newChar(psA, "CAD0C949-7DCE-4A04-9D80-E767C796B392", CHAR_READ, A1, 14);
    val[0] = 0x0F;
    BLECharacteristic* pcA2 = newChar(psA, "76E7DF30-E3AD-41B1-A05A-279C80FC7FB4", CHAR_READ, val, 1);
    val[0] = 0x00l;
    BLECharacteristic* pcA3 = newChar(psA, "CDB3A16B-02E1-4BC0-843C-665CBEB378B7", CHAR_READ, val, 1);
    uint8_t datA4[2] = {0x00, 0x01};     
    BLECharacteristic* pcA4 = newChar(psA, "53EEA35F-27F1-46E4-A790-34BECD28B701", CHAR_READ | CHAR_NOTIFY, datA4, 2);
    val[0] = 0x00;
    BLECharacteristic* pcA5 = newChar(psA, "54eea35f-27f1-46e4-a790-34becd28b701", CHAR_READ | CHAR_NOTIFY, val, 1);

    // B Service
    BLEService* psB = newService(pSCServer, "6FACFE71-A4C3-4E80-BA5C-533928830727", 20);
    val[0] = 0x00;
    BLECharacteristic* pcB1 = newChar(psB, "90D9A098-9CD8-4A7A-B176-91FFE80909F2", CHAR_READ | CHAR_NOTIFY, val, 1);

    // C Service
    BLEService* psC = newService(pSCServer, "5cb68410-6774-11e4-9803-0800200c9a66", 20);
    BLECharacteristic* pcC1 = newCharNoVal(psC, "407eda40-6774-11e4-9803-0800200c9a66", CHAR_WRITE);

    // Data Service
    BLEService* psData = newService(pSCServer, "7bdb8dc0-6c95-11e3-981f-0800200c9a66", 40);
    val[0] = 0x00;
    pcData1 =                    newChar(psData, "362f71a0-6c96-11e3-981f-0800200c9a66", CHAR_READ | CHAR_NOTIFY, val, 1);
    uint8_t datData2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    BLECharacteristic* pcData2 = newChar(psData, "BD066DA4-F9EC-4F0D-A53C-1CD99147A641", CHAR_READ | CHAR_NOTIFY, datData2, 16);
    val[0] = 0x01;    
    BLECharacteristic* pcData3 = newChar(psData, "85da4f4b-a2ca-4c7c-8c7d-fcd9e2daad56", CHAR_READ | CHAR_NOTIFY, val, 1);

#ifndef CLASSIC        
    // E Service
    BLEService* psE = newService(pSCServer, "FE59", 20);
    BLECharacteristic* pcE1 = newCharNoVal(psE, "8ec90003-f315-4f60-9fb8-838830daea50", CHAR_WRITE | CHAR_INDICATE);

    // F Service        
    BLEService* psF = newService(pSCServer, "25a22330-820f-11e3-baa7-0800200c9a66", 20);
    uint8_t datF1[12] = {0x32, 0x2c, 0x73, 0x31, 0x33, 0x32, 0x5f, 0x37, 0x2e, 0x32, 0x2e, 0x30};
    BLECharacteristic* pcF1 = newChar(psF, "ba7cc552-cc2c-404e-bf75-8778f023787d", CHAR_READ, datF1, 12);
        
    // G Service            
    BLEService* psG = newService(pSCServer, "03b80e5a-ede8-4b33-a751-6ce34ec4c700", 20);
    BLECharacteristic* pcG1 = newCharNoVal(psG, "7772e5db-3868-4112-a1a9-f2669d106bf3", CHAR_READ | CHAR_WRITE | CHAR_NOTIFY);

    // H Service
    BLEService* psH = newService(pSCServer, "97a16290-8c08-11e3-baa8-0800200c9a66", 60);
    val[0] = 0x00;    
    BLECharacteristic* pcH1 = newChar(     psH, "640c0d80-9b4f-11e3-a5e2-0800200c9a66", CHAR_READ | CHAR_WRITE, val, 1);
    val[0] = 0x00;    
    BLECharacteristic* pcH2 = newChar(     psH, "49aa8950-a40d-11e3-a5e2-0800200c9a66", CHAR_READ | CHAR_WRITE, val, 1);
    BLECharacteristic* pcH3 = newCharNoVal(psH, "3673862E-780A-4936-9C56-5477A6C62B94", CHAR_READ);
    uint8_t datH4[19] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
    BLECharacteristic* pcH4 = newChar(     psH, "fbeeeb29-c82a-4fb6-a5f7-aee512c93790", CHAR_READ | CHAR_WRITE, datH4, 19);
    val[0] = 0x28;
    BLECharacteristic* pcH5 = newChar(     psH, "75cbbe2f-09ec-4b34-86c4-6124c4dd68f4", CHAR_READ, val, 1);
    BLECharacteristic* pcH6 = newCharNoVal(psH, "1bcf02e0-d53e-468c-9d6d-e9251230d8c9", CHAR_READ | CHAR_WRITE);
    uint8_t datH7[2] = {0x20, 0x1c}; 
    BLECharacteristic* pcH7 = newChar(     psH, "7b51af7d-f28a-48be-8081-6a647e6249eb", CHAR_READ, datH7, 2);
    val[0] = 0x01; 
    BLECharacteristic* pcH8 = newChar(     psH, "db3207d4-97ff-4497-ad1d-0ba1bccc56ba", CHAR_READ | CHAR_WRITE, val, 1);
    val[0] = 0x01;
    BLECharacteristic* pcH9 = newChar(     psH, "b8980e72-799c-4172-af97-46380afb068c", CHAR_READ | CHAR_WRITE, val, 1);
    uint8_t datHA[2] = {0x32, 0x00}; 
    BLECharacteristic* pcHA = newChar(     psH, "13252451-fc8e-46e7-8dce-c799276bd61c", CHAR_READ | CHAR_WRITE, datHA, 2);
    BLECharacteristic* pcHB = newCharNoVal(psH, "29eaf09d-996d-4d94-a989-ed320fbac5c9", CHAR_WRITE);
    BLECharacteristic* pcHC = newCharNoVal(psH, "733ee9cc-174d-452E-b45c-0402aa75ff75", CHAR_READ | CHAR_WRITE);
  #endif

    psDevInf->start();
    psBat->start();
    psA->start();
    psB->start();
    psC->start();    
    psData->start();
  #ifndef CLASSIC 
    psE->start(); 
    psF->start();
    psG->start();
    psH->start();
  #endif

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(psData->getUUID());

#ifdef CLASSIC
    BLEAdvertisementData oAdvertisementData  = BLEAdvertisementData();
    oAdvertisementData.setName("SKC50S-4 v3.0.1 9E02");
    uint8_t adv_data[] = {0x22, 0x01, 0x01, 0x1b};
    oAdvertisementData.setManufacturerData(std::string((char *)adv_data, 4)); 
    oAdvertisementData.setAppearance(960);    
    pAdvertising->setAdvertisementData(oAdvertisementData);
    //pAdvertising->setScanResponseData(oAdvertisementData);
#else
    pAdvertising->setName("SKC50S-4 v3.0.1 9E02");
    uint8_t adv_data[] = {0x22, 0x01, 0x01, 0x1b};
    pAdvertising->setManufacturerData(std::string((char *)adv_data, 4)); 
//    pAdvertising->setAppearance(960);   

    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
#endif
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}


#ifdef ACTIVE_HIGH
  uint8_t logicON = HIGH;
  uint8_t logicOFF = LOW;
#else
  uint8_t logicON = LOW;
  uint8_t logicOFF = HIGH;
#endif



uint8_t switchPins[]{33,14,27,26}; 
uint8_t SCswitchPins[]{33,27,14,26}; 
uint8_t last_switch = 0;
uint8_t now_switch = 0;
uint8_t sw_dat[1];

void InitialiseGPIO() {
  for (int i = 0; i < 4; i++) {    
  #ifdef ACTIVE_HIGH
    pinMode(switchPins[i], INPUT_PULLDOWN);
  #else
    pinMode(switchPins[i], INPUT_PULLUP);
  #endif
  }
}

void SparkControlLoop() {
    now_switch = 0;  
    for (int i = 0; i < 4; i++) {
      int v = digitalRead(SCswitchPins[i]) == logicON ? 1 : 0;
      now_switch |= (v << i);
    }
    if (now_switch != last_switch) {
      sw_dat[0] = now_switch;
      pcData1->setValue(sw_dat, 1);
      pcData1->notify();
      last_switch = now_switch;
      Serial.print("Switched ");
      Serial.println(now_switch);
      delay(200);
    }     
}    

/////////////////////////////////////////
