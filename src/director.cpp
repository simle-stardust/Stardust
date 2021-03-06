#include "director.h"
#include <EEPROM.h>

Flight::Flight() : servos(SERVO_NUM), GPS_main(&Serial3)
{
}

void Flight::init()
{
	rtc.init();
	// Year/Month/Day directory with file named after init time.
	flash.init(rtc.dateString(rtc.getTime()), rtc.timeString(rtc.getTime()));
	sensors.init(&rtc, &GPS_main);
	udp.init();
	adc.init();
	logger.init(&flash, &sensors, &udp, &adc);
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
	static reason_t reason = (reason_t)0;
	int udp_phase = 0;

	// read data from the GPS in the 'main loop'
	char c = GPS_main.read();

#ifdef GPS_DEBUG
	// if you want to debug, this is a good time to do it!
	if (c) Serial.print(c);
#endif


	if (GPS_main.newNMEAreceived()) {
#ifdef GPS_DEBUG
		Serial.println("NEW GPS DATA");
		Serial.println(GPS_main.lastNMEA()); // this also sets the newNMEAreceived() flag to false
#endif
		// a tricky thing here is if we print the NMEA sentence, or data
		// we end up not listening and catching other sentences!
		// so be very wary if using OUTPUT_ALLDATA and trying to print out data
		GPS_main.parse(GPS_main.lastNMEA()); // this also sets the newNMEAreceived() flag to false
	}

	if (millis() - lastOperation > SAMPLING_TIME)
	{
		adc.tick();
		
		// Poll sensors and UDP server and save to SD card each sampling period
		udp_phase = logger.tick(flight.phase, flight.ground, flight.inFlight, 
									flight.sampling, flight.finished, reason); 
		if ((udp_phase != (int)UPLINK_NOTHING_RECEIVED) && (udp_phase > flight.phase) && (udp_phase >= 0) && (udp_phase <= 4))
		{
			nextPhase();
			reason = REASON_UDP;
		}

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

				servos.openSequence();

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
				servos.closeSequence();
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

void Flight::readFlightFromEEPROM()
{

	flight.phase = EEPROM.read(0);
	flight.ground = EEPROM.read(sizeof(int));
	flight.inFlight = EEPROM.read(sizeof(int) + sizeof(bool));
	flight.sampling = EEPROM.read(sizeof(int) + sizeof(bool) * 2);
	flight.finished = EEPROM.read(sizeof(int) + sizeof(bool) * 3);
}

void Flight::saveFlightToEEPROM()
{
	EEPROM.update(0, flight.phase);
	EEPROM.update(sizeof(int), flight.ground);
	EEPROM.update(sizeof(int) + sizeof(bool), flight.inFlight);
	EEPROM.update(sizeof(int) + sizeof(bool) * 2, flight.sampling);
	EEPROM.update(sizeof(int) + sizeof(bool) * 3, flight.finished);
}