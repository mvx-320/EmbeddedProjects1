
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

// INPUT
int targetDeg = 0;
int angleTol = 4;


int printCounter = 0;

void setup() {  
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(20, 4);

  Wire.beginTransmission(CMPS14_address);
  Wire.write(0x98);
  delay(20);
  Wire.write(0x95);
  delay(20);
  Wire.write(0x99);
  delay(20);
  Wire.write(0b10000001);
  Wire.endTransmission(false);

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
  //getMagnetometerValues();
  float deg = fmod((getCompDegree() - 33.0), 360.0);

  String comp = "";

  if (deg < 0 || deg > 360) comp = "nV";
  else if (deg < 23) comp = "N";
  else if (deg < 68) comp = "NE";
  else if (deg < 113) comp = "E";
  else if (deg < 158) comp = "SE";
  else if (deg < 203) comp = "S";
  else if (deg < 248) comp = "SW";
  else if (deg < 293) comp = "W";
  else if (deg < 338) comp = "NW";
  else comp = "N";



  float diff = targetDeg - deg;



  if (abs(diff) > angleTol) {
    diff = fmod(diff + 540, 360.0) - 180;
    
    if (diff < 0) {
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
  } else {
      analogWrite(Motor_L_pwm_pin, 0);
      analogWrite(Motor_R_pwm_pin, 0);
  }

  if (printCounter > 50) {
    printCounter = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(deg);
    lcd.setCursor(0,1);
    lcd.print(comp);
    lcd.setCursor(0, 2);
    lcd.print(diff);
    Serial.println(diff);
  }
  printCounter++;
//delay(100);
  }

