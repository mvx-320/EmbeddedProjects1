#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
//#include <Servo.h>

#define NRF_CE 48
#define NRF_CNS 49

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);


//#define button 19

RF24 radio(NRF_CE, NRF_CNS); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
//Servo myServo;
//boolean buttonState = 0;

void setup() {
  Serial.begin(9600);
  //pinMode(button, INPUT);
  //myServo.attach(5);
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
      int btn_pressed[2];
      radio.read(&btn_pressed, sizeof(btn_pressed));
      Serial.print(btn_pressed[0]);
      Serial.print(", ");
      Serial.println(btn_pressed[1]);
    }
    delay(5);
    radio.stopListening();
    //buttonState = digitalRead(button);
    //radio.write(&buttonState, sizeof(buttonState));
  }
  delay(5);
}