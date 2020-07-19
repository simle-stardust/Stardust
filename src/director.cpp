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

	if (millis() - lastOperation > SAMPLING_TIME)
	{
		logger.tick(flight.phase, flight.ground, flight.inFlight, flight.sampling, flight.finished); // Poll sensors and save to SD card each sampling period

		if (millis() - wifi_query_tick > 10000)
		{
			// ask wifi for state every 10s
			this->getStatus();
			// send current state to wifi
			this->sendStatusToWiFi();

			if (this->getLoRaStatus())
			{
				// if LoRa connection is valid, allow to overwrite current status
				int LoRaPhase = (int)this->getStardustFlightState();
				if ((LoRaPhase > flight.phase) && (flight.phase >= 0) && (flight.phase <= 4))
				{
					flight.phase = LoRaPhase;
				}
			}

			wifi_query_tick = millis();
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
			if ((sensors.altitude(0).average > 300 && sensors.altitude(0).isValid) 
				|| (sensors.altitude(1).average > 1000 && sensors.altitude(1).isValid)
				|| (sensors.altitude(2).average > 1000 && sensors.altitude(2).isValid))
			{
				this->nextPhase();
			}


			break;
		case 1: // Flight
			if (flight.inFlight != true)
			{
				Serial.println("In Flight");

				flight.inFlight = true;
			}
			
			// Check if it's time to switch state
			if ((sensors.altitude(0).average > 13000 && sensors.altitude(0).isValid) 
				|| (sensors.altitude(1).average > 15000 && sensors.altitude(1).isValid)
				|| (sensors.altitude(2).average > 15000 && sensors.altitude(2).isValid))
			{
				this->nextPhase();
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

			// Check if it's time to switch state
			if ((sensors.altitude(0).average < 12000 && sensors.altitude(0).isValid) 
				|| (sensors.altitude(1).average < 10000 && sensors.altitude(1).isValid)
				|| (sensors.altitude(2).average < 10000 && sensors.altitude(2).isValid))
				// and add condition for minimal state duration
			{
				this->nextPhase();
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
			if ((sensors.altitude(0).average < 12000 && sensors.altitude(0).isValid) 
				|| (sensors.altitude(1).average < 10000 && sensors.altitude(1).isValid)
				|| (sensors.altitude(2).average < 10000 && sensors.altitude(2).isValid))
			{
				this->nextPhase();
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
	}
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

void Flight::getStatus() 
{
	uint8_t buf[4];
	size_t ret = 0;

	Serial3.println("@MarcinGetStatus!");
	ret = Serial3.readBytes(&buf[0], 4);
	Serial3.flush();

	if (ret == 4 && buf[0] == '@' && buf[3] == '!')
	{
		MainGondolaFlags = ((uint16_t)buf[1] << 8) + buf[2];
	}
	else 
	{
		// set all errors to true
		MainGondolaFlags = 0xFFFF;
	}
}

bool Flight::getGPSStatus()
{
	if (MainGondolaFlags == 0xFFFF)
	{
		return false;
	}
	else 
	{
		// check if GPS_ERR is ON in main gondola
		return (MainGondolaFlags & 0x0800) ? false : true;
	}
}

uint8_t Flight::getStardustFlightStatus()
{
	if (MainGondolaFlags == 0xFFFF)
	{
		return 0xFF;
	}
	else 
	{	// return 3 LSbits from MainGondola flags
		return (MainGondolaFlags & 0x03);
	}
}

bool Flight::getLoRaStatus()
{
	if (MainGondolaFlags == 0xFFFF)
	{
		return false;
	}
	else 
	{
		// check if LORA_ERR is ON in main gondola
		return (MainGondolaFlags & 0x2000) ? false : true;
	}
}	

bool Flight::getLiftoffStatus()
{
	if (MainGondolaFlags == 0xFFFF)
	{
		// flight is on is false in case of WiFi error
		return false;
	}
	else 
	{
		// check if FLIGHT_IS_ON is ON in main gondola
		return (MainGondolaFlags & 0x0040) ? true : false;
	}
}

void Flight::sendStatusToWiFi()
{
	uint8_t buf[4];
	size_t ret = 0;

	Serial3.print("@MarcinSetStatus:");
	Serial3.print(flight.phase);
	Serial3.println("!");
	ret = Serial3.readBytes(&buf[0], 10);
	Serial3.flush();
}