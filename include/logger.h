#pragma once
#include <Arduino.h>
#include "sdcard.h"

class Logger {
	private:
	String log = "";
	MySD *flash;
	public:

	void init(MySD *_flash) {
		flash = _flash;
		flash->writeLine("Date,Time,RTC_Temp,DHT_Humid,DHT_Temp,DS_Temp,Pressure,Temperature");
	};

	void add(String value) {
		Serial.print(value);
		log += value + ",";
	}

	void add(float value) {
		Serial.print(value);
		log += String(value) + ",";
	}

	void add(int value) {
		Serial.print(value);
		log += String(value) + ",";
	}

	void save() {
		log += "\n";
		flash->writeLine(log);
		log = "";
	}

	String line() {
		return log;
	}
};