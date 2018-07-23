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

volatile unsigned int rps = 0;
double kmh;

unsigned long prevSendPing, prevSendSpeed, prevSpeed;
char buff;

RF24 radio(9, 10);
const byte address[][6] = {"Helm", "Motor"};

void setup() {
  Serial.begin(9600);
  pinMode(LedMerah, OUTPUT);
  pinMode(LedKuning, OUTPUT);
  pinMode(LedBiru, OUTPUT);
  //pinMode(vibrateSensor, INPUT_PULLUP);
  
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS); 
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);  

  digitalWrite(LedBiru, HIGH);
  Serial.println("Inisialisasi koneksi ke helm...");
  sendToHelm(30000,0,&prevSendPing,ping1st);
  Serial.println("Koneksi berhasil!");

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
  if((millis() - prevSpeed) > 1000) {
    kmh = rps*4.025;
    //Serial.println(kmh);
    if(kmh > 50.0) {
      ledled(1);
      Serial.println("Over Speed!");
      sendToHelm(500,2500,&prevSendSpeed,speeds);
    }
    prevSpeed = millis();
    rps = 0;
  }
  sendToHelm(500,10000,&prevSendPing,pings);
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
    digitalWrite(LedBiru, !ok);
    if(ok) {
      Serial.println("sent:"+String(buff));
    }
    radio.startListening();

  }
}

void ledled(byte jumlah) {
  for (byte i=0; i < jumlah; i++) {
    digitalWrite(LedMerah, HIGH);
    delay(100);
    digitalWrite(LedMerah, LOW);
  }
}



