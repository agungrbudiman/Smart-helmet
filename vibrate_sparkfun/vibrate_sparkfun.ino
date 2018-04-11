/*  ********************************************* 
 *  SparkFun_ADXL345_Example
 *  Triple Axis Accelerometer Breakout - ADXL345 
 *  Hook Up Guide Example 
 *  
 *  Utilizing Sparkfun's ADXL345 Library
 *  Bildr ADXL345 source file modified to support 
 *  both I2C and SPI Communication
 *  
 *  E.Robert @ SparkFun Electronics
 *  Created: Jul 13, 2016
 *  Updated: Sep 06, 2016
 *  
 *  Development Environment Specifics:
 *  Arduino 1.6.11
 *  
 *  Hardware Specifications:
 *  SparkFun ADXL345
 *  Arduino Uno
 *  *********************************************/

#include <SparkFun_ADXL345.h>         // SparkFun ADXL345 Library

/*********** COMMUNICATION SELECTION ***********/
/*    Comment Out The One You Are Not Using    */
//ADXL345 adxl = ADXL345(10);           // USE FOR SPI COMMUNICATION, ADXL345(CS_PIN);
ADXL345 adxl = ADXL345();             // USE FOR I2C COMMUNICATION

int interruptPin = 2;                 // Setup pin 2 to be the interrupt pin (for most Arduino Boards)
volatile boolean singletap;
byte interrupts;


/******************** SETUP ********************/
/*          Configure ADXL345 Settings         */
void setup(){
  
  Serial.begin(9600);                 // Start the serial terminal
  Serial.println("SparkFun ADXL345 Accelerometer Hook Up Guide Example");
  Serial.println();
  
  adxl.powerOn();                     // Power on the ADXL345

  adxl.setRangeSetting(16);           // Give the range settings
                                      // Accepted values are 2g, 4g, 8g or 16g
                                      // Higher Values = Wider Measurement Range
                                      // Lower Values = Greater Sensitivity

  adxl.setTapDetectionOnXYZ(0, 0, 1); // Detect taps in the directions turned ON "adxl.setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
 
  // Set values for what is considered a TAP and what is a DOUBLE TAP (0-255)
  adxl.setTapThreshold(50);           // 62.5 mg per increment
  adxl.setTapDuration(15);            // 625 μs per increment
  
  // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
  adxl.singleTapINT(1);
  
attachInterrupt(digitalPinToInterrupt(interruptPin), interrupt0, RISING);   // Attach Interrupt

}

/****************** MAIN CODE ******************/
/*     Accelerometer Readings and Interrupt    */
void loop(){
  
  // Accelerometer Readings
//  int x,y,z;   
//  adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z

  // Output Results to Serial
  /* UNCOMMENT TO VIEW X Y Z ACCELEROMETER VALUES */  
  //Serial.print(x);
  //Serial.print(", ");
  //Serial.print(y);
  //Serial.print(", ");
  //Serial.println(z); 

  adxl.getInterruptSource();
  if(singletap) {
    delay(500);
    Serial.println("*** SINGLE TAP ***");
    singletap = false;
  }
}

/********************* ISR *********************/
/* Look for Interrupts and Triggered Action    */
void interrupt0() { 
//  if(adxl.triggered(interrupts, ADXL345_SINGLE_TAP)){
    singletap = true;
//  }
}

