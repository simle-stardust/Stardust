#pragma once
#include <Arduino.h>
#include "sdcard.h"
#include "sensors.h"
#include "servo.h"
#include "myudp.h"
#include "adc.h"
#include "pwm.h"

typedef enum reason_t_def
{
	REASON_ALTITUDE,
	REASON_PRESSURE1,
	REASON_PRESSURE2,
	REASON_LORA,
	REASON_UDP,
} reason_t;

class Logger {
	private:
	String log = "";
	uint8_t log_udp[204];
	uint16_t log_udp_len;
	MySD *flash;
	MySensors *sensors;
	MyUDP *udp;
	MyADC *adc;
	MyServo *servos;
	MyPWMs *pwms;

public:
	void init(MySD *_flash, MySensors *_sensors, MyUDP *_udp, MyADC *_adc, MyServo *_servos, MyPWMs *_pwms);
	void tick(int phase, bool ground, bool inFlight, bool sampling, bool finished, reason_t reason);

	void add(String value);
	void add(float value);
	void add(int value);
	void add(uint32_t value);
	void add(int32_t value);
	void add(reason_t value);
	void add_udp(uint8_t value);
	void add_udp(uint16_t value);
	void add_udp(uint32_t value);
	void add_udp(int8_t value);
	void add_udp(int16_t value);
	void add_udp(int32_t value);
	void add_udp(float value);
	void save();
	String line();
	void write_to_sd(String value);
};