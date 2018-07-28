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
#define vibrate '4'

boolean prevHelmState, prevTaliState, changeTrig, pingTrig, beeping;
unsigned long lastBeeping, lastChange;

RF24 radio(10, 9);
const byte address[][6] = {"helmm", "motor"};

int counter;

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

  changeTrig = 1;
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
      pingTrig = 1; //setiap ping diterima dari motor, kirim status helm dan tali
      delay(50);
    }
  }
  /*bagian deteksi tali dan helm*/
  boolean currentHelmState = digitalRead(helm);
  boolean currentTaliState = digitalRead(tali);
  if((prevTaliState != currentTaliState) || (prevHelmState != currentHelmState)) { //state berubah
    lastChange = millis();
    changeTrig = !changeTrig; //mendeteksi perubahan akibat longgar, dieksekusi sebanyak ganjil=berubah; genap=tidak berubah
    prevTaliState = currentTaliState;
    prevHelmState = currentHelmState;
//    if(!stateChange) {
//      Serial.print("ch-");
//      stateChange = 1;
//    }
  }
  if(((millis() - lastChange) > 1000) && (changeTrig || pingTrig)) { //status minimal bertahan 1 detik untuk dianggap berubah, menghindari false warning
    if((currentHelmState == 0) && (currentTaliState == 0)) { //tali dikunci dan helm dikenakan
      sendToMotor(helmsOK);
      beeping = false; 
      if(changeTrig) { //bunyikan buzzer hanya jika status tali benar-benar berubah, bukan karena ping
        bipbip(2,100);
      }
    }
    else {
      sendToMotor(helmsNO);
      beeping = true;
      if(changeTrig) {
        bipbip(1,500); //bunyikan buzzer hanya jika status tali benar-benar berubah, bukan karena ping
      }
    }
    changeTrig = 0;
    pingTrig = 0;
//    stateChange = 0;
  }
  if(beeping) {
    if((millis() - lastBeeping) > 1000) { //bunyikan buzzer setiap 1 detik jika tali belum terkunci dan/atau helm belum digunakan
      bipbip(1,100);
      lastBeeping = millis();
    }
  }
}

void sendToMotor(char buff) {
//  String msg = String(counter++)+"-"+buff; //buat tes response time
//  char pesan[5];
//  msg.toCharArray(pesan,5);
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

