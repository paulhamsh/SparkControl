# SparkControl
## Emulate the Spark Control pedal   

Use the latest version - SparkControlSpoofer6

Runs on an ESP32 - tested on Heltec WIFI, M5Core, M5Core 2, generic ESP32.   

Example build is running on a Spark Box - see picture below.   

Will appear to the Spark app as the Spark Control pedal.  You can use an existing ESP32 pedal (like Spark Box) or create your own, or use something like an M5Core which has switches on the case (not foot pedal worthy, but good for a demonstration).    

Works with ios and android - although you need some compilation changes and tricks.   

This can be run in 'BLUEDROID' mode or 'NimBLE' mode.   

There are #defines for Heltec, M5Core, M5Core2 - if nothing is defined it will compile for a generic ESP32.

You can set #BLUEDROID to force bluedroid BLE stack, or leave it undefined for the NimBLE stack.   

If you are using a pedal like SparkBox, then there is a ```#define ACTIVE_HIGH``` to set if your switches are active high (a switch press connects +ve to the GPIO input). Mine are active low (ground at GPIO input on switch press) so I have this commented out.   

And in the SparkControl.ino file you can change which GPIOs are your switches.  Note this is in the order ```1, 3, 2, 4```.
```
// pins                 1  3  2  4
uint8_t SCswitchPins[]{33,27,14,26}; 
```

For IOS - compile without #BLUEDROID. This forces NimBLE.    
For Android - compile either way, both work.    

## If using NimBLE:    
Comment out ```#define BLUEDROID ```  
Ensure ```#define FULL_SERVICES``` is set    

Open the Spark app, connect to the Spark and then select the + Spark Control button.    
It should find the ESP32 and see it as the Spark Control.    

Also, in your Arduino director search for the NimBLE package and edit NimBLEDevice.cpp.   

Change:   
``` uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_PUBLIC; ```   
to:   
``` uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_RANDOM; ```   
   
## If using BLUEDROID:    
Ensure ```#define BLUEDROID``` is set    
Comment out  ```#define FULL_SERVICES```   

This won't work with IOS.    
Open the Spark app, connect to the Spark and then select the + Spark Control button.    
It should find the ESP32 and see it as the Spark Control.     



<p align="center">
  <img src="https://github.com/paulhamsh/SparkControl/blob/main/SparkBoxSparkControl.jpg" width="700" title="connections">
</p>


-------------------------------------------
## Comments on the limitations in NimBLE and Bluedroid with device name and number of services   

Bluedroid only works with 7 services, so needs to have ```#define FULL_SERVICES``` commented out, and ```#define BLUEDROID``` set  
This will set the advertised device name to something the Spark app recognises, and advertises enough services for Android to connect.   

Sadly, ios required more services to be advertised, which bluedroid seems to not allow.   
So then we can use NimBLE, which can advertise all the services.   
But - NimBLE doesn't set the advertising name correctly for the app - for Lightblue on ios only, seems to work for other os and apps.   

And there is also something strange that requires LightBlue to scan the services first, then the app will find the "Spark Control".   

So, we need to compile bluedroid to set the advertising name, then with NimBLE to allow all services, then scan with LightBlue, then connect from the app.   

Hmmm.....    

I've raised three issues for this:     

https://github.com/h2zero/NimBLE-Arduino/issues/400   

https://github.com/espressif/arduino-esp32/issues/6739

https://github.com/espressif/arduino-esp32/issues/6692

The limit of 8 is set here:

esp-idf-master\components\bt\host\bluedroid\common\include\common\bluedroid_user_config.h

```
#ifdef CONFIG_BT_GATT_MAX_SR_PROFILES
#define UC_CONFIG_BT_GATT_MAX_SR_PROFILES       CONFIG_BT_GATT_MAX_SR_PROFILES
#else
#define UC_CONFIG_BT_GATT_MAX_SR_PROFILES       8
#endif
```
