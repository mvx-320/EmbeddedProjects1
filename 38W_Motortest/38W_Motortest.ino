#include <LiquidCrystal.h>

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

const int VRX = A8;
const int VRY = A9;
const int interruptPin = 19;

#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  7
#define Motor_R_dir_pin  8
#define Motor_L_pwm_pin  9
#define Motor_R_pwm_pin  10

volatile bool buttonPressedFlag = false;
unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);

  pinMode(interruptPin, INPUT_PULLUP);

  pinMode(Motor_L_dir_pin, OUTPUT);
  pinMode(Motor_R_dir_pin, OUTPUT);
  pinMode(Motor_L_pwm_pin, OUTPUT);
  pinMode(Motor_R_pwm_pin, OUTPUT);

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
      lastButtonPress = millis();
      lcd.clear();
      motorTest();
    }
  }

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

  delay(500);
}

void buttonPressed() {
  buttonPressedFlag = true;
}

void motorTest() {
  digitalWrite(Motor_R_dir_pin, Motor_return);
  digitalWrite(Motor_L_dir_pin, Motor_return);

  for (int pwm = 150; pwm > 50; pwm--) {
    analogWrite(Motor_L_pwm_pin, pwm);
    analogWrite(Motor_R_pwm_pin, pwm);
    delay(50);
  }

  digitalWrite(Motor_R_dir_pin, Motor_forward);
  digitalWrite(Motor_L_dir_pin, Motor_forward);

  for (int pwm = 50; pwm < 150; pwm++) {
    analogWrite(Motor_L_pwm_pin, pwm);
    analogWrite(Motor_R_pwm_pin, pwm);
    delay(50);
  }

  analogWrite(Motor_L_pwm_pin, 0);
  analogWrite(Motor_R_pwm_pin, 0);
}
