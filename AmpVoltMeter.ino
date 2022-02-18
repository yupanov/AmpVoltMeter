const uint8_t mAmpStep = 66;

// pins:
const uint8_t currentPin = 14;

float current = 0.0;

void setup() {
pinMode(currentPin, INPUT);
Serial.begin(9600);
}

void loop() {
current = (analogRead(currentPin) - 512) / 1000 * mAmpStep;
Serial.print("Current = ");
Serial.print(current, 2);
Serial.println(" A");
}
