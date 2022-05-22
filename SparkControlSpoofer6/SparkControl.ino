/////////////////////////////////////////
// Good link on standard services: 
// https://docs.springcard.com/books/SpringCore/Host_interfaces/Physical_and_Transport/Bluetooth/Standard_Services
/////////////////////////////////////////



#ifdef BLUEDROID
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

// Had to put these here to make the compiler work but not really sure why

BLEService* newService(BLEServer *server, const char *service_UUID, int num_chars);
BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties);
BLECharacteristic* newCharData(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len);
BLECharacteristic* newCharVal(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t val);


void show_addr(uint8_t val[6]) {
  for (int i = 0; i < 6; i++) {
    int v = val[5-i];
    if (v < 16) Serial.print("0");
    Serial.print(v, HEX); 
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

// Server callbacks
class SCServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected");
    //BLEDevice::startAdvertising();
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
  BLEService *pService;
  #ifdef BLUEDROID
    pService =  server->createService(BLEUUID(service_UUID), num_chars);
  #else
    pService = server->createService(service_UUID);
  #endif
  return pService;
}

BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties) {
  BLECharacteristic *pChar;

  pChar = pService->createCharacteristic(char_UUID, properties);
  pChar->setCallbacks(&SCchrCallbacks);
  #ifdef BLUEDROID
    if (properties & CHAR_NOTIFY)
       pChar->addDescriptor(new BLE2902());
  #endif 
  return pChar;
}

BLECharacteristic* newCharData(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len) {
  BLECharacteristic *pChar;

  pChar = newCharNoVal(pService, char_UUID, properties);
  pChar->setValue(data, data_len); 
  return pChar;
}

BLECharacteristic* newCharVal(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t val) {
  BLECharacteristic *pChar;

  uint8_t dat[1];

  dat[0] = val;
  pChar = newCharNoVal(pService, char_UUID, properties);
  pChar->setValue(dat, 1); 
  return pChar;
}


