#define buzzer 8
#define tali 3
#define helm 2

volatile boolean beeping;
unsigned long lastBuzzerMillis;

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(tali, INPUT_PULLUP);
  pinMode(helm, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tali),change,CHANGE);
  attachInterrupt(digitalPinToInterrupt(helm),change,CHANGE);
}

void loop() { 
  if(beeping) {
    if((millis() - lastBuzzerMillis) > 2000) {
      tone(buzzer,2000);
      delay(500);
      noTone(buzzer);
      delay(500);
      lastBuzzerMillis = millis();
    }
  }
}

void change() {
  if((digitalRead(tali) == LOW) && (digitalRead(helm) == LOW)) {
    beeping = 0;
  }
  else {
    beeping = 1;
  }
}

