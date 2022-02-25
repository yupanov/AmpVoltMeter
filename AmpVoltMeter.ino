#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <GyverNTC.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // SDA -> A4   SCL -> A5
GyverNTC roomTherm(A6, 10000, 3950, 25, 9790);  // pin, R thermostor, B thermistor, base temp, R resistor
GyverNTC evapTherm(A7, 10000, 3950, 25, 9830);  // GND --- thermistor --- An --- 10к --- 5V

// constants:
// Volts:
const float divResOne = 219.0; // kOhm
const float divResTwo = 32.0; // kOhm
const float multRatio = (divResOne + divResTwo) / divResTwo;
const float vRef = 5.0;
const float multVolt = multRatio * vRef / 1024.0;
// Amps:
const uint8_t mVoltPerAmpLoad = 185; // 5A -> 185, 20A -> 100, 30A -> 185 mV per A
const uint8_t mVoltPerAmpCharge = 66;
const float mVoltPerStep = vRef * 1000.0 / 1024.0;
const float multCurrentLoad = mVoltPerStep / mVoltPerAmpLoad;
const float multCurrentCharge = mVoltPerStep / mVoltPerAmpCharge;
// Thermostate:
const float compStopTemp = -7.0;
const float compStartTemp = 6.0;

File file;
const String fileName = "SolarStats.txt";

// pins:
const uint8_t currentLoadPin = A0;
const uint8_t currentChargePin = A1;
const uint8_t voltagePin = A2;
const uint8_t relayPin = 6;

float currentLoad = 0.0;
float currentCharge = 0.0;
float voltage = 0.0;
float roomTemp = 0.0;
float evapTemp = 0.0;

bool lowBatt = false;
bool isFridgeOn = false;

void setup() {
  pinMode(currentLoadPin, INPUT);
  pinMode(currentChargePin, INPUT);
  pinMode(voltagePin, INPUT);
  pinMode(relayPin, OUTPUT);

  Serial.begin(9600);
  // while (!Serial);
  
  printHeaders();
  
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

void printHeaders() {
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) Serial.println("initialization failed!");
    else Serial.println("initialization done.");
    file = SD.open("SolarStats.txt", FILE_WRITE);
      if (file) {
        file.print("currentLoad"); file.print(",");
        file.print("currentCharge"); file.print(",");
        file.print("voltage"); file.print(",");
        file.print("roomTemp"); file.print(",");
        file.print("evapTemp");  file.print(",");
        file.println("isFridgeOn"); 
        file.close();
        Serial.println("done printing headers to file.");
      } else {
        Serial.println("error opening file");
      }
}

void getMeasures() {
  currentLoad = (average(currentLoadPin) - 512) * multCurrentLoad; // Amps
    if (currentLoad < 0.0 || currentLoad > 30.0) currentLoad = 0.0;
    currentCharge = (average(currentChargePin) - 512) * multCurrentLoad; // Amps
    if (currentCharge < 0.0 || currentCharge > 30.0) currentCharge = 0.0;
    voltage = average(voltagePin) * multVolt; // Volts
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
    
  if(millis() - timer >= 60000) { // once per minute
    timer = millis();

    file = SD.open("SolarStats.txt", FILE_WRITE);
    if (file) {
      file.print(currentLoad); file.print(",");
      file.print(currentCharge, 2); file.print(",");
      file.print(voltage, 2); file.print(",");
      file.print(roomTemp, 2); file.print(",");
      file.print(evapTemp, 2);  file.print(",");
      file.println(isFridgeOn ? 1 : 0); 

      file.close();
      Serial.println("done printing to file.");
    } else {
      Serial.println("error opening file");
    }
  }  
}

void serialPrintAll() {
  Serial.print("CurrentLoad = "); Serial.print(currentLoad, 2); Serial.println(" A   ");
  Serial.print("CurrentCharge = "); Serial.print(currentCharge, 2); Serial.println(" A   ");
  Serial.print("Voltage = "); Serial.print(voltage, 2); Serial.println(" V   ");
  Serial.print("Room temperature = "); Serial.print(roomTemp, 2); Serial.println(" *C   ");
  Serial.print("Evaporator temperature = "); Serial.print(evapTemp, 2); Serial.println(" *C   ");
  Serial.println(isFridgeOn ? "Fridge on" : "Fridge off");
  Serial.println();
}

void lcdPrintAll() {
  lcd.setCursor(0, 0);
  lcd.print("Ld: ");  
  lcd.setCursor(4, 0);
  lcd.print("    "); // clear previous
  lcd.setCursor(4, 0);
  lcd.print(currentLoad, 1);
  lcd.setCursor(8, 0);
  lcd.print("A");

  lcd.setCursor(11, 0);
  lcd.print("Cg: ");  
  lcd.setCursor(15, 0);
  lcd.print("    "); // clear previous
  lcd.setCursor(15, 0);
  lcd.print(currentCharge, 1);
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
