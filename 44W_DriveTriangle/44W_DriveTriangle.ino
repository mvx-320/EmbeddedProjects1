
#include <Wire.h>
#include <LiquidCrystal.h>
#include <string.h>

#define CMPS14_address 0x60
#define CompH 0x02 
#define CompL 0x03
#define MagXH 0x06 
#define MagXL 0x07
#define MagYH 0x08
#define MagYL 0x09
#define MagZH 0x0A
#define MagZL 0x0B

#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  7
#define Motor_R_dir_pin  8
#define Motor_L_pwm_pin  9
#define Motor_R_pwm_pin  10

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

const uint8_t magRegs[] = {MagXH, MagXL, MagYH, MagYL, MagZH, MagZL};
uint16_t MagValues[3];

// Encoder
const int ENC_PIN = 3;
// Joystick
const int JOY_BTN = 19;
volatile bool joyBtnPressed = false;
volatile unsigned long joyLastBtnPress = 0;
const int JOY_X = A8;
const int JOY_Y = A9;

// INPUT
int angleTol = 14;

struct Instruction {
  int speed; // If speed is negative the instruction is a rotation
  int distance; // distance (in cm) or angle (While rotating)
} inst_array[5] = {
    { (int)(.75 * 255), 20 },
    { -1, 110 },
    { (int)(.25 * 255), 13},
    { -1, 110 },
    { (int)(.5 * 255), 20}
  };

float getPpcm(int speed) {
  return 49.62 + 182.16 * exp(-0.021354 * speed);
}

volatile unsigned long pulseCount = 0;
void isrEncoder() {
  pulseCount++;
}
void isrButton() {
  unsigned long now = millis();
  if (now - joyLastBtnPress > 200) {
    joyLastBtnPress = now;
    joyBtnPressed = true;
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(20, 4);

  // Joystick
  pinMode(JOY_BTN, INPUT_PULLUP);
  // Encoder
  attachInterrupt(digitalPinToInterrupt(ENC_PIN), isrEncoder, FALLING);

  // Compass
  Wire.beginTransmission(CMPS14_address);
  Wire.write(0x98);
  delay(20);
  Wire.write(0x95);
  delay(20);
  Wire.write(0x99);
  delay(20);
  Wire.write(0b10000001);
  Wire.endTransmission(false);

  // Motors
  pinMode(Motor_L_dir_pin, OUTPUT);
  pinMode(Motor_R_dir_pin, OUTPUT);
  pinMode(Motor_L_pwm_pin, OUTPUT);
  pinMode(Motor_R_pwm_pin, OUTPUT);


}

uint8_t readReg(uint8_t addr) {
  Wire.beginTransmission(CMPS14_address);
  Wire.write(addr);
  Wire.endTransmission(false);
  Wire.requestFrom(CMPS14_address, 1, true);

  if (Wire.available() >= 1 ){
      return Wire.read();
  }
}

float getCompDegree(){
  uint16_t rawValue;

  rawValue = readReg(CompH) << 8;
  rawValue |= readReg(CompL);

  return ((float) rawValue) / 10;
}

void getMagnetometerValues(){
  MagValues[0] = 0;
  MagValues[1] = 0;
  MagValues[2] = 0;

  for(int i=0 ; i < sizeof(magRegs) / sizeof(magRegs[0]) ; i++){
    byte raw = readReg(magRegs[i]);
    Serial.print(raw);
  
    uint16_t value;
    if(i % 2 == 0){
      value = raw << 8;
    } else {
      value = raw;
    }
  
    MagValues[i/2] |= value;  
    Serial.print(" ");
  }

  Serial.println();
}


void loop() {
  if (digitalRead(JOY_BTN)) { 
    //Serial.println("Btn not pressed");
    delay(100); 
    return; 
  }

  for (int i = 0; i < sizeof(inst_array) / sizeof(inst_array[0]); i++) {
    Serial.println(i);
    if (inst_array[i].speed < 0) {
      // Rotation
      float diffAngle = (float) inst_array[i].distance;
      float deg = getCompDegree() * (-1) + 360;
      float targetAngle = fmod(deg + diffAngle, 360);

      while (abs(diffAngle) > angleTol) {
        Serial.print("cur:");
        Serial.print(getCompDegree());
        Serial.print(", tar:");
        Serial.print(targetAngle);
        Serial.print(", diff:");
        Serial.println(diffAngle);
        if (diffAngle > 0) {
          digitalWrite(Motor_L_dir_pin, Motor_forward);
          digitalWrite(Motor_R_dir_pin, Motor_return);
          analogWrite(Motor_L_pwm_pin, 100);
          analogWrite(Motor_R_pwm_pin, 100);
        } else {
          digitalWrite(Motor_L_dir_pin, Motor_return);
          digitalWrite(Motor_R_dir_pin, Motor_forward);
          analogWrite(Motor_L_pwm_pin, 100);
          analogWrite(Motor_R_pwm_pin, 100);
        }
        diffAngle = fmod(targetAngle - (getCompDegree()*(-1)+360) + 360, 360);
        // TODO: May add some delay
      }
      analogWrite(Motor_L_pwm_pin, 0);
      analogWrite(Motor_R_pwm_pin, 0);
    } else {
      // Driving
      int distInCm = inst_array[i].distance;
      int targetSpeed = inst_array[i].speed;
      float ppcm = getPpcm(targetSpeed);
      Serial.print("ppcm: ");
      Serial.println(ppcm);
      float distInPulses = distInCm * ppcm;
      
      long startEncCount = pulseCount;
      long targetEncCount = startEncCount + distInPulses;

      digitalWrite(Motor_L_dir_pin, Motor_forward);
      digitalWrite(Motor_R_dir_pin, Motor_forward);
      analogWrite(Motor_L_pwm_pin, targetSpeed);
      analogWrite(Motor_R_pwm_pin, targetSpeed);
      while (targetEncCount > pulseCount) {
        delay(3);
      }
      analogWrite(Motor_L_pwm_pin, 0);
      analogWrite(Motor_R_pwm_pin, 0);
    }
    delay(100);
  }
}

