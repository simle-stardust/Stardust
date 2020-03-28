#pragma once
#include <SimpleDHT.h>

class MyDHT : public SimpleDHT22 {
	private:
	float temperature;
  	float humidity;
	
	void readSensor();

	public:
	MyDHT(int pin) : SimpleDHT22(pin) {}

	float getTemperature();

	float getHumidity();
};