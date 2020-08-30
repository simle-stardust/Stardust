#pragma once
#include <Arduino.h>
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

// // TODO: Why you no work ? :(
// 
// for (int i = 0; i < temp_main.getNumberOfDevices(); i++)
// {
// 	Serial.print("DS18B20 main [");
// 	Serial.print(i);
// 	Serial.print("]:				");
// 	this->add(temp_main.getTemperature(i));
// 	Serial.println(" *C;");
// }

// 
// for (int i = 0; i < temp_sens.getNumberOfDevices(); i++)
// {
// 	Serial.print("DS18B20 sens [");
// 	Serial.print(i);
// 	Serial.print("]:				");
// 	this->add(temp_sens.getTemperature(i));
// 	Serial.println(" *C;");
// }