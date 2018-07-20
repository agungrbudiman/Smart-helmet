#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define buzzer 8
#define tali 3
#define helm 2

#define pings '0'
#define speeds '1'
#define helmsOK '2'
#define helmsNO '3'
#define vibrate '5'

boolean prevHelmState, prevTaliState, changeTrig, beeping;
char buff;
unsigned long lastBuzzerMillis, lastChangeMillis;

RF24 radio(10, 9);
const byte address[][6] = {"Helm", "Motor"};

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(tali, INPUT_PULLUP);
  pinMode(helm, INPUT_PULLUP);

  radio.begin();
//  radio.setPALevel(RF24_PA_MIN);
//  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.startListening();
}

void loop() {
  if(radio.available()) {
    radio.read(&buff, sizeof(buff));
    Serial.println("r:"+String(buff));
    if(!beeping) {
      if(buff == speeds){
        bipbip(1,500);
      }
      else if(buff == vibrate) {
        bipbip(4,100);
        bipbip(1,500);
      }
    }
    if(buff == pings) {
      changeTrig = 1;
    }
  }
  /*bagian deteksi tali dan helm*/
  boolean currentHelmState = digitalRead(helm);
  boolean currentTaliState = digitalRead(tali);
  if((prevTaliState != currentTaliState) || (prevHelmState != currentHelmState)) {
    lastChangeMillis = millis();
    changeTrig = !changeTrig;
    prevTaliState = currentTaliState;
    prevHelmState = currentHelmState;
  }
  if(((millis() - lastChangeMillis) > 1000) && changeTrig) {
    if((currentHelmState == 0) && (currentTaliState == 0)) {
      beeping = false;
      sendToMotor(500,helmsOK);
    }
    else {
      beeping = true;
      sendToMotor(500,helmsNO);
    }
    changeTrig = 0;
  }
  if(beeping) {
    if((millis() - lastBuzzerMillis) > 1000) {
      bipbip(1,100);
      lastBuzzerMillis = millis();
    }
  }
}

void sendToMotor(int timeout, char buff) {
  Serial.println("s:"+String(buff));
  radio.stopListening();
  radio.writeFast(&buff, sizeof(buff));
  boolean ok = radio.txStandBy(timeout);
//  if(ok) {
//    Serial.println("-ok");
//  }
//  else {
//    Serial.println("-fail");
//  }
  radio.startListening();
}

void bipbip(byte jumlah, int durasi) {
  for (byte i=0; i < jumlah; i++) {
    digitalWrite(buzzer,HIGH);
    delay(durasi);
    digitalWrite(buzzer,LOW);
    delay(durasi);
  }
}

