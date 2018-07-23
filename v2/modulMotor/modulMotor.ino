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

volatile unsigned long period;
unsigned long prevSendPing, prevSendSpeed, prevSendVibrate, prevCountSpeed, 
  lastKedipMerah, lastKedipKuning, lastKedipBiru, t1, t2;
boolean merahKelip, kuningKelip, biruKelip;
int prx, pry, prz; //previous raw accel

RF24 radio(9, 10);
const byte address[][6] = {"helmm", "motor"};
ADXL345 adxl = ADXL345();

void setup() {
  Serial.begin(9600);
  pinMode(LedMerah, OUTPUT);
  pinMode(LedKuning, OUTPUT);
  pinMode(LedBiru, OUTPUT);
  kuningKelip = 1;
  biruKelip = 1;
  
  radio.begin();
//  radio.setPALevel(RF24_PA_MIN);
//  radio.setDataRate(RF24_250KBPS); 
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.startListening();

  adxl.powerOn();
  adxl.setRangeSetting(16);
//  adxl.setActivityXYZ(1, 0, 0);
//  adxl.setActivityThreshold(64); //4000mg
//  adxl.ActivityINT(1);
  
  attachInterrupt(digitalPinToInterrupt(speedSensor),speed_ISR,FALLING);
}

void loop() {
  if(radio.available()) {
    char buff;
    radio.read(&buff, sizeof(buff));
    prevSendPing = millis();
    Serial.println("r:"+String(buff));
    if(buff == helmsOK) {
      digitalWrite(LedKuning, LOW);
      kuningKelip = 0;
    }
    else if(buff == helmsNO){
      kuningKelip = 1;
    }
    biruKelip = 0;
    digitalWrite(LedBiru, HIGH);
  }  
  /*bagian deteksi kecepatan*/
  if((millis() - prevCountSpeed) > 1000) {
    double kmh = 5.65*(1/(period/1000000.0)); //5.65 * rps
    if(kmh > 50.0) {
      merahKelip = 1;
      sendToHelm(5000,&prevSendSpeed,speeds);
    }
    else {
      digitalWrite(LedMerah, LOW);
      merahKelip = 0;
    }
    period = 0;
  }
  /*bagian deteksi guncangan*/
  int rx,ry,rz; //raw accel
  double sAx,sAy,sAz; //selisih dengan previous
  adxl.readAccel(&rx, &ry, &rz);
  sAx = abs((prx-rx)*15.6); //15.6 mg/LSB
  sAy = abs((pry-ry)*15.6);
  sAz = abs((prz-rz)*15.6);
  if((sAx > 4000) || (sAy > 4000) || (sAz > 4000)) {
    sendToHelm(2000,&prevSendVibrate,vibrate);
    for(int i=0; i<10; i++) {
      digitalWrite(LedMerah, !digitalRead(LedMerah));
      delay(100);
    }
  }
  prx = rx;
  pry = ry;
  prz = rz;
  
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
  sendToHelm(5000,&prevSendPing,pings);
}

void speed_ISR() {
  t1 = micros();
  period = t1-t2;
  t2 = t1;
}

void sendToHelm(int jeda, long *prev, char buff) {
  if((millis() - *prev) > jeda) {
    radio.stopListening();
    Serial.print("s:"+String(buff));
    boolean ok = radio.write(&buff, sizeof(buff));
    radio.startListening();
    *prev = millis();
    prevSendPing = *prev;
    if(ok) {
      digitalWrite(LedBiru, HIGH);
      biruKelip = 0;
      Serial.println("-ok");
    }
    else {
      biruKelip = 1;
      Serial.println("-fail");
    }
  }
}



