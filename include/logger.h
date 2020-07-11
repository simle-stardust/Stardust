#pragma once
#include <Arduino.h>
#include "sdcard.h"

class Logger {
	private:
	String log = "";
	MySD *myFlash;
	public:

	void init(MySD *flash) {
		myFlash = flash;
		myFlash->writeLine("Date,Time,RTC_Temp,DHT_Humid,DHT_Temp,DS_Temp,Pressure,Temperature,Knob");
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
		Serial.println(log);
		myFlash->writeLine(log);
		log = "";
	}

};