unsigned long prevMillis = millis();
volatile unsigned int rps = 0;
double kmh;
unsigned int rpm;

void setup() {
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2),change,CHANGE);
  Serial.println("ok...");
}

void loop() {
  if((millis() - prevMillis) > 1000) {
    rpm = rps*60;
    kmh = 4.025*rps;
    Serial.println(String(rpm)+" | "+String(kmh,2));
    prevMillis = millis();
    rps = 0;
  }
  digitalWrite(LED_BUILTIN, !digitalRead(2));
}

void change() {
  rps++;
}

