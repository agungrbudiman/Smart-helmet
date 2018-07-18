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
#define ping1st '4'
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

  prevHelmState = !digitalRead(helm);

  radio.begin();
  //radio.setPALevel(RF24_PA_MIN);
  //radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(1);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.startListening();

  bipbip(2,500);
  Serial.println("Menunggu koneksi motor...");
  delay(1000);
}

void loop() {
  if(radio.available()) {
    radio.read(&buff, sizeof(buff));
    Serial.println("received:"+String(buff));
    if(buff == speeds){
      tone(buzzer,3000,1000);
    }
    else if(buff == vibrate) {
      tone(buzzer,5000,1000);
    }
    else if(buff == ping1st) {
      Serial.println("Koneksi helm-motor berhasil!");
      bipbip(2,100);
    }
  }
  boolean currentHelmState = digitalRead(helm);
  boolean currentTaliState = digitalRead(tali);
  if((prevTaliState != currentTaliState) || (prevHelmState != currentHelmState)) {
    lastChangeMillis = millis();
    changeTrig = !changeTrig;
    prevTaliState = currentTaliState;
    prevHelmState = currentHelmState;
  }
  if((millis() - lastChangeMillis) > 1000) {
    if(changeTrig) {
      if((currentHelmState == 0) && (currentTaliState == 0)) {
        beeping = false;
        bipbip(3,100);
        sendToMotor(1000,helmsOK);
      }
      else {
        beeping = true;
        tone(buzzer,1500,250);
        sendToMotor(1000,helmsNO);
      }
      changeTrig = 0;
    }
  }
  if(beeping) {
    if((millis() - lastBuzzerMillis) > 1000) {
      tone(buzzer,2000,250);
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

