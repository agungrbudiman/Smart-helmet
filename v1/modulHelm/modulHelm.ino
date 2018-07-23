#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define buzzer 8
#define tali 2
#define helm 3

#define pings '0'
#define speeds '1'
#define helmsOK '2'
#define helmsNO '3'
#define ping1st '4'

boolean currentHelmState, currentTaliState, beeping;
volatile boolean sendNotif;
char buff;
unsigned long lastBuzzerMillis;

RF24 radio(10, 9);
const byte address[][6] = {"Helm", "Motor"};

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(tali, INPUT_PULLUP);
  pinMode(helm, INPUT_PULLUP);

  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.startListening();

  attachInterrupt(digitalPinToInterrupt(tali),interrupt,CHANGE);
  attachInterrupt(digitalPinToInterrupt(helm),interrupt,CHANGE);

  bipbip(2,500);Serial.println("Menunggu koneksi motor...");
}

void loop() {
  if(radio.available()) {
    radio.read(&buff, sizeof(buff));
    Serial.println("received:"+String(buff));
    if(buff == speeds){
      tone(buzzer,2000,500);
    }
    else if(buff == ping1st) {
      Serial.println("Ping diterima!");
      bipbip(2,100);
      sendNotif = 1;
    }
  }
  if(sendNotif == 1) {
    delay(500);
    currentHelmState = digitalRead(helm);
    currentTaliState = digitalRead(tali);
    if((currentHelmState == 0) && (currentTaliState == 0)) {
      bipbip(3,100);
      beeping = false;
      sendToMotor(500,helmsOK);
    }
    else {
      beeping = true;
      sendToMotor(500,helmsNO);
    }
    sendNotif = 0;
  }
  if(beeping) {
    if((millis() - lastBuzzerMillis) > 1000) {
      tone(buzzer,2000,100);
      lastBuzzerMillis = millis();
    }
  }
}

void sendToMotor(int timeout, char buff) {
  radio.stopListening();
  radio.writeFast(&buff, sizeof(buff));
  boolean ok = radio.txStandBy(timeout);
  if(!ok) {
     Serial.println("Not sent!");
  }
  else {
    Serial.println("sent:"+String(buff));
  }
  radio.startListening();
}

void bipbip(byte jumlah, int durasi) {
  for (byte i=0; i < jumlah; i++) {
    tone(buzzer, 2000, durasi);
    delay(durasi*2);
  }
}

void interrupt() {
  sendNotif = 1;
}

