// Home solar station controller with DC fridge as a load (arduino).

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <GyverNTC.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);            // SDA -> A4   SCL -> A5
GyverNTC roomTherm(A6, 10000, 3950, 25, 9790); // pin, R thermostor, B thermistor, base temp, R resistor
GyverNTC evapTherm(A7, 10000, 3950, 25, 9830); // GND --- thermistor --- An --- 10к --- 5V

// constants:
// Volts:
#define divResOne 219.0 // kOhm
#define divResTwo 32.0  // kOhm
#define vRef 5.0
const float multRatio = (divResOne + divResTwo) / divResTwo;
const float multVolt = multRatio * vRef / 1024.0;
// Amps:
#define mVoltPerAmpLoad 185 // 5A -> 185, 20A -> 100, 30A -> 185 mV per A
#define mVoltPerAmpCharge 66
const float mVoltPerStep = vRef * 1000.0 / 1024.0;
const float multCurrentLoad = mVoltPerStep / mVoltPerAmpLoad;
const float multCurrentCharge = mVoltPerStep / mVoltPerAmpCharge;
// Thermostate:
const float compStopTemp = -16.0;
const float compStartTemp = 6.0;

File file;
const char fileName[] = "stats.csv";

// pins:
const uint8_t currentLoadPin = A0;
const uint8_t currentChargePin = A1;
const uint8_t voltagePin = A2;
const uint8_t relayPin = 6;
const uint8_t sdCardCS = 4;

float currentLoad = 0.0;
float currentCharge = 0.0;
float voltage = 0.0;
float roomTemp = 0.0;
float evapTemp = 0.0;

bool lowBatt = false;
bool isFridgeOn = false;
bool sdOk = false;

void setup()
{
  pinMode(currentLoadPin, INPUT);
  pinMode(currentChargePin, INPUT);
  pinMode(voltagePin, INPUT);
  pinMode(relayPin, OUTPUT);

  Serial.begin(9600);
  // while (!Serial);
  startSD();
  printHeaders();
  lcd.init();
  lcd.backlight();
}

void loop()
{
  static uint32_t timer = millis();
  if (millis() - timer >= 1000)
  {
    timer = millis();

    getMeasures();
    fridgeRelay();

    printToSdFile();
    serialPrintAll();
    lcdPrintAll();
  }
}

void startSD()
{
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(sdCardCS))
    Serial.println(F("initialization failed!"));
  else
    Serial.println(F("initialization done."));
}

void printHeaders()
{
  file = SD.open(fileName, FILE_WRITE);
  if (file)
  {
    sdOk = true;
    file.print(F("currentLoad"));
    file.print(F(","));
    file.print(F("currentCharge"));
    file.print(F(","));
    file.print(F("voltage"));
    file.print(F(","));
    file.print(F("roomTemp"));
    file.print(F(","));
    file.print(F("evapTemp"));
    file.print(F(","));
    file.println(F("isFridgeOn"));
    file.close();

    Serial.println(F("done printing headers to file."));
  }
  else
  {
    sdOk = false;
    Serial.println(F("error opening file"));
  }
}

void getMeasures()
{
  currentLoad = (average(currentLoadPin) - 512) * multCurrentLoad; // Amps
  if (currentLoad < 0.0 || currentLoad > 30.0)
    currentLoad = 0.0;
  currentCharge = (average(currentChargePin) - 512) * multCurrentCharge; // Amps
  if (currentCharge < 0.0 || currentCharge > 30.0)
    currentCharge = 0.0;
  voltage = average(voltagePin) * multVolt; // Volts
  lowBatt = voltage < 11.0;
  roomTemp = roomTherm.getTempAverage();
  evapTemp = evapTherm.getTempAverage();
}

void fridgeRelay()
{
  if (!lowBatt)
  {
    if (evapTemp <= compStopTemp)
      isFridgeOn = false;
    if (evapTemp >= compStartTemp)
      isFridgeOn = true;
  }
  else
    isFridgeOn = false;
  digitalWrite(relayPin, isFridgeOn);
}

