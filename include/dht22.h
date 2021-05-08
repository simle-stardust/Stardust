#pragma once
#include <SimpleDHT.h>

class MyDHT : public SimpleDHT22 {
	private:
	float temperature;
  	float humidity;
	int lastErrCode;
	
	void readSensor();

	public:
	MyDHT(int pin) : SimpleDHT22(pin) { lastErrCode = SimpleDHTErrSuccess; }

	float getTemperature();

	float getHumidity();

	int getStatus();
};