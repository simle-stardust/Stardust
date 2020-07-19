#pragma once
#include <Arduino.h>
#include "sdcard.h"
#include "sensors.h"

#define SAMPLING_TIME 1000 // Czas próbkowania


class Logger
{
private:
	unsigned long lastOperation = 0;
	String log = "";
	MySD *flash;
	MySensors *sensors;

public:
	void init(MySD *_flash, MySensors *_sensors);
	void tick(int phase, bool ground, bool inFlight, bool sampling, bool finished);

	void add(String value);
	void add(float value);
	void add(int value);
	void add(int32_t value);
	void save();
	String line();
};