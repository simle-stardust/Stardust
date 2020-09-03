#pragma once
#include "logger.h"
#include "sdcard.h"
#include "rtc.h"
#include "servo.h"
#include "sensors.h"
#include <Adafruit_GPS.h>

#define SERVO_NUM (int)8

#define SAMPLING_TIME 1000 // Czas próbkowania

#define PHASE_LOCK = 10000 // jak długo zablokować zmianę fazy na następną? minimalny czas trwania fazy lotu.

struct MyFlight
{
	int phase = 0;
	bool ground = false;
	bool inFlight = false;
	bool sampling = false;
	bool finished = false;
	int liftoff_day = 0;
	int liftoff_hour = 0;
	int liftoff_minute = 0;
};

class Flight
{
private:
	struct MyFlight flight;

	MySD flash;
	MyUDP udp;
	MyADC adc;
	MyRTC rtc;
	MySensors sensors;
	MyServo servos;
	Adafruit_GPS GPS_main;
	Logger logger;

	unsigned long lastOperation = 0;
	unsigned long lastPhaseChange = 0;

public:
	Flight();
	void init();
	void tick();
	void nextPhase();
	void prevPhase();
	void setPhase(int Phase);
	void readFlightFromEEPROM();
	void saveFlightToEEPROM();

};