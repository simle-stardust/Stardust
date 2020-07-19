#include "director.h"
#include <EEPROM.h>

Flight::Flight() : servos(SERVO_NUM) {

}

void Flight::init()
{
	rtc.init();
	// Year/Month/Day directory with file named after init time.
	flash.init(rtc.dateString(rtc.getTime()), rtc.timeString(rtc.getTime()));
	sensors.init(&rtc);
	logger.init(&flash, &sensors);
	servos.init();

	readFlightFromEEPROM();

	Serial.print("Flight phase: ");
	Serial.println(flight.phase);

	Serial.print("Ground: ");
	Serial.println(flight.ground);

	Serial.print("Flight: ");
	Serial.println(flight.inFlight);

	Serial.print("Sampling: ");
	Serial.println(flight.sampling);

	Serial.print("Finished: ");
	Serial.println(flight.finished);
}

void Flight::tick()
{
	logger.tick(flight.phase, flight.ground, flight.inFlight, flight.sampling, flight.finished); // Poll sensors and save to SD card each sampling period

	switch (flight.phase)
	{
	default:
		if (flight.phase < 0)
			flight.phase = 0;
		if (flight.phase > 4)
			flight.phase = 4;
	case 0: // Pre-launch
		//Serial.println(" Awaiting liftoff");
		// Make sure the valves are closed?
		if (flight.ground != true || flight.inFlight != false)
		{
			flight.ground = true;
			flight.inFlight = false;
			flight.sampling = false;
			flight.finished = false;
		}

		break;
	case 1: // Flight
		if (flight.inFlight != true)
		{
			Serial.println("In Flight");

			flight.inFlight = true;
		}

		break;
	case 2:							  // Sampling ON
		if (flight.sampling == false) // run once
		{
			Serial.println(" Sampling ON");

			Serial.println("Open sequence start");
			Serial.print("Set ");
			for (uint8_t i = 1; i <= SERVO_NUM; ++i)
			{
				servos.setOpen(i);

				Serial.print(i);
				Serial.print(", ");
			}
			Serial.println(" to Open");
			flight.sampling = true;
		}

		servos.tick();

		break;
	case 3:							 // Sampling OFF
		if (flight.sampling == true) // run once
		{
			Serial.println(" Sampling OFF - Safing");
			Serial.println("Close sequence start");
			Serial.print("Set ");
			for (uint8_t i = 1; i <= SERVO_NUM; ++i)
			{
				servos.setClosed(i);

				Serial.print(i);
				Serial.print(", ");
			}
			Serial.println(" to Closed");
			flight.sampling = false;
		}

		servos.tick();
		break;
	case 4: // After Launch
		if (flight.finished != flight.finished)
		{
			Serial.println(" Mission complete");
		}
		break;
	}

	this->saveFlightToEEPROM();
}

void Flight::nextPhase()
{
	flight.phase++;
	Serial.print("\n Advancing to Phase: ");
	Serial.println(flight.phase);
}

void Flight::prevPhase()
{
	flight.phase--;
	Serial.print("\n Advancing to Phase: ");
	Serial.println(flight.phase);
}

void Flight::readFlightFromEEPROM() {

	flight.phase = EEPROM.read(0);
	flight.ground = EEPROM.read(sizeof(int));
	flight.inFlight = EEPROM.read(sizeof(int) + sizeof(bool));
	flight.sampling = EEPROM.read(sizeof(int) + sizeof(bool) * 2);
	flight.finished = EEPROM.read(sizeof(int) + sizeof(bool) * 3);
}

void Flight::saveFlightToEEPROM() {
	EEPROM.update(0, flight.phase);
	EEPROM.update(sizeof(int), flight.ground);
	EEPROM.update(sizeof(int) + sizeof(bool), flight.inFlight);
	EEPROM.update(sizeof(int) + sizeof(bool) * 2, flight.sampling);
	EEPROM.update(sizeof(int) + sizeof(bool) * 3, flight.finished);
}

bool Flight::getLiftoffStatus() {

}