void SparkControlStart() {
    uint8_t dat[1];
    uint8_t val[1];
    Serial.println("Spark Control emulation");
    static BLEServer* pSCServer;
        
    BLEDevice::init("SKC50S-4 v3.0.1 9E02"); // " 9E02"
    BLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */


    
    pSCServer = BLEDevice::createServer();
    pSCServer->setCallbacks(new SCServerCallbacks());
    // Device Information
    BLEService* psDevInf = newService(pSCServer, "180a", 20);        
    uint8_t manuf[7]={'A', 'i', 'r', 'T', 'u', 'r', 'n'};
    BLECharacteristic* pcManufName = newCharData(   psDevInf, "2a29", CHAR_READ, manuf, 7); 
    uint8_t model[8]={'B','T','5','0','0','S','-','4'};
    BLECharacteristic* pcModelNumber = newCharData( psDevInf, "2a24", CHAR_READ, model, 8);
    uint8_t hwrev[8]={'H','7','.','0','.','0'};
    BLECharacteristic* pcHwRev = newCharData(       psDevInf, "2a27", CHAR_READ, hwrev, 6);
    uint8_t fwrev[6]={'F','3','.','0','.','1'};    
    BLECharacteristic* pcFwRev = newCharData(       psDevInf, "2a26", CHAR_READ, fwrev, 6);
    uint8_t serial[16]={'7','8','5','0','3','5','4','0','C','6','6','9','9','E','0','2'};   
    BLECharacteristic* pcSerialNumber = newCharData(psDevInf, "2a25", CHAR_READ, serial, 16);
    uint8_t systemid[8] = {0x5c, 0x2f, 0xe0, 0xfe, 0xff, 0x7f, 0xab, 0xdb};
    BLECharacteristic* pcSystemID = newCharData(    psDevInf, "2a23", CHAR_READ, systemid, 8);
    uint8_t pnpid[7] = {0x01, 0x22, 0x01, 0x0b, 0x00, 0x01, 0x00};
    BLECharacteristic* pcPnPID = newCharData(       psDevInf, "2a50", CHAR_READ, pnpid, 7);

    // Battery Service
    BLEService* psBat = newService(pSCServer, "180f", 10);
    BLECharacteristic* pcBatLvl = newCharVal(psBat, "2a19", CHAR_READ | CHAR_NOTIFY, 80);

    // Service A
    BLEService* psA = newService(pSCServer, "34452F38-9E44-46AB-B171-0CC578FEB928", 20);
    uint8_t A1[14] = {0x04, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x83, 0x00};    
    BLECharacteristic* pcA1 = newCharData(psA, "CAD0C949-7DCE-4A04-9D80-E767C796B392", CHAR_READ, A1, 14);
    BLECharacteristic* pcA2 = newCharVal(psA, "76E7DF30-E3AD-41B1-A05A-279C80FC7FB4", CHAR_READ, 0x0f);
    BLECharacteristic* pcA3 = newCharVal(psA, "CDB3A16B-02E1-4BC0-843C-665CBEB378B7", CHAR_READ, 0x01);
    uint8_t datA4[2] = {0x00, 0x01};     
    BLECharacteristic* pcA4 = newCharData(psA, "53EEA35F-27F1-46E4-A790-34BECD28B701", CHAR_READ | CHAR_NOTIFY, datA4, 2);
    BLECharacteristic* pcA5 = newCharVal(psA, "54eea35f-27f1-46e4-a790-34becd28b701", CHAR_READ | CHAR_NOTIFY, 0x00);

    // B Service
    BLEService* psB = newService(pSCServer, "6FACFE71-A4C3-4E80-BA5C-533928830727", 10);
    BLECharacteristic* pcB1 = newCharVal(psB, "90D9A098-9CD8-4A7A-B176-91FFE80909F2", CHAR_READ | CHAR_NOTIFY, 0x00);

    // C Service
    BLEService* psC = newService(pSCServer, "5cb68410-6774-11e4-9803-0800200c9a66", 10);
    BLECharacteristic* pcC1 = newCharNoVal(psC, "407eda40-6774-11e4-9803-0800200c9a66", CHAR_WRITE);

    // Data Service
    BLEService* psData = newService(pSCServer, "7bdb8dc0-6c95-11e3-981f-0800200c9a66", 10);
    pcData1 = newCharVal(psData, "362f71a0-6c96-11e3-981f-0800200c9a66", CHAR_READ | CHAR_NOTIFY, 0x00);
    uint8_t datData2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    BLECharacteristic* pcData2 = newCharData(psData, "BD066DA4-F9EC-4F0D-A53C-1CD99147A641", CHAR_READ | CHAR_NOTIFY, datData2, 16);
    BLECharacteristic* pcData3 = newCharVal(psData, "85da4f4b-a2ca-4c7c-8c7d-fcd9e2daad56", CHAR_READ | CHAR_NOTIFY, 0x01);

#ifdef FULL_SERVICES   
    // E Service
    BLEService* psE = newService(pSCServer, "FE59", 10);
    BLECharacteristic* pcE1 = newCharNoVal(psE, "8ec90003-f315-4f60-9fb8-838830daea50", CHAR_WRITE | CHAR_INDICATE);
  
    // F Service        
    BLEService* psF = newService(pSCServer, "25a22330-820f-11e3-baa7-0800200c9a66", 10);
    uint8_t datF1[12] = {0x32, 0x2c, 0x73, 0x31, 0x33, 0x32, 0x5f, 0x37, 0x2e, 0x32, 0x2e, 0x30};
    BLECharacteristic* pcF1 = newCharData(psF, "ba7cc552-cc2c-404e-bf75-8778f023787d", CHAR_READ, datF1, 12);
        
    // G Service            
    BLEService* psG = newService(pSCServer, "03b80e5a-ede8-4b33-a751-6ce34ec4c700", 10);
    BLECharacteristic* pcG1 = newCharNoVal(psG, "7772e5db-3868-4112-a1a9-f2669d106bf3", CHAR_READ | CHAR_WRITE | CHAR_NOTIFY);

    // H Service
    BLEService* psH = newService(pSCServer, "97a16290-8c08-11e3-baa8-0800200c9a66", 40);
    BLECharacteristic* pcH1 = newCharVal(psH, "640c0d80-9b4f-11e3-a5e2-0800200c9a66", CHAR_READ | CHAR_WRITE, 0x00);
    BLECharacteristic* pcH2 = newCharVal(psH, "49aa8950-a40d-11e3-a5e2-0800200c9a66", CHAR_READ | CHAR_WRITE, 0x00);
    BLECharacteristic* pcH3 = newCharNoVal(psH, "3673862E-780A-4936-9C56-5477A6C62B94", CHAR_READ);
    uint8_t datH4[19] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
    BLECharacteristic* pcH4 = newCharData(psH, "fbeeeb29-c82a-4fb6-a5f7-aee512c93790", CHAR_READ | CHAR_WRITE, datH4, 19);
    BLECharacteristic* pcH5 = newCharVal(psH, "75cbbe2f-09ec-4b34-86c4-6124c4dd68f4", CHAR_READ, 0x28);
    BLECharacteristic* pcH6 = newCharNoVal(psH, "1bcf02e0-d53e-468c-9d6d-e9251230d8c9", CHAR_READ | CHAR_WRITE);
    uint8_t datH7[2] = {0x20, 0x1c}; 
    BLECharacteristic* pcH7 = newCharData(psH, "7b51af7d-f28a-48be-8081-6a647e6249eb", CHAR_READ, datH7, 2);
    BLECharacteristic* pcH8 = newCharVal(psH, "db3207d4-97ff-4497-ad1d-0ba1bccc56ba", CHAR_READ | CHAR_WRITE, 0x01);
    BLECharacteristic* pcH9 = newCharVal(psH, "b8980e72-799c-4172-af97-46380afb068c", CHAR_READ | CHAR_WRITE, 0x01);
    uint8_t datHA[2] = {0x32, 0x00}; 
    BLECharacteristic* pcHA = newCharData(psH, "13252451-fc8e-46e7-8dce-c799276bd61c", CHAR_READ | CHAR_WRITE, datHA, 2);
    BLECharacteristic* pcHB = newCharNoVal(psH, "29eaf09d-996d-4d94-a989-ed320fbac5c9", CHAR_WRITE);
    BLECharacteristic* pcHC = newCharNoVal(psH, "733ee9cc-174d-452E-b45c-0402aa75ff75", CHAR_READ | CHAR_WRITE);
  #endif

    psDevInf->start();
    psBat->start();
    psA->start();
    psB->start();
    psC->start();    
    psData->start();
  #ifdef FULL_SERVICES
    psE->start(); 
    psF->start();
    psG->start();
    psH->start();
  #endif

    psDevInf->dump();
    psBat->dump();
    psA->dump();
    psB->dump();
    psC->dump();    
    psData->dump();
  #ifdef FULL_SERVICES
    psE->dump(); 
    psF->dump();
    psG->dump();
    psH->dump();
  #endif

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
      
  
#ifdef BLUEDROID      
    BLEAdvertisementData oAdvertisementData  = BLEAdvertisementData();
    BLEAdvertisementData oScanAdvertisementData = BLEAdvertisementData();  
   
    oScanAdvertisementData.setName("SKC50S-4 v3.0.1 9E02");
    oScanAdvertisementData.setAppearance(960);  
      
    uint8_t adv_data[] = {0x22, 0x01, 0x01, 0x0b};
    oAdvertisementData.setManufacturerData(std::string((char *)adv_data, 4)); 
    oAdvertisementData.setPartialServices(BLEUUID("7bdb8dc0-6c95-11e3-981f-0800200c9a66"));
    
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanAdvertisementData);
 
