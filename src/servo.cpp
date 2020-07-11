#include "servo.h"


MyServo::MyServo(unsigned int number) {
	pwm = new Adafruit_PWMServoDriver(ADDRESS);
	servoNumber = number;
	lastOperation = millis();
};

void MyServo::init() {

	pinMode(SERVO_DC, OUTPUT);
	digitalWrite(SERVO_DC, LOW);

	pwm->begin();

	pwm->setOscillatorFrequency(27000000);
	pwm->setPWMFreq(SERVO_FREQ); 
}

void MyServo::reset() {
	for (uint8_t servo = 0; servo < servoNumber; servo++) {
		pwm->setPin(servo, 0);
	}
}

void MyServo::setOpen(uint8_t servo) {
	servos[servo-1].desired = 1;
	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Open");
}

void MyServo::setClosed(uint8_t servo) {
	servos[servo-1].desired = 0;
	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Closed");
}

bool MyServo::getStatus(uint8_t servo) {
	return servos[servo-1].status;
}

void MyServo::tick() {		
	if(servo_pointer > servoNumber-1) servo_pointer = 0;
	if(millis() - lastOperation > TIME_DEADZONE) {
		reset();
		digitalWrite(SERVO_DC, LOW);
		if(servos[servo_pointer].desired != servos[servo_pointer].status) {
			if(servos[servo_pointer].desired == 1) open(servo_pointer + 1);
			if(servos[servo_pointer].desired == 0) close(servo_pointer + 1);
			servo_pointer++;
		} else {
			servo_pointer++;
			tick();
		}
	}
}

bool MyServo::ready() {
	for(uint8_t i = 0; i < servoNumber; ++i) {
		if(servos[i].desired != servos[i].status) return 0;
	}
	return 1;
}

void MyServo::open(uint8_t servo) {
	digitalWrite(SERVO_DC, HIGH);
	delay(10);
	pwm->setPWM(servo, 0, SERVOMAX);
	servos[servo-1].status = 1;
	lastOperation = millis();
	
	Serial.print("Opened ");
	Serial.println(servo);
}

void MyServo::close(uint8_t servo) {
	digitalWrite(SERVO_DC, HIGH);
	delay(10);
	pwm->setPWM(servo, 0, SERVOMIN);
	servos[servo-1].status = 0;
	lastOperation = millis();

	Serial.print("Closed ");
	Serial.println(servo);
}
