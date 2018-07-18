#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define vibrateSensor 2
#define speedSensor 3
#define LedMerah 8
#define LedKuning 7
#define LedBiru 6

#define pings '0'
#define speeds '1'
#define helmsOK '2'
#define helmsNO '3'
#define ping1st '4'
#define vibrate '5'

volatile unsigned int rps;
double kmh;

unsigned long prevSendPing, prevSendSpeed, prevCountSpeed;
char buff;

RF24 radio(9, 10);
const byte address[][6] = {"Helm", "Motor"};

void setup() {
  Serial.begin(9600);
  pinMode(LedMerah, OUTPUT);
  pinMode(LedKuning, OUTPUT);
  pinMode(LedBiru, OUTPUT);
  
  radio.begin();
  //radio.setPALevel(RF24_PA_MIN);
  //radio.setDataRate(RF24_250KBPS); 
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);  

  ledled(3,1000);

  Serial.println("Inisialisasi koneksi ke helm...");
  sendToHelm(30000,0,&prevSendPing,ping1st);
  Serial.println("Koneksi motor-helm berhasil!");

  attachInterrupt(digitalPinToInterrupt(speedSensor),speed_ISR,CHANGE);
  //attachInterrupt(digitalPinToInterrupt(vibrateSensor),vibrate_ISR,RISING);
}

void loop() {
  if(radio.available()) {
    radio.read(&buff, sizeof(buff));
    Serial.println("received:"+String(buff));
    if(buff == helmsOK) {
      digitalWrite(LedKuning, LOW);
    }
    else if(buff == helmsNO){
      digitalWrite(LedKuning, HIGH);
    }
  }  
  /*bagian deteksi kecepatan*/
  if((millis() - prevCountSpeed) > 1000) {
    kmh = 5.63*rps;
    //Serial.println(kmh);
    if(kmh > 60.0) {
      digitalWrite(LedMerah,HIGH);
      Serial.println("Over Speed!");
      sendToHelm(500,2000,&prevSendSpeed,speeds);
    }
    else {
      digitalWrite(LedMerah,LOW);
    }
    rps = 0;
  }
  
  sendToHelm(1000,10000,&prevSendPing,pings); //ping setiap 10 detik
}

void speed_ISR() {
  rps++;
}

void vibrate_ISR() {
  
}

void sendToHelm(int timeout, int jeda, long *prev, char buff) {
  if((millis() - *prev) > jeda) {
    radio.stopListening();
    radio.writeFast(&buff, sizeof(buff));
    boolean ok = radio.txStandBy(timeout);
    *prev = millis();
    digitalWrite(LedBiru, ok);
    if(ok) {
      Serial.println("sent:"+String(buff));
    }
    radio.startListening();
  }
}

void ledled(byte jumlah, int durasi) {
  for (byte i=0; i < jumlah; i++) {
    digitalWrite(LedMerah, HIGH);
    delay(durasi);
    digitalWrite(LedMerah, LOW);
    delay(durasi);
  }
}



