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
	static unsigned long wifi_query_tick = 0;
	static reason_t reason = (reason_t)0;

	if (millis() - lastOperation > SAMPLING_TIME)
	{
		if (millis() - wifi_query_tick > 10000)
		{
			if (sensors.getLoRaStatus())
			{
				// if LoRa connection is valid, allow to overwrite current status
				int LoRaPhase = (int)sensors.getStardustFlightStatus();
				if ((LoRaPhase > flight.phase) && (LoRaPhase >= 0) && (LoRaPhase <= 4))
				{
					flight.phase = LoRaPhase;
					reason = REASON_LORA;
				}
			}

			wifi_query_tick = millis();
		}

		logger.tick(flight.phase, flight.ground, flight.inFlight, flight.sampling, flight.finished, reason); // Poll sensors and save to SD card each sampling period

		switch (flight.phase)
		{
		default:
			if (flight.phase < 0)
				flight.phase = 0;
			if (flight.phase > 4)
				flight.phase = 4;
			break;
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

			// Check if it's time to switch state
			if (sensors.altitude(0).isValid)
			{
				if (sensors.altitude(0).average > 300)
				{
					reason = REASON_ALTITUDE;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(1).isValid)
			{
				if (sensors.altitude(1).average > 1000)
				{
					reason = REASON_PRESSURE1;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(2).isValid)
			{
				if (sensors.altitude(2).average > 1000)
				{
					reason = REASON_PRESSURE2;
					this->nextPhase();
				}
			}

			break;
		case 1: // Flight
			if (flight.inFlight != true)
			{
				Serial.println("In Flight");

				flight.inFlight = true;
			}

			// Check if it's time to switch state
			if (sensors.altitude(0).isValid)
			{
				if (sensors.altitude(0).average > 13000)
				{
					reason = REASON_ALTITUDE;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(1).isValid)
			{
				if (sensors.altitude(1).average > 15000)
				{
					reason = REASON_PRESSURE1;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(2).isValid)
			{
				if (sensors.altitude(2).average > 15000)
				{
					reason = REASON_PRESSURE2;
					this->nextPhase();
				}
			}

			break;
		case 2:							  // Sampling ON
			if (flight.sampling == false) // run once
			{
				Serial.println(" Sampling ON");

				Serial.println("Open sequence start");
				
				for (uint8_t i = 1; i <= SERVO_NUM; ++i)
				{
					servos.setOpen(i);

					
				}
				flight.sampling = true;
			}

			servos.tick();

			// Check if it's time to switch state
			if (sensors.altitude(0).isValid)
			{
				if (sensors.altitude(0).average < 12000)
				{
					reason = REASON_ALTITUDE;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(1).isValid)
			{
				if (sensors.altitude(1).average < 10000)
				{
					reason = REASON_PRESSURE1;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(2).isValid)
			{
				if (sensors.altitude(2).average < 10000)
				{
					reason = REASON_PRESSURE2;
					this->nextPhase();
				}
			}


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

			// Check if it's time to switch state
			if (sensors.altitude(0).isValid)
			{
				if (sensors.altitude(0).average < 1000)
				{
					reason = REASON_ALTITUDE;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(1).isValid)
			{
				if (sensors.altitude(1).average < 1000)
				{
					reason = REASON_PRESSURE1;
					this->nextPhase();
				}
			}
			else if (sensors.altitude(2).isValid)
			{
				if (sensors.altitude(2).average < 1000)
				{
					reason = REASON_PRESSURE2;
					this->nextPhase();
				}
			}

			break;
		case 4: // After Launch
			if (flight.finished != flight.finished)
			{
				Serial.println(" Mission complete");
			}
			break;
		}
		

		this->saveFlightToEEPROM();

		lastOperation = millis();
	}
}

void Flight::nextPhase()
{
	static unsigned long status_change_tick = 0;
	static unsigned long time_since_change = 0;
	static unsigned long min_duration = 60000;

	time_since_change = millis() - status_change_tick;

	if (time_since_change > min_duration)
	{
		status_change_tick = millis();
		flight.phase++;
		Serial.print("\n Advancing to Phase: ");
		Serial.println(flight.phase);
	}
	else
	{
		Serial.println("TooSoon");
	}
}

void Flight::prevPhase()
{
	flight.phase--;
	Serial.print("\n Advancing to Phase: ");
	Serial.println(flight.phase);
}

void Flight::setPhase(int phase)
{
	flight.phase = phase;
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