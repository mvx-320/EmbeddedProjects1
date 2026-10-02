#define Motor_forward    0
#define Motor_return     1
#define Motor_L_dir_pin  7
#define Motor_R_dir_pin  8
#define Motor_L_pwm_pin  9
#define Motor_R_pwm_pin  10

void setup() {
    pinMode(Motor_L_dir_pin, OUTPUT);
    pinMode(Motor_R_dir_pin, OUTPUT);
    pinMode(Motor_L_pwm_pin, OUTPUT);
    pinMode(Motor_R_pwm_pin, OUTPUT);
}

void loop() {

    digitalWrite(Motor_R_dir_pin, Motor_return);
    digitalWrite(Motor_L_dir_pin, Motor_return);

    for (int pwm = 150; pwm > 50; pwm--) {
        analogWrite(Motor_L_pwm_pin, pwm);
        analogWrite(Motor_R_pwm_pin, pwm);
	delay(50);
    }

    digitalWrite(Motor_R_dir_pin, Motor_forward);
    digitalWrite(Motor_L_dir_pin, Motor_forward);

    for (int pwm = 50; pwm < 150; pwm++) {
        analogWrite(Motor_L_pwm_pin, pwm);
        analogWrite(Motor_R_pwm_pin, pwm);
	delay(50);
    }
}





