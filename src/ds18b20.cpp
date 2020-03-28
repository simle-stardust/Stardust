#include "ds18b20.h"

MyDS18B20::MyDS18B20(int _pin)
{
	pin = _pin;
	OneWire oneWire(_pin);
	sensors = new DallasTemperature(&oneWire);
}

void MyDS18B20::init()
{
	Serial.print("Locating devices...");
	sensors->begin();
	Serial.print("Found ");
	numberOfSensors = sensors->getDeviceCount();
	Serial.print(numberOfSensors, DEC);
	Serial.println(" temperature sensors.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (sensors->isParasitePowerMode())
		Serial.println("ON");
	else
		Serial.println("OFF");

	if (!sensors->getAddress(thermometers, 0))
	{
		Serial.println("Unable to find address for Device 0");
	}
	else
	{
		Serial.print("Device 0 Address: ");
		printAddress(thermometers);
		Serial.println();

		Serial.print("Device 0 Resolution: ");
 		Serial.print(sensors->getResolution(thermometers), DEC); 
  		Serial.println();
	};

	sensors->setResolution(thermometers, 9);
}

void MyDS18B20::readSensors()
{
	sensors->requestTemperatures();
	temperature = sensors->getTempC(thermometers);
}

float MyDS18B20::getTemperature() {
	readSensors();
	return temperature;
}

void MyDS18B20::printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}