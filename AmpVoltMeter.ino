#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

const uint8_t mAmpStep = 66;
const double divResOne = 995.0; // kOhm
const double divResTwo = 219.0; // kOhm
const double divRatio =  divResTwo / (divResOne + divResTwo); 
const double vRef = 3.94;

// pins:
const uint8_t currentPin = A0;
const uint8_t voltagePin = A1;

float current = 0.0;
float voltage = 0.0;

void setup() {
  pinMode(currentPin, INPUT);  
  pinMode(voltagePin, INPUT);
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
}

void loop() {

  current = (analogRead(currentPin) - 510) / 1000.0 * mAmpStep; // Amps
  voltage = analogRead(voltagePin) / divRatio * vRef / 1024.0; // Volts
  Serial.print("Current = ");
  Serial.print(current, 2);
  Serial.println(" A   ");

  //lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Current = ");
  lcd.print(current, 2);
  lcd.print(" A   ");

  Serial.print("Voltage = ");
  Serial.print(voltage, 2);
  Serial.println(" V   ");

  //lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Voltage = ");
  lcd.print(voltage, 2);
  lcd.print(" V   ");
  delay(500);
}
