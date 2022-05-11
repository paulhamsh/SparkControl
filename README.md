# SparkControl
Emulate the Spark Control pedal

This can be run in 'bluedroid' mode or 'NimBLE' mode   

Bluedroid only works with 7 services, so needs to have #define FULL_SERVICES commented out, and #define CLASSIC set  
This will set the advertised device name to something the Spark app recognises, and advertises enough services for Android to connect.   

Sadly, ios required more services to be advertised, which bluedroid seems to not allow.   
So then we can use NimBLE, which can advertise all the services.   
But - NimBLE doesn't set the advertising name correctly for the app - for Lightblue on ios only, seems to work for other os and apps.   

And there is also something strange that requires LightBlue to scan the services first, then the app will find the "Spark Control".   

So, we need to compile bluedroid to set the advertising name, then with NimBLE to allow all services, then scan with LightBlue, then connect from the app.   

Hmmm.....    

I've raised two issues for this:     

https://github.com/h2zero/NimBLE-Arduino/issues/400    


https://github.com/espressif/arduino-esp32/issues/6692

