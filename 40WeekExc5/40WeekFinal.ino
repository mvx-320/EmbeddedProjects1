#include <LiquidCrystal.h>
#include <LIDARLite.h>
#include <Wire.h>

LIDARLite lidarLite;
int cal_cnt = 0;
int distance;

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

int drivingDistance = 30;      // in cm

const int PIN_ENC_LEFT  = 3;
const int PIN_ENC_RIGHT = 18;

volatile unsigned long leftPulses = 0;
volatile unsigned long rightPulses = 0;

void isrLeft()  { leftPulses++; }
void isrRight() { rightPulses++; }

void isrButton() {
  unsigned long now = millis();
  if (now - jsLastButtonPress > 200) {
    jsLastButtonPress = now;
    jsButtonPressed = true;
  }
}

PulsePair runMotors() {
  int startDistance = lidarLite.distance();
  distance = startDistance;
  int targetDistance = startDistance - drivingDistance;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start:   ");
  lcd.print(startDistance);
  lcd.print(" cm     ");
  lcd.setCursor(0,1);
  lcd.print("End:     ");
  lcd.print(targetDistance);
  lcd.print(" cm    ");

  digitalWrite(Motor_L_dir_pin, Motor_forward);
  digitalWrite(Motor_R_dir_pin, Motor_forward);
  analogWrite(Motor_L_pwm_pin, 56);
  analogWrite(Motor_R_pwm_pin, 60);


  while (distance > targetDistance) {
    distance = lidarLite.distance();
    delay(5);

    lcd.setCursor(0,3);
    lcd.print("Current: ");
    lcd.print(distance);
    lcd.print(" cm    ");
  }

  analogWrite(Motor_L_pwm_pin, 0);
  analogWrite(Motor_R_pwm_pin, 0);

  PulsePair result;
  noInterrupts();
  result.l = leftPulses;
  result.r = rightPulses;
  interrupts();
  return result;
}


void setup() {
  lidarLite.begin(0,true);
  lidarLite.configure(0);

  pinMode(Motor_L_pwm_pin, OUTPUT);
  pinMode(Motor_R_pwm_pin, OUTPUT);
  digitalWrite(Motor_L_pwm_pin, LOW);
  digitalWrite(Motor_R_pwm_pin, LOW);
  pinMode(Motor_L_dir_pin, OUTPUT);
  pinMode(Motor_R_dir_pin, OUTPUT);

  Serial.begin(115200);                   

  lcd.begin(20, 4);

  pinMode(PIN_ENC_LEFT, INPUT);
  pinMode(PIN_ENC_RIGHT, INPUT);
  pinMode(PIN_JS_BTN, INPUT_PULLUP);   

  attachInterrupt(digitalPinToInterrupt(PIN_JS_BTN), isrButton, FALLING);


  lcd.setCursor(0, 0);
  lcd.print("Press button");
}

void loop() {
  if (jsButtonPressed) {
    PulsePair pulses = runMotors();
    jsButtonPressed = false;   
  }




}
