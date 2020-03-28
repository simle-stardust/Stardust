#include <Arduino.h>
#include <SPI.h>
#include "rtc.h"
#include "dht22.h"
#include "ds18b20.h"
#include <HoneywellTruStabilitySPI.h>

#define DHT22pin 22
#define ONE_WIRE_BUS 24
#define PRESSURE_SS 26

MyRTC rtc;
MyDHT dht(DHT22pin);
MyDS18B20 temp(ONE_WIRE_BUS);
TruStabilityPressureSensor pressure(PRESSURE_SS, 0, 15.0 );

void setup()
{


  Serial.begin(115200);
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  SPI.begin();

  rtc.init();
  temp.init();
  pressure.begin();
}

void loop()
{
	rtc.getStatus();

	Serial.print("RTC:		");
	rtc.printDateTime(rtc.getTime());

	Serial.print(",	");
	rtc.getTemp().Print(Serial);
	// you may also get the temperature as a float and print it
	// Serial.print(temp.AsFloatDegC());
	Serial.println(" *C;");

	Serial.print("DHT22:		");
	Serial.print(dht.getHumidity());
	Serial.print(" RH%,		");

	Serial.print(dht.getTemperature());
	Serial.println(" *C;");

	Serial.print("DS18B20:				");
	Serial.print(temp.getTemperature());
	Serial.println(" *C;");

	pressure.readSensor();
	Serial.print("Pressure:	");
	Serial.print(pressure.pressure());
	Serial.print(" psi,		");

	Serial.print(pressure.temperature());
	Serial.println(" *C;");

	Serial.println();
	delay(1000);
}