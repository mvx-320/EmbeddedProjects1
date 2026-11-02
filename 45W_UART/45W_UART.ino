
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

const int ENC_PIN = 3;
int angleTol = 4;

float getPpcm(int speed) {
  return 49.62 + 182.16 * exp(-0.021354 * speed);
}

volatile unsigned long pulseCount = 0;
void isrEncoder() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(20, 4);
  
  //Encoder
  attachInterrupt(digitalPinToInterrupt(ENC_PIN), isrEncoder, FALLING);
  
  //Compass
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

  Serial.println("Write something to the serial monitor.");
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
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    message.trim();                      
    Serial.print("Message received, content: ");
    Serial.println(message);

    int lcd_pos   = message.indexOf("LCD");
    int dist_pos      = message.indexOf("Dist");
    int degree_pos    = message.indexOf("Degree");
    int colon_pos    = message.indexOf(":");

    if (lcd_pos > -1) {              
      Serial.println("Command = LCD");
      if (colon_pos > -1) {
        String stat = message.substring(colon_pos + 1);
        stat.trim();
        lcd.clear();                     
        lcd.setCursor(0, 0);
        lcd.print(stat);
      }
    }
    else if (dist_pos > -1) {
      Serial.println("Command = Dist");
      if (colon_pos > -1) {
        String stat = message.substring(colon_pos + 1);
        stat.trim();
        int distInCm = stat.toInt();
        int targetSpeed = 255;
        float ppcm = getPpcm(targetSpeed);
        Serial.print("ppcm: ");
        Serial.println(ppcm);
        float distInPulses = abs(distInCm) * ppcm;
        
        long startEncCount = pulseCount;
        long targetEncCount = startEncCount + distInPulses;
        if (distInCm > 0) {
          digitalWrite(Motor_L_dir_pin, Motor_forward);
          digitalWrite(Motor_R_dir_pin, Motor_forward);
        } else {
          digitalWrite(Motor_L_dir_pin, Motor_return);
          digitalWrite(Motor_R_dir_pin, Motor_return);   
        } 
        analogWrite(Motor_L_pwm_pin, targetSpeed);
        analogWrite(Motor_R_pwm_pin, targetSpeed);

        // Print
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Dist:");
        lcd.print(distInCm);
        lcd.setCursor(0,2);
        lcd.print("TarEnc: ");
        lcd.print(targetEncCount);
        while (targetEncCount > pulseCount) {
          lcd.setCursor(0, 1);
          lcd.print("CurEnc: ");
          lcd.print(pulseCount);
          lcd.print("     ");
          delay(3);
        }
        analogWrite(Motor_L_pwm_pin, 0);
        analogWrite(Motor_R_pwm_pin, 0);
      }
    }
    else if (degree_pos > -1) {
      Serial.println("Command = Degree");
      if (colon_pos > -1) {
        String stat = message.substring(colon_pos + 1);
        stat.trim();
        int targetDegree = stat.toInt();
        lcd.clear();
        lcd.setCursor(0, 3);
        lcd.print("TarDeg: ");
        lcd.print(targetDegree);
        int printCounter = 0;
        while (true) {
          float deg = fmod(getCompDegree(), 360.0);
          float diff = targetDegree - deg;

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
              
              break;
          }

          // Print
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

          if (printCounter > 50) {
            printCounter = 0;
            lcd.setCursor(0,0);
            lcd.print("DegDiff: ");
            lcd.print(diff);
            lcd.print("    ");
            lcd.setCursor(0,1);
            lcd.print("Compass: ");
            lcd.print(comp);
            lcd.print("     ");
            lcd.setCursor(0,2);
            lcd.print("CurDeg: ");
            lcd.print(deg);
            lcd.print("    ");
            Serial.println(diff);
          }
          printCounter++;
        }
      }
    } 
    else {
      Serial.println("No command found, try Print:Hi, Print:Hello or LCD:text");
    }
  }
}
