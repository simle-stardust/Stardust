#pragma once
#include "logger.h"
#include "sdcard.h"
#include "rtc.h"
#include "servo.h"
#include "sensors.h"

#define SERVO_NUM (int)6

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
	MyRTC rtc;
	MySensors sensors;
	MyServo servos;
	Logger logger;

	unsigned long lastOperation = 0;
	unsigned long lastPhaseChange = 0;

	uint16_t MainGondolaFlags = 0;

public:
	Flight();
	void init();
	void tick();
	void nextPhase();
	void prevPhase();
	void setPhase();
	void readFlightFromEEPROM();
	void saveFlightToEEPROM();

	void getStatus();
	bool getGPSStatus();
	uint8_t getStardustFlightStatus();
	bool getLoRaStatus();
	bool getLiftoffStatus();
	void sendStatusToWiFi();
};