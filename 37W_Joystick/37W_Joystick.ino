#include <LiquidCrystal.h>

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

const int VRX = A8;
const int VRY = A9;
const int interruptPin = 19;

volatile bool buttonPressedFlag = false;

bool showCounter = false;
int counter = 0;
unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);

  pinMode(interruptPin, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(interruptPin),
    buttonPressed,
    FALLING
  );
}

void loop() {

  if (buttonPressedFlag) {
    buttonPressedFlag = false;
    if (millis() - lastButtonPress > 200) {
    showCounter = !showCounter;
    counter++;
    lcd.clear();

    lastButtonPress = millis();
    }
  }

  if (showCounter == false) {

    int joyValueX = analogRead(VRX);
    int joyValueY = analogRead(VRY);

    float xPercentage = joyValueX / 1023.0 * 100;
    float yPercentage = joyValueY / 1023.0 * 100;

    lcd.setCursor(0, 0);
    lcd.print("Joystick Axis X:");

    lcd.setCursor(0, 1);
    lcd.print(joyValueX);
    lcd.print(" ");
    lcd.print(xPercentage, 0);
    lcd.print(" %   ");

    lcd.setCursor(0, 2);
    lcd.print("Joystick Axis Y:");

    lcd.setCursor(0, 3);
    lcd.print(joyValueY);
    lcd.print(" ");
    lcd.print(yPercentage, 0);
    lcd.print(" %   ");

  } 
  else {
    lcd.setCursor(0, 0);
    lcd.print("Push counter:");

    lcd.setCursor(0, 1);
    lcd.print(counter);
  }

  delay(500);
}

void buttonPressed() {
  buttonPressedFlag = true;
}
