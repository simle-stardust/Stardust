#include <OneWire.h>
#include <DallasTemperature.h>

class MyDS18B20
{
private:
	int pin;
	int numberOfSensors;
	float temperature;
	DallasTemperature *sensors;
	DeviceAddress thermometers;
	void readSensors();
	void printAddress(DeviceAddress deviceAddress);
	uint8_t findDevices();

public:
	MyDS18B20(int _pin);
	void init();
	float getTemperature();
};
