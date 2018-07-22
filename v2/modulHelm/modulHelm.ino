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

boolean prevHelmState, prevTaliState, changeTrig, pingTrig, beeping;
unsigned long lastBeeping, lastChange;

RF24 radio(10, 9);
const byte address[][6] = {"helmm", "motor"};

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
    char buff;
    radio.read(&buff, sizeof(buff));
    Serial.println("r:"+String(buff));
    if(!beeping) {
      if(buff == speeds){
        bipbip(1,500);
      }
      else if(buff == vibrate) {
        bipbip(2,100);
        bipbip(1,500);
      }
    }
    if(buff == pings) {
      pingTrig = 1;
      delay(50);
    }
  }
  /*bagian deteksi tali dan helm*/
  boolean currentHelmState = digitalRead(helm);
  boolean currentTaliState = digitalRead(tali);
  if((prevTaliState != currentTaliState) || (prevHelmState != currentHelmState)) {
    lastChange = millis();
    changeTrig = !changeTrig;
    prevTaliState = currentTaliState;
    prevHelmState = currentHelmState;
  }
  if(((millis() - lastChange) > 1000) && (changeTrig || pingTrig)) {
    if((currentHelmState == 0) && (currentTaliState == 0)) {
      sendToMotor(helmsOK);
      beeping = false; 
      if(changeTrig) {
        bipbip(2,100);
      }
    }
    else {
      sendToMotor(helmsNO);
      beeping = true;
      if(changeTrig) {
        bipbip(1,500);
      }
    }
    changeTrig = 0;
    pingTrig = 0;
  }
  if(beeping) {
    if((millis() - lastBeeping) > 1000) {
//      bipbip(1,200);
      lastBeeping = millis();
    }
  }
}

void sendToMotor(char buff) {
  radio.stopListening();
  Serial.print("s:"+String(buff));
  boolean ok = radio.write(&buff, sizeof(buff));
  radio.startListening();
  if(ok) {
    Serial.println("-ok");
  }
  else {
    Serial.println("-fail");
  }
}

void bipbip(byte jumlah, int durasi) {
  for (byte i=0; i < jumlah; i++) {
    digitalWrite(buzzer,HIGH);
    delay(durasi);
    digitalWrite(buzzer,LOW);
    delay(durasi);
  }
}

