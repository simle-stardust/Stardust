#pragma once
#include <Arduino.h>
#include "sdcard.h"

class Logger {
	private:
	String log = "";
	MySD *myFlash;
	HardwareSerial *mySerial;
	public:

	void init(MySD *flash, HardwareSerial *serial, String name) {
		mySerial = serial;
		myFlash = flash;
		myFlash->init(name);
	};

	void add(String value) {
		mySerial->print(value);
		log += value + ",";
	}

	void add(float value) {
		mySerial->print(value);
		log += String(value) + ",";
	}

	void add(int value) {
		mySerial->print(value);
		log += String(value) + ",";
	}

	void save() {
		log += "\n";
		mySerial->println(log);
		myFlash->writeLine(log);
		log = "";
	}

};