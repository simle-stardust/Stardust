#pragma once
#include <Arduino.h>
#include "rtc.h"
#include "dht22.h"
#include "ds18b20.h"
#include <HoneywellTruStabilitySPI.h>
#include <Adafruit_GPS.h>

// Pinout definition
#define DHT22_main 36
#define DHT22_mech 45
#define DHT22_sens 35

#define ONE_WIRE_main 28
#define ONE_WIRE_mech 47
#define ONE_WIRE_sens 29

#define LIFTOFF_PRESSURE 1014.0f

#define PRESSURE_SS_main 48
#define PRESSURE_SS_sens 33

struct MyAltitude
{
	unsigned long timestamp = 0;
	bool isValid = 0;
	int32_t value = 0;
	int32_t average = 0;
	uint8_t buf[10];
	size_t ret = 0;

	static const int measurment_num = 30;
	int32_t history[measurment_num];
	uint8_t pointer = 0;
};

struct MyPressure
{
	unsigned long timestamp = 0;
	bool isValid = 0;
	float value = 0;
	float average = 0;
	static const int measurment_num = 30;
	float history[measurment_num];
	uint8_t pointer = 0;
};

class MySensors
{
private:
	MyRTC *rtc;

	OneWire onewire_main;
	OneWire onewire_sens;

	// humidity

	MyDHT dht_main;
	MyDHT dht_sens;
	MyDHT dht_mech;

	// temperature
	MyDS18B20 temp_main;
	MyDS18B20 temp_sens;

	// pressure
	TruStabilityPressureSensor pressure_main;
	TruStabilityPressureSensor pressure_sens;
	
	Adafruit_GPS *GPS_main;

	struct MyPressure pressure_1;
	struct MyPressure pressure_2;

	// altitude
	struct MyAltitude altitude_0;
	struct MyAltitude altitude_1;
	struct MyAltitude altitude_2;

	uint16_t MainGondolaFlags = 0;

public:
	

	MySensors();

	void init(MyRTC *rtc, Adafruit_GPS* _GPS_main);

	void readSensors();

	RtcDateTime time();
	String getTime();
	String getDate();
	int isDateTimeValid();
	void getAltitude();
	void getPressure(int sensor);

	struct MyAltitude altitude(int sensor);
	struct MyPressure pressure(int sensor);
	float temperature(int sensor);
	int temperatureStatus(int sensor);
	float humidity(int sensor);

	float pressureToAltitude(float seaLevel, float atmospheric, float temp);


	// Sensor ID's
	// 0 - RTC (time, temperature) / GPS (altitude)
	// 1 - Pressure sensor on the main board (pressure, temperature, altitude)
	// 2 - Pressure sensor on the sensor board (pressure, temperature, altitude)
	// 3 - DHT 22 on the main board (humidity, temperature)
	// 4 - DHT 22 on the sensor board (humidity, temperature)
	// 5 - DHT 22 on the mech board (humidity, temperature)
	// 10 ... 19 - DS18B20 Main Board (temperature)
	// 20 ... 29 - DS18B20 Sensor Board (temperature)
};
