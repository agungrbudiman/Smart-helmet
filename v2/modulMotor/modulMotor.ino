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
#define vibrate '4'

volatile unsigned long period;
unsigned long prevSendPing, prevSendSpeed, prevSendVibrate, 
  lastKedipMerah, lastKedipKuning, lastKedipBiru, t1, t2, prevCountSpeed;
boolean merahKelip, kuningKelip, biruKelip;
int prx, pry, prz; //previous raw accel
int macc; //maximum raw accel

RF24 radio(9, 10);
const byte address[][6] = {"helmm", "motor"};
ADXL345 adxl = ADXL345();

byte counter;

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
//  if (Serial.available() > 0) {
//    Serial.read();
//    Serial.println(String(macc)+"-guncangan direset");
//    macc = 0;
//    delay(1000);
//  }
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
    digitalWrite(LedBiru, LOW);
    biruKelip = 0;
  }
    
  /*bagian deteksi kecepatan*/
  int kmh;
  if(millis() - prevCountSpeed > 1000) {
   prevCountSpeed = millis();
   if(period > 0) {
     kmh = 5.83*(1/(period/1000000.0)); //beat=5.65 km/1rps ; pcx=5.83 km/1rps
     if(kmh > 60) {
       merahKelip = 1;
       sendToHelm(2500,&prevSendSpeed,speeds);
     }
     else {
       digitalWrite(LedMerah, LOW);
       merahKelip = 0;
     }
   }
   else {
    kmh = 0;
   }
   //Serial.println(kmh);    
   period = 0;
  }
  
  /*bagian deteksi guncangan*/
  int rx,ry,rz; //raw accel
  int sAx,sAy,sAz;
  int maxTemp;
  adxl.readAccel(&rx, &ry, &rz);
  //Serial.println(String(rx)+"-"+String(ry)+"-"+String(rz));
  sAx = abs((prx-rx)); //selisih dengan akselerasi sebelumnya
  sAy = abs((pry-ry));
  sAz = abs((prz-rz));
  //Serial.println(String(sAx)+"-"+String(sAy)+"-"+String(sAz));
  if((sAx > 355) || (sAy > 355) || (sAz > 355)) { //355*31.2 = 11076 mg (31.2 mg/LSB)
    sendToHelm(2500,&prevSendVibrate,vibrate);
    for(int i=0; i<10; i++) { //strobe led merah
      digitalWrite(LedMerah, !digitalRead(LedMerah));
      delay(50);
    }
  }
  maxTemp = max(maxTemp, sAx); //simpan nilai terbesar dari 3 sumbu
  maxTemp = max(maxTemp, sAy);
  maxTemp = max(maxTemp, sAz);
  if(maxTemp > macc) { //pengukuran sekarang lebih besar dari sebelumnya
    //Serial.println("g:"+String(maxTemp));
    macc = maxTemp;
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
  //digitalWrite(LedKuning, !digitalRead(speedSensor));
}

void speed_ISR() {
  noInterrupts();
  t1 = micros();
  period = t1-t2;
  t2 = t1;
  interrupts();
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
      digitalWrite(LedBiru, LOW);
      biruKelip = 0;
      Serial.println("-ok");
    }
    else {
      biruKelip = 1;
      Serial.println("-fail");
    }
  }
}



