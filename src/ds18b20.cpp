#include "ds18b20.h"

void MyDS18B20::init(OneWire oneWire)
{

	sensors = new DallasTemperature(&oneWire);
	sensors->begin();

	// Grab a count of devices on the wire
	numberOfDevices = sensors->getDeviceCount();

	// locate devices on the bus
	Serial.print("Locating devices...");

	Serial.print("Found ");
	Serial.print(numberOfDevices, DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (sensors->isParasitePowerMode())
		Serial.println("ON");
	else
		Serial.println("OFF");

	// Loop through each device, print out address
	for (int i = 0; i < numberOfDevices; i++)
	{
		// Search the wire for address
		if (sensors->getAddress(tempDeviceAddress, i))
		{
			Serial.print("Found device ");
			Serial.print(i, DEC);
			Serial.print(" with address: ");
			printAddress(tempDeviceAddress);
			Serial.println();

			Serial.print("Setting resolution to ");
			Serial.println(12, DEC);

			sensors->setResolution(tempDeviceAddress, 12);

			Serial.print("Resolution actually set to: ");
			Serial.print(sensors->getResolution(tempDeviceAddress), DEC);
			Serial.println();
		}
		else
		{
			Serial.print("Found ghost device at ");
			Serial.print(i, DEC);
			Serial.print(" but could not detect address. Check power and cabling");
		}
	}
}
void MyDS18B20::tick()
{
	if (numberOfDevices == 0) return;
	
	sensors->requestTemperatures();

	for (int i = 0; i < numberOfDevices; i++)
	{
		if (sensors->getAddress(tempDeviceAddress, i))
		{
			float tempC = sensors->getTempC(tempDeviceAddress);
			Serial.println(tempC);
			if (tempC == DEVICE_DISCONNECTED_C)
			{
				Serial.println("Error: Could not read temperature data");
			}
			temperature[i] = tempC;
		}
	}
}

float MyDS18B20::getTemperature(int i) {
	return temperature[i];
}

int  MyDS18B20::getNumberOfDevices() {
	return numberOfDevices;
}

void MyDS18B20::printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (deviceAddress[i] < 16)
			Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}