void printToSdFile()
{
  static uint32_t timer = millis();

  if (millis() - timer >= 60000)
  { // once per minute
    timer = millis();

    file = SD.open(fileName, FILE_WRITE);
    if (file)
    {
      sdOk = true;
      file.print(currentLoad);
      file.print(F(","));
      file.print(currentCharge, 2);
      file.print(F(","));
      file.print(voltage, 2);
      file.print(F(","));
      file.print(roomTemp, 2);
      file.print(F(","));
      file.print(evapTemp, 2);
      file.print(F(","));
      file.println(isFridgeOn ? 1 : 0);

      file.close();
      Serial.println(F("done printing to file."));
    }
    else
    {
      sdOk = false;
      Serial.println(F("error opening file"));
    }
  }
}

void serialPrintAll()
{
  Serial.print(F("CurrentLoad = "));
  Serial.print(currentLoad, 2);
  Serial.println(F(" A   "));
  Serial.print(F("CurrentCharge = "));
  Serial.print(currentCharge, 2);
  Serial.println(F(" A   "));
  Serial.print(F("Voltage = "));
  Serial.print(voltage, 2);
  Serial.println(F(" V   "));
  Serial.print(F("Room temperature = "));
  Serial.print(roomTemp, 2);
  Serial.println(F(" *C   "));
  Serial.print(F("Evaporator temperature = "));
  Serial.print(evapTemp, 2);
  Serial.println(F(" *C   "));
  Serial.println(isFridgeOn ? F("Fridge on") : F("Fridge off"));
  Serial.println();
}

void lcdPrintAll()
{
  lcd.setCursor(0, 0);
  lcd.print(F("Ld:"));
  lcd.setCursor(4, 0);
  lcd.print(F("    ")); // clear previous
  lcd.setCursor(4, 0);
  lcd.print(currentLoad, 1);
  lcd.setCursor(8, 0);
  lcd.print(F("A"));

  lcd.setCursor(11, 0);
  lcd.print(F("Cg:"));
  lcd.setCursor(15, 0);
  lcd.print(F("    ")); // clear previous
  lcd.setCursor(15, 0);
  lcd.print(currentCharge, 1);
  lcd.setCursor(19, 0);
  lcd.print(F("A"));

  lcd.setCursor(0, 1);
  lcd.print(F("Voltage: "));
  lcd.setCursor(11, 1);
  lcd.print(F("         ")); // clear previous
  lcd.setCursor(11, 1);
  lcd.print(voltage, 1);
  lcd.setCursor(19, 1);
  lcd.print(F("V"));

  lcd.setCursor(0, 2);
  lcd.print(F("Rm:"));
  lcd.setCursor(4, 2);
  lcd.print(F("    ")); // clear previous
  lcd.setCursor(4, 2);
  lcd.print(roomTemp, 1);
  lcd.setCursor(8, 2);
  lcd.print(F("C"));

  lcd.setCursor(11, 2);
  lcd.print(F("Ev:"));
  lcd.setCursor(15, 2);
  lcd.print(F("    ")); // clear previous
  if (evapTemp < 0.0)
    lcd.setCursor(14, 2);
  else
    lcd.setCursor(15, 2);
  lcd.print(evapTemp, 1);
  lcd.setCursor(19, 2);
  lcd.print(F("C"));

  lcd.setCursor(0, 3);
  lcd.print(F("SD:"));
  lcd.setCursor(4, 3);
  lcd.print(F("    ")); // clear previous
  lcd.setCursor(4, 3);
  lcd.print(sdOk ? F("Ok") : F("Fail"));

  lcd.setCursor(11, 3);
  lcd.print(F("Comp:"));
  lcd.setCursor(17, 3);
  lcd.print(F("   ")); // clear previous
  lcd.setCursor(17, 3);
  lcd.print(isFridgeOn ? F("On") : F("Off"));
}

uint16_t average(uint8_t pin)
{
  uint8_t times = 50;
  uint16_t acc = 0;
  for (int i = 0; i < times; i++)
  {
    acc += analogRead(pin);
    delay(1);
  }
  return acc / times;
}
