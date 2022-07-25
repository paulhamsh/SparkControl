#define SPARK_CONTROL

void SparkControlStart();
void InitialiseGPIO();
void SparkControlLoop();

void setup() {
  Serial.begin(115200); 
  delay(2000);
  Serial.println("SparkControlSpooferTest2");
  InitialiseGPIO();
  SparkControlStart();
}

void loop() {
  SparkControlLoop();
}
