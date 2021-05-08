#include "dht22.h"



float MyDHT::getHumidity()
{
	readSensor();
	return (float)humidity;
}

float MyDHT::getTemperature()
{
	readSensor();
	return (float)temperature;
}

int MyDHT::getStatus()
{
	return this->lastErrCode;
}

void MyDHT::readSensor()
{
	this->lastErrCode = SimpleDHT22::read2(&temperature, &humidity, NULL);
	if (this->lastErrCode != SimpleDHTErrSuccess)
	{
		// TODO: Works despite error codes?
		//Serial.print("Read DHT22 failed, err=");
		//Serial.println(err);
	}
}