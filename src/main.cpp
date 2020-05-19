#include <Arduino.h>
#include "logger.h"
#include "sdcard.h"
#include "rtc.h"
#include "dht22.h"
#include "ds18b20.h"
#include <HoneywellTruStabilitySPI.h>

#define DHT22pin 22
#define ONE_WIRE_BUS 24
#define PRESSURE_SS 26
#define POTENTIOMETER A14
#define SD_CARD_SS 4


OneWire oneWire(ONE_WIRE_BUS);
MySD flash(SD_CARD_SS);
MyRTC rtc;
MyDHT dht(DHT22pin);
MyDS18B20 temp;
TruStabilityPressureSensor pressure(PRESSURE_SS, 0, 15.0 );

Logger logger;

DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; 


void setup()
{
	pinMode(13, OUTPUT);

	Serial.begin(115200);
	Serial.print("compiled: ");
	Serial.print(__DATE__);
	Serial.println(__TIME__);

	SPI.begin();
	rtc.init();
	temp.init(oneWire);
	pressure.begin();
	logger.init(&flash, &Serial, rtc.timeString(rtc.getTime()));

}

void loop()
{
	String dataLine = "";
	rtc.getStatus();

	Serial.print("RTC:		");
	logger.add(rtc.dateString(rtc.getTime()));
	Serial.print(",");
	logger.add(rtc.timeString(rtc.getTime()));
	Serial.print(",	");
	logger.add(rtc.getTemp().AsFloatDegC());
	Serial.println(" *C;");

	Serial.print("DHT22:		");
	logger.add(dht.getHumidity());
	Serial.print(" RH%,		");

	logger.add(dht.getTemperature());
	Serial.println(" *C;");
	
	temp.update();
	for (int i = 0; i < temp.getNumberOfDevices(); i++)
	{
		Serial.print("DS18B20 [");
		Serial.print(i);
		Serial.print("]:				");
		logger.add(temp.getTemperature(i));
		Serial.println(" *C;");
	}

	pressure.readSensor();
	Serial.print("Pressure:	");
	logger.add(pressure.pressure());
	Serial.print(" psi,		");

	logger.add(pressure.temperature());
	Serial.println(" *C;");

	Serial.print("Knob:		");
	logger.add((float)analogRead(POTENTIOMETER)*100/1023);
	Serial.println(" %;");

	Serial.println();

	logger.save();
	digitalWrite(13, HIGH);
	delay(500);
	digitalWrite(13, LOW);
	delay(500);
}