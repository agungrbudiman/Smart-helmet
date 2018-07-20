#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SparkFun_ADXL345.h>

#define vibrateSensor 2
#define speedSensor 3
#define LedMerah 8
#define LedKuning 7
#define LedBiru 6

#define pings '0'
#define speeds '1'
#define helmsOK '2'
#define helmsNO '3'
#define vibrate '5'

volatile unsigned int rps;
unsigned long prevSendPing, prevSendSpeed, prevSendVibrate, prevCountSpeed, 
  lastKedipMerah, lastKedipKuning, lastKedipBiru;
boolean merahKelip, kuningKelip, biruKelip;
char buff;

RF24 radio(9, 10);
const byte address[][6] = {"Helm", "Motor"};
ADXL345 adxl = ADXL345();

void setup() {
  Serial.begin(9600);
  pinMode(LedMerah, OUTPUT);
  pinMode(LedKuning, OUTPUT);
  pinMode(LedBiru, OUTPUT);
  
  radio.begin();
//  radio.setPALevel(RF24_PA_MIN);
//  radio.setDataRate(RF24_250KBPS); 
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);

  adxl.powerOn();
  adxl.setRangeSetting(16);
//  adxl.setActivityXYZ(1, 0, 0);
//  adxl.setActivityThreshold(64); //4000mg
//  adxl.ActivityINT(1);

  kuningKelip = 1;
  biruKelip = 1;
  
  attachInterrupt(digitalPinToInterrupt(speedSensor),speed_ISR,CHANGE);
}

void loop() {
  if(radio.available()) {
    radio.read(&buff, sizeof(buff));
    Serial.println("r:"+String(buff));
    if(buff == helmsOK) {
      digitalWrite(LedKuning, LOW);
      kuningKelip = 0;
    }
    else if(buff == helmsNO){
      kuningKelip = 1;
    }
  }  
  /*bagian deteksi kecepatan*/
  if((millis() - prevCountSpeed) > 1000) {
    double kmh = 5.63*rps;
    if(kmh > 60.0) {
      merahKelip = 1;
      sendToHelm(500,5000,&prevSendSpeed,speeds);
    }
    else {
      merahKelip = 0;
    }
    rps = 0;
  }
  /*bagian deteksi guncangan*/
  int rx,ry,rz;
  double ax,ay,az;
  adxl.readAccel(&rx, &ry, &rz);
  ax = rx*15.6;
  ay = ry*15.6;
  az = rz*15.6;
  if((ax > 4000) || (ay > 4000) || (az > 4000)) {
    sendToHelm(500,2500,&prevSendVibrate,vibrate);
    for(int i=0; i<10; i++) {
      digitalWrite(LedMerah, !digitalRead(LedMerah));
      delay(100);
    }
  }
  
  if(merahKelip) {  
    if((millis() - lastKedipMerah) > 1000) {
      digitalWrite(LedMerah, !digitalRead(LedMerah));
      lastKedipMerah = millis();
    }
  }
  if(kuningKelip) {
    if((millis() - lastKedipKuning) > 1000) {
      digitalWrite(LedKuning, !digitalRead(LedKuning));
      lastKedipKuning = millis();
    }
  }
  if(biruKelip) {
    if((millis() - lastKedipBiru) > 1000) {
      digitalWrite(LedBiru, !digitalRead(LedBiru));
      lastKedipBiru = millis();
    }
  }  
  sendToHelm(250,10000,&prevSendPing,pings);
}

void speed_ISR() {
  rps++;
}

void sendToHelm(int timeout, int jeda, long *prev, char buff) {
  if((millis() - *prev) > jeda) {
    Serial.println("s:"+String(buff));
    radio.stopListening();
    radio.writeFast(&buff, sizeof(buff));
    boolean ok = radio.txStandBy(timeout);
    *prev = millis();
    prevSendPing = millis();
    if(ok) {
      biruKelip = 0;
      digitalWrite(LedBiru, HIGH);
    }
    else {
      biruKelip = 1;
    }
    radio.startListening();
  }
}



