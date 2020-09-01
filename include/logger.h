#pragma once
#include <Arduino.h>
#include "sdcard.h"
#include "sensors.h"
#include "myudp.h"
#include "adc.h"

typedef enum reason_t_def
{
	REASON_ALTITUDE,
	REASON_PRESSURE1,
	REASON_PRESSURE2,
	REASON_LORA
} reason_t;

class Logger {
	private:
	String log = "";
	MySD *flash;
	MySensors *sensors;
	MyUDP *udp;
	MyADC *adc;

public:
	void init(MySD *_flash, MySensors *_sensors, MyUDP *_udp, MyADC *_adc);
	void tick(int phase, bool ground, bool inFlight, bool sampling, bool finished, reason_t reason);

	void add(String value);
	void add(float value);
	void add(int value);
	void add(int32_t value);
	void add(reason_t value);
	void save();
	String line();
};