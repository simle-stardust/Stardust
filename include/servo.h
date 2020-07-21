#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN  300 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60
#define SERVO_SAMPLING_TIME 2000

#define ADDRESS 0x40

#define SERVO_DC 47

struct servo_status {
	bool status = 0;
	bool desired = 0;
};

class MyServo {
private:
	Adafruit_PWMServoDriver *pwm;

	unsigned int servoNumber = 0;
	struct servo_status servos[16];

	uint8_t servo_pointer = 0;

	unsigned long lastOperation = 0;
	
	void move(uint8_t servo);

public:
	MyServo(unsigned int number);
	void init();
	void tick();
	void reset();
	void setOpen(uint8_t servo);
	void setClosed(uint8_t servo);
	bool getStatus(uint8_t servo);
	bool ready();
	void openSequence();
	void closeSequence();
};