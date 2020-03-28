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

void MyDHT::readSensor()
{
	int err = SimpleDHTErrSuccess;
	if ((err = SimpleDHT22::read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
	{
		// TODO: Works despite error codes?
		//Serial.print("Read DHT22 failed, err=");
		//Serial.println(err);
	}
}