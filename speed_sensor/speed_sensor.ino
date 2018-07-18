#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define HallPin 3

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long refreshMillis;
boolean state = 1;
volatile byte rps;

void setup() {
  lcd.begin();
  Serial.begin(9600);
  pinMode(HallPin,INPUT_PULLUP);
  pinMode(12,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(12,HIGH);
  attachInterrupt(digitalPinToInterrupt(HallPin),enter,FALLING);
  lcd.print("Ready!");
  delay(2000);
}

void loop() {
  if((millis() - refreshMillis) > 1000) {
    lcd.clear();
    double kmh = 5.63*rps;
    Serial.println(String(rps)+" | "+String(kmh));
    lcd.print(rps);
    lcd.setCursor(0,1);
    lcd.print(kmh);
    rps = 0;
    refreshMillis = millis();
  }
  digitalWrite(LED_BUILTIN, !digitalRead(HallPin));
}

void enter() {
  rps++;
}

