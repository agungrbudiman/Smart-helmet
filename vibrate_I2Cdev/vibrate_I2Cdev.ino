#include "Wire.h"

#include "I2Cdev.h"
#include "ADXL345.h"

// class default I2C address is 0x53
// specific I2C addresses may be passed as a parameter here
// ALT low = 0x53 (default for SparkFun 6DOF board)
// ALT high = 0x1D
ADXL345 adxl345;

int16_t ax, ay, az;

#define interruptPin 2

unsigned int counter;
volatile boolean singletap;

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();
    
    Serial.begin(9600);

    // initialize device
    Serial.println("Initializing I2C devices...");
    adxl345.initialize();
    adxl345.setTapThreshold(50);
    adxl345.setTapDuration(15);
    adxl345.setInterruptMode(0);
    adxl345.setIntSingleTapPin(0);
    adxl345.setIntSingleTapEnabled(1);

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(adxl345.testConnection() ? "ADXL345 connection successful" : "ADXL345 connection failed");

    attachInterrupt(digitalPinToInterrupt(interruptPin),interrupt0,RISING);
}

void loop() {
  adxl345.getIntSingleTapSource();
  if(singletap) {
    Serial.println(counter);
    delay(500);
    counter++;
    singletap = false;
  }
  //Serial.println(digitalRead(interruptPin));
    // read raw adxl345 measurements from device
//    adxl345.getAcceleration(&ax, &ay, &az);
//
//    // display tab-separated adxl345 x/y/z values
//    Serial.print("adxl345:\t");
//    Serial.print(ax); Serial.print("\t");
//    Serial.print(ay); Serial.print("\t");
//    Serial.println(az);
}

void interrupt0(){
  singletap = true;
}
