#include <LiquidCrystal.h>

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  7
#define Motor_R_dir_pin  8
#define Motor_L_pwm_pin  9
#define Motor_R_pwm_pin  10

struct PulsePair {
  unsigned long l;
  unsigned long r;
};

const int PIN_JS_BTN = 19;
volatile bool jsButtonPressed = false;
volatile unsigned long jsLastButtonPress = 0;

float targetDistanceCm = 65.0;      // in cm
float pulsesPerCm = 66.0;//23.5;      // 4000 pulse ≈ 70 cm
unsigned long targetPulses = 0;     // global: runMotors() da kullanıyor

const int PIN_ENC_LEFT  = 3;
const int PIN_ENC_RIGHT = 18;

volatile unsigned long leftPulses = 0;
volatile unsigned long rightPulses = 0;

void isrLeft()  { leftPulses++; }
void isrRight() { rightPulses++; }

// joystick butonu (debounce ISR içinde)
void isrButton() {
  unsigned long now = millis();
  if (now - jsLastButtonPress > 200) {
    jsLastButtonPress = now;
    jsButtonPressed = true;
  }
}

PulsePair runMotors() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("...");


  noInterrupts();
  leftPulses = 0;
  rightPulses = 0;
  interrupts();

  digitalWrite(Motor_L_dir_pin, Motor_forward);
  digitalWrite(Motor_R_dir_pin, Motor_forward);
  analogWrite(Motor_L_pwm_pin, 56);
  analogWrite(Motor_R_pwm_pin, 60);

  unsigned long l = 0;
  while (l < (targetPulses)) {
    noInterrupts();
    l = leftPulses;
    interrupts();
    delay(5);
  }

  //analogWrite(Motor_L_pwm_pin, 45);
  //analogWrite(Motor_R_pwm_pin, 50);

  //while (l < targetPulses) {
   // noInterrupts();
    //l = leftPulses;
    //interrupts();
    //delay(5);
  //}

  // targetPulses reached -> turn off motors!
  analogWrite(Motor_L_pwm_pin, 0);
  analogWrite(Motor_R_pwm_pin, 0);

  PulsePair result;
  noInterrupts();
  result.l = leftPulses;
  result.r = rightPulses;
  interrupts();
  return result;
}

void writeResultToLcd(unsigned long left, unsigned long right) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("finish!");

  lcd.setCursor(0, 1);
  lcd.print("L pulses: ");
  lcd.print(left);

  lcd.setCursor(0, 2);
  lcd.print("R pulses: ");
  lcd.print(right);
}

void setup() {
  // motorları en başta durdur
  pinMode(Motor_L_pwm_pin, OUTPUT);
  pinMode(Motor_R_pwm_pin, OUTPUT);
  digitalWrite(Motor_L_pwm_pin, LOW);
  digitalWrite(Motor_R_pwm_pin, LOW);
  pinMode(Motor_L_dir_pin, OUTPUT);
  pinMode(Motor_R_dir_pin, OUTPUT);

  lcd.begin(20, 4);

  pinMode(PIN_ENC_LEFT, INPUT);
  pinMode(PIN_ENC_RIGHT, INPUT);
  pinMode(PIN_JS_BTN, INPUT_PULLUP);   // buton basılınca LOW olur

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_LEFT), isrLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_RIGHT), isrRight, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_JS_BTN), isrButton, FALLING);

  targetPulses = targetDistanceCm * pulsesPerCm;

  lcd.setCursor(0, 0);
  lcd.print("Press button");
}

void loop() {
  if (jsButtonPressed) {
    PulsePair pulses = runMotors();
    writeResultToLcd(pulses.l, pulses.r);
    jsButtonPressed = false;   // sürüş sırasındaki zıplamaları yok say
  }
}