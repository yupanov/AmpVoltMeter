#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <GyverNTC.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // SDA -> A4   SCL -> A5
GyverNTC roomTherm(A2, 10000, 3950, 25, 9790);  // pin, R thermostor, B thermistor, base temp, R resistor
GyverNTC evapTherm(A3, 10000, 3950, 25, 9830);  // GND --- thermistor --- An --- 10к --- 5V

// constants:
const uint8_t mAmpStep = 66;
const float divResOne = 219.0; // kOhm
const float divResTwo = 32.0; // kOhm
const float divRatio =  divResTwo / (divResOne + divResTwo);
const float vRef = 5.0;
const float compStopTemp = -5.0;
const float compStartTemp = 6.0;

File file;

// pins:
const uint8_t currentPin = A0;
const uint8_t voltagePin = A1;
const uint8_t relayPin = 6;

float current = 0.0;
float voltage = 0.0;
float roomTemp = 0.0;
float evapTemp = 0.0;

bool lowBatt = false;
bool isFridgeOn = false;

void setup() {
  pinMode(currentPin, INPUT);
  pinMode(voltagePin, INPUT);
  pinMode(relayPin, OUTPUT);

  Serial.begin(9600);
  while (!Serial);

  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) Serial.println("initialization failed!");
  else Serial.println("initialization done.");
  file = SD.open("voltamp.txt", FILE_WRITE);
    if (file) {
      file.print("counter"); file.print(",");
      file.print("current"); file.print(",");
      file.print("voltage"); file.print(",");
      file.print("roomTemp"); file.print(",");
      file.print("evapTemp");  file.print(",");
      file.println("isFridgeOn"); 
      file.close();
      Serial.println("done printing headers to file.");
    } else {
      Serial.println("error opening voltamp.txt");
    }
  lcd.init();
  lcd.backlight();
}

void loop() {
  static uint32_t timer = millis();
  if (millis() - timer >= 1000) {
    timer = millis();

    getMeasures();
    fridgeRelay();

    printToSdFile();
    serialPrintAll();
    lcdPrintAll();
  }
}

void getMeasures() {
  current = (average(currentPin) - 511) * 0.001 * mAmpStep; // Amps
    if (current < 0.0) current = 0.0;
    voltage = average(voltagePin) / divRatio * vRef / 1024.0; // Volts
    lowBatt = voltage < 11.0;
    roomTemp = roomTherm.getTempAverage();
    evapTemp = evapTherm.getTempAverage();
}

void fridgeRelay() {
  if (!lowBatt) {
    if (evapTemp <= compStopTemp) isFridgeOn = false;
    if (evapTemp >= compStartTemp) isFridgeOn = true;
  } else isFridgeOn = false;
  digitalWrite(relayPin, isFridgeOn);
}

void printToSdFile() {
  static uint32_t timer = millis();  
  static uint16_t counter = 0;
    
  if(millis() - timer >= 60000) { // once per minute
    timer = millis();
    counter++;

    file = SD.open("solarFridgeStats.txt", FILE_WRITE);
    if (file) {
      file.print(counter); file.print(",");
      file.print(current, 2); file.print(",");
      file.print(voltage, 2); file.print(",");
      file.print(roomTemp, 2); file.print(",");
      file.print(evapTemp, 2);  file.print(",");
      file.println(isFridgeOn ? 1 : 0); 

      file.close();
      Serial.println("done printing to file.");
    } else {
      Serial.println("error opening voltamp.txt");
    }
  }  
}

void serialPrintAll() {
  Serial.print("Current = "); Serial.print(current, 2); Serial.println(" A   ");
  Serial.print("Voltage = "); Serial.print(voltage, 2); Serial.println(" V   ");
  Serial.print("Room temperature = "); Serial.print(roomTemp, 2); Serial.println(" *C   ");
  Serial.print("Evaporator temperature = "); Serial.print(evapTemp, 2); Serial.println(" *C   ");
  Serial.println(isFridgeOn ? "Fridge on" : "Fridge off");
  Serial.println();
}

void lcdPrintAll() {
  lcd.setCursor(0, 0);
  lcd.print("Current: ");  
  lcd.setCursor(11, 0);
  lcd.print("         "); // clear previous
  lcd.setCursor(11, 0);
  lcd.print(current, 2);
  lcd.setCursor(19, 0);
  lcd.print("A");

  lcd.setCursor(0, 1);
  lcd.print("Voltage: ");
  lcd.setCursor(11, 1);
  lcd.print("         "); // clear previous
  lcd.setCursor(11, 1);
  lcd.print(voltage, 2);
  lcd.setCursor(19, 1);
  lcd.print("V");

  lcd.setCursor(0, 2);
  lcd.print("Room temp: ");
  lcd.setCursor(11, 2);
  lcd.print("         "); // clear previous
  lcd.setCursor(11, 2);
  lcd.print(roomTemp, 2);
  lcd.setCursor(19, 2);
  lcd.print("C");

  lcd.setCursor(0, 3);
  lcd.print("Evap temp: ");
  lcd.setCursor(11, 3);
  lcd.print("         "); // clear previous
  lcd.setCursor(11, 3);
  lcd.print(evapTemp, 2);
  lcd.setCursor(19, 3);
  lcd.print("C");
}

uint16_t average(uint8_t pin) {
  uint8_t times = 50;
  uint16_t acc = 0;
  for (int i = 0; i < times; i++) {
    acc += analogRead(pin);
    delay(1);
  }
  return acc / times;
}
