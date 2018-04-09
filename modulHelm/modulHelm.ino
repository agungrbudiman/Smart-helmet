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

boolean prevHelmState, prevTaliState, currentHelmState, currentTaliState;
char buff;

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

  bipbip(2,500);Serial.println("Menunggu koneksi motor...");
  while(!radio.available()); //tunggu ping1st dari motor
  delay(100);
  bipbip(2,100);Serial.println("Koneksi berhasil!");
}

void loop() {
  if(radio.available()) {
    radio.read(&buff, sizeof(buff));
    Serial.println("received:"+String(buff));
    if(buff == speeds){
      bipbip(1,500);
    }
    else if(buff == ping1st) {
      prevHelmState = !prevHelmState;
      prevTaliState = !prevTaliState;
    }
  }
  taliHelmCheck();
}

void taliHelmCheck() {
  currentHelmState = digitalRead(helm);
  currentTaliState = digitalRead(tali);
  if((prevHelmState != currentHelmState) || (prevTaliState != currentTaliState)) {
    if((currentHelmState == 0) && (currentTaliState == 0)) {
      sendToMotor(500,helmsOK);
    }
    else {
      sendToMotor(500,helmsNO);
    }
    prevHelmState = currentHelmState;
    prevTaliState = currentTaliState;
    delay(500);
  }
}

void sendToMotor(int timeout, char buff) {
  radio.stopListening();
  radio.writeFast(&buff, sizeof(buff));
  boolean ok = radio.txStandBy(timeout);
  if(!ok) {
     bipbip(2,500);
  }
  else {
    Serial.println("sent:"+String(buff));
  }
  radio.startListening();
}

void bipbip(byte jumlah, int durasi) {
  for (byte i=0; i < jumlah; i++) {
    tone(buzzer, 2000);
    delay(durasi);
    noTone(buzzer);
    delay(durasi);
  }
}

