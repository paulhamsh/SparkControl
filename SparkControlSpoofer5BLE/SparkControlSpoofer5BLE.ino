#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define CHAR_READ      BLECharacteristic::PROPERTY_READ 
#define CHAR_WRITE     BLECharacteristic::PROPERTY_WRITE
#define CHAR_NOTIFY    BLECharacteristic::PROPERTY_NOTIFY
#define CHAR_INDICATE  BLECharacteristic::PROPERTY_INDICATE

 
bool conns;
unsigned long ti;
uint8_t state;
int sm;
uint8_t dat[1];
uint8_t val[1];
uint8_t switches[4] = {1, 4, 2, 8};

static BLEServer* pServer;

//////// Server callbacks

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected");
        BLEDevice::startAdvertising();
    };
    void onDisconnect(BLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        BLEDevice::startAdvertising();
    };
};

//////// Characteristic callbacks

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public BLECharacteristicCallbacks {
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
        conns = true;
        
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

static CharacteristicCallbacks chrCallbacks;

// define here to loop() can see it
BLECharacteristic* pcData1;


BLECharacteristic* newChar(BLEService *pService, const char *char_UUID, uint8_t properties, uint8_t *data, int data_len)
{
    BLECharacteristic *pChar;
    
    pChar = pService->createCharacteristic(char_UUID, properties);
    pChar->setValue(data, data_len); 
    pChar->setCallbacks(&chrCallbacks);
    if (properties & CHAR_NOTIFY)
       pChar->addDescriptor(new BLE2902()); 
    return pChar;
}

BLECharacteristic* newCharNoVal(BLEService *pService, const char *char_UUID, uint8_t properties)
{
    BLECharacteristic *pChar;
    
    pChar = pService->createCharacteristic(char_UUID, properties);
    pChar->setCallbacks(&chrCallbacks);
    if (properties & CHAR_NOTIFY)
       pChar->addDescriptor(new BLE2902()); 
    return pChar;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Server");

    conns = false;
    
    BLEDevice::init("SKC50S-4 v3.0.1 9E02");
    BLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
//    BLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

// Device Information
    BLEService* psDevInf = pServer->createService(BLEUUID("180a"),40);
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
    BLEService* psBat = pServer->createService(BLEUUID("180f"), 40);
    val[0] = 80;    
    BLECharacteristic* pcBatLvl = newChar(psBat, "2a19", CHAR_READ | CHAR_NOTIFY, val, 1);

// Service A
    BLEService* psA = pServer->createService(BLEUUID("34452F38-9E44-46AB-B171-0CC578FEB928"), 60);
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
    BLEService* psB = pServer->createService(BLEUUID("6FACFE71-A4C3-4E80-BA5C-533928830727"), 20);
    val[0] = 0x00;
    BLECharacteristic* pcB1 = newChar(psB, "90D9A098-9CD8-4A7A-B176-91FFE80909F2", CHAR_READ | CHAR_NOTIFY, val, 1);

// C Service
    BLEService* psC = pServer->createService("5cb68410-6774-11e4-9803-0800200c9a66");
    BLECharacteristic* pcC1 = newCharNoVal(psC, "407eda40-6774-11e4-9803-0800200c9a66", CHAR_WRITE );

// Data Service
    BLEService* psData = pServer->createService(BLEUUID("7bdb8dc0-6c95-11e3-981f-0800200c9a66"), 40);
    val[0] = 0x00;
    pcData1 =                    newChar(psData, "362f71a0-6c96-11e3-981f-0800200c9a66", CHAR_READ | CHAR_NOTIFY, val, 1);
    uint8_t datData2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    BLECharacteristic* pcData2 = newChar(psData, "BD066DA4-F9EC-4F0D-A53C-1CD99147A641", CHAR_READ | CHAR_NOTIFY, datData2, 16);
    val[0] = 0x01;    
    BLECharacteristic* pcData3 = newChar(psData, "85da4f4b-a2ca-4c7c-8c7d-fcd9e2daad56", CHAR_READ | CHAR_NOTIFY, val, 1);
 

    psDevInf->start();
    psBat->start();
    psA->start();
    psB->start();
    psC->start();    
    psData->start(); 
    
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();

    pAdvertising->addServiceUUID(psData->getUUID());

    BLEAdvertisementData oAdvertisementData  = BLEAdvertisementData();
    oAdvertisementData.setName("SKC50S-4 v3.0.1 9E02");
    uint8_t adv_data[] = {0x22, 0x01, 0x01, 0x1b};
    oAdvertisementData.setManufacturerData(std::string((char *)adv_data, 4)); 
    oAdvertisementData.setAppearance(960);    
    pAdvertising->setAdvertisementData(oAdvertisementData);
    //pAdvertising->setScanResponseData(oAdvertisementData);
    
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");

    while (!conns) 
       delay(500);
     
    Serial.println ("Written a value so assuming we are connected");
    delay(5000);

    ti = millis();
    state = 0;
    sm = 1;
}

void loop() {
   if (sm == 1) {
     if (millis() - ti > 4000) {
       ti = millis();
       Serial.print(switches[state]);
       dat[0] = switches[state];
       pcData1->setValue(dat, 1);
       pcData1->notify();

       state++;
       if (state > 3) state = 0;
       sm = 2;
     };
   }  
   else if (sm == 2) {
     if (millis() - ti > 300) {
       ti = millis();
       Serial.println("       OFF");
       dat[0] = 0;
       pcData1->setValue(dat, 1);
       pcData1->notify();
       sm = 1;
     };
   }
}
