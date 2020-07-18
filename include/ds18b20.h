#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>

class MyDS18B20
{
private:
	int pin;
	int numberOfDevices;
	float temperature[10];
	DeviceAddress tempDeviceAddress;
	DallasTemperature *sensors;
	void printAddress(DeviceAddress deviceAddress);
	uint8_t findDevices();

public:
	void init(OneWire oneWire);
	void update();
	float readTemperature(DeviceAddress deviceAddress);
	float getTemperature(int i);
	int getNumberOfDevices();
	void printTemperature(DeviceAddress deviceAddress);
};
