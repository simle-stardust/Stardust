#pragma once
#include <Arduino.h>
#include "rtc.h"
#include "dht22.h"
#include "ds18b20.h"
#include <HoneywellTruStabilitySPI.h>

#define DHT22_main 33
#define DHT22_mech 27
#define DHT22_sens 41

#define ONE_WIRE_main 35
#define ONE_WIRE_mech 31
#define ONE_WIRE_sens 37

#define LIFTOFF_PRESSURE 1014.0f

#define PRESSURE_SS_main 25
#define PRESSURE_SS_sens 39

struct MyAltitude
{
	unsigned long timestamp = 0;
	bool isValid = 0;
	int32_t value = 0;
	int32_t average = 0;
	uint8_t buf[8];
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

	// temperature
	MyDS18B20 temp_main;
	MyDS18B20 temp_sens;

	// pressure
	TruStabilityPressureSensor pressure_main;
	TruStabilityPressureSensor pressure_sens;
	struct MyPressure pressure_1;
	struct MyPressure pressure_2;

	// altitude
	struct MyAltitude altitude_0;
	struct MyAltitude altitude_1;
	struct MyAltitude altitude_2;

public:
	

	MySensors();

	void init(MyRTC *rtc);

	void readSensors();

	RtcDateTime time();
	String getTime();
	String getDate();
	void getAltitude();
	void getPressure(int sensor);

	struct MyAltitude altitude(int sensor);
	struct MyPressure pressure(int sensor);
	float temperature(int sensor);
	float humidity(int sensor);

	float pressureToAltitude(float seaLevel, float atmospheric, float temp);

	// Sensor ID's
	// 0 - RTC (temperature)
	// 1 - Pressure sensor on the main board (pres, temp)
	// 2 - Pressure sensor on the sensor board (pres, temp)
	// 3 - DHT 22 on the main board (humid, temp)
	// 4 - DHT 22 on the sensor board (humid, temp)
};
