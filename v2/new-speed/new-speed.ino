unsigned long t1, t2, lastCount;
volatile unsigned long period;

void setup() {
  Serial.begin(9600);
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2),interrupt,FALLING);
}

void loop() {
  if((millis() - lastCount)>1000) {
    double rps = 1/(period/1000000.0);
    double kmh = 5.65*rps;
    Serial.println(String(rps)+"-"+String(kmh));
    lastCount = millis();
    period = 0;
  }
}

void interrupt() {
  //noInterrupts();
  t1 = micros();
  period = t1-t2;
  t2 = t1;
  //interrupts();
}

