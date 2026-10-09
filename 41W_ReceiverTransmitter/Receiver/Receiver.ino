#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

#define NRF_CE 48
#define NRF_CNS 49

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  8
#define Motor_R_dir_pin  7
#define Motor_L_pwm_pin  10
#define Motor_R_pwm_pin  9


RF24 radio(NRF_CE, NRF_CNS); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};


void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);

  radio.begin();
  radio.setChannel(40);
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  Serial.println(radio.isChipConnected() ? "chip ok" : "chip lost");

}

void loop() {
  radio.startListening();
  if ( radio.available()) {
    while (radio.available()) {
      //int angleV = 0;
      //radio.read(&angleV, sizeof(angleV));
      //myServo.write(angleV);
      int joyValues[2];
      radio.read(&joyValues, sizeof(joyValues));
      Serial.print(joyValues[0]);
      Serial.print(", ");
      Serial.println(joyValues[1]);

      float xPercentage = joyValues[0] / 1023.0 * 100;
      float yPercentage = joyValues[1] / 1023.0 * 100;

      // controlling the car with Joystick
      float xNormalized = (joyValues[0] / 1023.0 * 2 - 1);
      float yNormalized = (joyValues[1] / 1023.0 * 2 - 1) * -1;

      float left  = yNormalized + xNormalized;
      float right = yNormalized - xNormalized;
      float maxMag = fmaxf(fabsf(left), fmaxf(fabsf(right), 1.0f));
      left  /= maxMag;
      right /= maxMag;

      if (left > 0){
        digitalWrite(Motor_L_dir_pin, Motor_forward);
        analogWrite(Motor_L_pwm_pin, 255 * left);  
      } else{
        digitalWrite(Motor_L_dir_pin, Motor_return);
        analogWrite(Motor_L_pwm_pin, 255 * -left); 
      }


      if (right > 0){
        digitalWrite(Motor_R_dir_pin, Motor_forward);
        analogWrite(Motor_R_pwm_pin, 255 * right);  
      } else{
        digitalWrite(Motor_R_dir_pin, Motor_return);
        analogWrite(Motor_R_pwm_pin, 255 * -right); 
      }

      lcd.setCursor(0, 0);
      lcd.print("Joystick Axis X:");

      lcd.setCursor(0, 1);
      lcd.print(joyValues[0]);
      lcd.print(" ");
      lcd.print(xPercentage, 0);
      lcd.print(" %   ");

      lcd.setCursor(0, 2);
      lcd.print("Joystick Axis Y:");

      lcd.setCursor(0, 3);
      lcd.print(joyValues[1]);
      lcd.print(" ");
      lcd.print(yPercentage, 0);
      lcd.print(" %   ");
    }
    delay(5);
    radio.stopListening();
  }
  delay(5);
}