#else
    
    char scan_data[] = {0x15, 0x09, 'S', 'K', 'C', '5', '0', 'S', '-', '4', ' ', 'v', '3', '.', '0', '.', '1',
      ' ','9', 'E', '0', '2',
      0x03, 0x19, 0xc0, 0x03};
    char adv_data[] = {0x02, 0x01, 0x06, 
      0x05, 0xff, 0x22, 0x01, 0x01, 0x0b, 
      0x11, 0x06, 0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x1f, 0x98, 0xe3, 0x11, 0x95, 0x6c, 0xc0, 0x8d, 0xdb, 0x7b};

    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanAdvertisementData = BLEAdvertisementData();  

    oScanAdvertisementData.addData(scan_data, 26);
    oAdvertisementData.addData(adv_data, 27);

    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanAdvertisementData);

#endif

    ble_addr_t blead;
    int rc;

    rc = ble_hs_id_gen_rnd(1, &blead);
    if (rc != 0) Serial.println("Rand failed");

    rc = ble_hs_id_set_rnd(blead.val);
    if (rc != 0) Serial.println("Addr failed");

    rc = ble_hs_id_copy_addr(BLE_ADDR_PUBLIC, blead.val, NULL);
    if (rc == 0) show_addr(blead.val);
    rc = ble_hs_id_copy_addr(BLE_ADDR_RANDOM, blead.val, NULL);
    if (rc == 0) show_addr(blead.val);

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

// pins                 1  3  2  4
uint8_t SCswitchPins[]{33,27,14,26}; 
uint8_t last_switch = 0;
uint8_t now_switch = 0;
uint8_t sw_dat[1];

void InitialiseGPIO() {
  for (int i = 0; i < 4; i++) {    
  #ifdef ACTIVE_HIGH
    pinMode(SCswitchPins[i], INPUT_PULLDOWN);
  #else
    pinMode(SCswitchPins[i], INPUT_PULLUP);
  #endif
  }
}

void SparkControlLoop() {

    now_switch = 0;  
    for (int i = 0; i < 4; i++) {
      int v = digitalRead(SCswitchPins[i]) == logicON ? 1 : 0;
      now_switch |= (v << i);
    }
    #if defined M5CORE2 || defined M5CORE
      M5.update();
      if (M5.BtnA.isPressed()) now_switch = 1;
      if (M5.BtnB.isPressed()) now_switch = 4;
      if (M5.BtnC.isPressed()) now_switch = 2;            
    #endif
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
