#include <Arduino.h>
#include "logger.h"
#include "sdcard.h"
#include "rtc.h"
#include "dht22.h"
#include "ds18b20.h"
#include "servo.h"
#include <HoneywellTruStabilitySPI.h>

#define DHT22_main 33
#define DHT22_mech 27
#define DHT22_sens 41
#define ONE_WIRE_main 35
#define ONE_WIRE_mech 31
#define ONE_WIRE_sens 37
#define PRESSURE_SS_main 25
#define PRESSURE_SS_sens 39
#define POTENTIOMETER A14
#define SD_CARD_SS 4


OneWire onewire_main(ONE_WIRE_main);
OneWire onewire_sens(ONE_WIRE_sens);
MySD flash(SD_CARD_SS);
MyRTC rtc;
MyDHT dht_main(DHT22_main);
MyDHT dht_sens(DHT22_sens);
MyDS18B20 temp_main;
MyDS18B20 temp_sens;
TruStabilityPressureSensor pressure_main(PRESSURE_SS_main, -15.0, 15.0 );
TruStabilityPressureSensor pressure_sens(PRESSURE_SS_sens, -15.0, 15.0 );

MyServo servo(2);

Logger logger;

uint16_t servo_num = 1;
bool close = 0;


void setup()
{
	pinMode(13, OUTPUT);

	Serial.begin(115200);
	Serial.print("compiled: ");
	Serial.print(__DATE__);
	Serial.println(__TIME__);

	servo.init();
	rtc.init();
	temp_main.init(onewire_main);
	temp_sens.init(onewire_sens);
	pressure_main.begin();
	pressure_sens.begin();
	flash.init(rtc.timeString(rtc.getTime()), &Serial);
	logger.init(&flash);


	for(uint8_t i = 1; i < 7; ++i) {
		servo.setClosed(i);
	}
}

void loop()
{
	if(servo.ready()) {
		Serial.println("Finished Queue");
		for(uint8_t i = 1; i < 7; ++i) {
			if(servo.getStatus(i) == 1) servo.setClosed(i);
			if(servo.getStatus(i) == 0) servo.setOpen(i);
		}
	}

	servo.tick();

	rtc.getStatus();
	Serial.print("RTC:		");
	logger.add(rtc.dateString(rtc.getTime()));
	Serial.print(",");
	logger.add(rtc.timeString(rtc.getTime()));
	Serial.print(",	");
	logger.add(rtc.getTemp().AsFloatDegC());
	Serial.println(" *C;");

	Serial.print("DHT22 main:	");
	logger.add(dht_main.getHumidity());
	Serial.print(" RH%,		");

	logger.add(dht_main.getTemperature());
	Serial.println(" *C;");

	Serial.print("DHT22 sens:	");
	logger.add(dht_sens.getHumidity());
	Serial.print(" RH%,		");

	logger.add(dht_sens.getTemperature());
	Serial.println(" *C;");
	
	temp_main.update();
	for (int i = 0; i < temp_main.getNumberOfDevices(); i++)
	{
		Serial.print("DS18B20 main [");
		Serial.print(i);
		Serial.print("]:				");
		logger.add(temp_main.getTemperature(i));
		Serial.println(" *C;");
	}

	temp_sens.update();
	for (int i = 0; i < temp_sens.getNumberOfDevices(); i++)
	{
		Serial.print("DS18B20 sens [");
		Serial.print(i);
		Serial.print("]:				");
		logger.add(temp_sens.getTemperature(i));
		Serial.println(" *C;");
	}

	pressure_main.readSensor();
	Serial.print("Pressure main:	");
	logger.add(pressure_main.pressure());
	Serial.print(" psi,		");

	logger.add(pressure_main.temperature());
	Serial.println(" *C;");

	pressure_sens.readSensor();
	Serial.print("Pressure sens:	");
	logger.add(pressure_sens.pressure());
	Serial.print(" psi,		");

	logger.add(pressure_sens.temperature());
	Serial.println(" *C;");

	logger.save();

	digitalWrite(13, HIGH);
	delay(997);
	digitalWrite(13, LOW);
}