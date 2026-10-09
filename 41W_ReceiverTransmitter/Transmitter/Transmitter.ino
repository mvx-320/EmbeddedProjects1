#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const int VRX = A8;
const int VRY = A9;
const int interruptPin = 19;

#define NRF_CE 48
#define NRF_CNS 49

//#define BTN 12

RF24 radio(NRF_CE, NRF_CNS); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
int js_values[2] = {0, 0};
int prev_js_values[2] = {0, 0};

void setup() {
  Serial.begin(9600);
  //pinMode(interruptPin, INPUT_PULLUP);
  radio.begin();
  radio.setChannel(40);
  radio.openWritingPipe(addresses[1]); // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);


  Serial.println(radio.isChipConnected() ? "chip ok" : "chip lost");
  radio.stopListening();

}

void loop() {
  js_values[0] = analogRead(VRX);
  js_values[1] = analogRead(VRY);

  if (prev_js_values[0] != js_values[0] || 
      prev_js_values[1] != js_values[1]) {

    Serial.println();
    Serial.println(js_values[0]);
    Serial.println(js_values[1]);

    bool done = radio.write(&js_values, sizeof(js_values));
    Serial.println(done ? "Message sent" : "Message not sent");

    prev_js_values[0] = js_values[0];
    prev_js_values[1] = js_values[1];
  }

  delay(5);
}