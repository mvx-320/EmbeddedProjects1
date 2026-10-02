#include <LiquidCrystal.h>

LiquidCrystal lcd(37, 36, 35, 34, 33, 32);

// --- Řízení motorů (podle vašeho zapojení) ---
#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  7
#define Motor_R_dir_pin  8
#define Motor_L_pwm_pin  9
#define Motor_R_pwm_pin  10

// --- Piny enkodéru podle prezentace TAMK (Slide 15 - Pravý motor) ---
// Pro pravý motor: ENC_B = D2 (INT4), ENC_A = D23
// Pro levý motor:  ENC_B = D3 (INT5), ENC_A = D24
const int ENC_B_PIN = 4;   // Spouští přerušení (pulse counting)
const int ENC_A_PIN = 3;  // Čte se v přerušení pro směr (direction sensing)

// Čítač pulsů (volatile pro bezpečné čtení z ISR)
// Použijeme znaménkový "long", aby mohl počítat i vzad (Slide 24)
volatile long pulseCount = 0;

// Obsluha přerušení - vyvolaná náběžnou hranou na ENC B
void isrEncoder() {
  // Podle slidů 21 a 22:
  // Když se provede přerušení na B, stav signálu A určuje směr otáčení
  //if (digitalRead(ENC_B_PIN) == HIGH) {
    pulseCount++;
    //Serial.println("xxxxx");
  

    
  //} else {
    //pulseCount--;
  //}
}

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);

  // Nastavení motorů
  pinMode(Motor_L_dir_pin, OUTPUT);
  pinMode(Motor_R_dir_pin, OUTPUT);
  pinMode(Motor_L_pwm_pin, OUTPUT);
  pinMode(Motor_R_pwm_pin, OUTPUT);

  // Nastavení pinů enkodéru (Slide 20 má externí pull-up 3.9k, stačí INPUT)
  pinMode(ENC_B_PIN, INPUT);
  pinMode(ENC_A_PIN, INPUT);

  // Připojení přerušení na pin D2 (INT4) na náběžnou hranu (Slide 15)
  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), isrEncoder, RISING);

  lcd.setCursor(0, 0);
  lcd.print("Ready... wait 2s");
  delay(2000); // Čas na položení vozítka na startovní značku

  // 1. Vynulování pulsů
  pulseCount = 0;

  // 2. Rozjezd dopředu na 4 sekundy
  digitalWrite(Motor_L_dir_pin, Motor_forward);
  digitalWrite(Motor_R_dir_pin, Motor_forward);
  analogWrite(Motor_L_pwm_pin, 150);
  analogWrite(Motor_R_pwm_pin, 150);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Driving 4 seconds...");

  delay(4000); // Jízda přesně 4 sekundy

  // 3. Zastavení motorů
  analogWrite(Motor_L_pwm_pin, 0);
  analogWrite(Motor_R_pwm_pin, 0);

  // 4. Bezpečné načtení hodnoty
  long finalPulses = pulseCount;

  // 5. Zobrazení výsledků (Slide 23)
  Serial.print("Encoder pulses: ");
  Serial.println(finalPulses);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Measurement done!");
  lcd.setCursor(0, 1);
  lcd.print("Pulses: ");
  lcd.print(finalPulses);
}

void loop() {
}
