#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <pca9555.h>

#define NB_OF_SERVOS  7

#define SERVO_FREQ 60
#define SERVO_RESET_TIME    1000
#define SERVO_SAMPLING_TIME 2000

#define ADDRESS 0x40
#define GPIO_EXP_ADDRESS 0x20

#define SERVO_DC 47

typedef struct 
{
	char name[4];
	uint16_t open_position;
	uint16_t close_position;
	uint8_t PWM_driver_index;
} servo_configuration_t;

struct servo_status {
	bool status = 0;
	bool desired = 0;
	servo_configuration_t config;
};

typedef enum
{
	SERVO_IDLE,
	SERVO_RESETTING,
	SERVO_MOVING,
} ServoState_t;

class MyServo {
private:
	Adafruit_PWMServoDriver *pwm;
	
	pca9555 *gpio_expander;

	unsigned int servoNumber = 0;
	struct servo_status servos[NB_OF_SERVOS];

	uint8_t servo_pointer = 0;

	ServoState_t state = SERVO_IDLE;

	unsigned long lastOperation = 0;

public:
	MyServo();
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