#include <LiquidCrystal.h>

// Hardware Connections
#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  7
#define Motor_R_dir_pin  8
#define Motor_L_pwm_pin  9
#define Motor_R_pwm_pin  10

#define PIN_JS_BTN 19

#define PIN_ENC_LEFT 3
#define PIN_ENC_RIGHT 18 

// Initializations
struct LongPair {
  unsigned long l;
  unsigned long r;
};

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

// Global variables
volatile bool jsButtonPressed = false;
volatile unsigned long jsLastButtonPress = 0;

float targetDistanceCm = 10.0; //in cm
float pulsesPerCm = 400 / 6.75; 
unsigned long targetPulses = targetDistanceCm * pulsesPerCm;

volatile unsigned long leftPulses = 0;
volatile unsigned long rightPulses = 0;

// Function Prototypes
LongPair runMotors(unsigned long targetPulses);
void writeResultToLcd();

// Function Interrupts
void isrLeft() {leftPulses++;}
void isrRight() {rightPulses++;}


void setup() {
  lcd.begin(20, 4);

  pinMode(Motor_L_dir_pin, OUTPUT);
  pinMode(Motor_R_dir_pin, OUTPUT);
  pinMode(Motor_L_pwm_pin, OUTPUT);
  pinMode(Motor_R_pwm_pin, OUTPUT);

  pinMode(PIN_ENC_LEFT, INPUT);
  pinMode(PIN_ENC_RIGHT, INPUT);


  attachInterrupt(digitalPinToInterrupt(PIN_ENC_LEFT), isrLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_RIGHT), isrRight, RISING);

  lcd.setCursor(0, 0);
  lcd.print("Start 0,5s...");
  delay(500); 
}

void loop() {
  if (jsButtonPressed) {
    jsButtonPressed = false;
    if (millis() - jsLastButtonPress > 200) {
      jsLastButtonPress = millis();

      LongPair pulses = runMotors(targetPulses);
      writeResultToLcd(pulses.l, pulses.r);
    }
  }
}

LongPair runMotors(unsigned long targetPulses) {
  leftPulses = 0;
  rightPulses = 0;

  digitalWrite(Motor_L_dir_pin, Motor_forward);
  digitalWrite(Motor_R_dir_pin, Motor_forward);
  analogWrite(Motor_L_pwm_pin, 143);
  analogWrite(Motor_R_pwm_pin, 150);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("...");

  // delay until targetPulses is reached
  while (leftPulses <= targetPulses) {
    delay(5); 
  }

  // targetpusles reached -> turn of motors!
  analogWrite(Motor_L_pwm_pin, 0);
  analogWrite(Motor_R_pwm_pin, 0);

  LongPair result = {leftPulses, rightPulses};
  return result;
}

void writeResultToLcd(int left, int right) {
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