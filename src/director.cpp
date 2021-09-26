#include "director.h"
#include <EEPROM.h>
#include <Watchdog.h>

#define PUMP_SAMPLING_ON_PWM_VALUE   255
#define PUMP_SAMPLING_OFF_PWM_VALUE  0
#define EXT_LEDS_ADDRESS 0x26

uint32_t state_cnt = 0;

Watchdog watchdog;
pca9555 *external_leds;

Flight::Flight() : servos(), GPS_main(&Serial3)
{
}

void Flight::init()
{
	watchdog.enable(Watchdog::TIMEOUT_8S);

	rtc.init();
	// Year/Month/Day directory with file named after init time.
	flash.init(rtc.dateString(rtc.getTime()), rtc.timeString(rtc.getTime()));
	sensors.init(&rtc, &GPS_main);
	udp.init();
	adc.init();
	servos.init();
	logger.init(&flash, &sensors, &udp, &adc, &servos, &pwms);

	readFlightFromEEPROM();

	Serial.print("Flight phase: ");
	Serial.println(flight.phase);

	Serial.print("Ground: ");
	Serial.println(flight.ground);

	Serial.print("Flight: ");
	Serial.println(flight.inFlight);

	Serial.print("Sampling: ");
	Serial.println(flight.sampling);

	Serial.print("Valves Opened: ");
	Serial.println(flight.valvesOpened);
	Serial.print("Valves Closed: ");
	Serial.println(flight.valvesClosed);

	Serial.print("Finished: ");
	Serial.println(flight.finished);
	
	external_leds = new pca9555(EXT_LEDS_ADDRESS);//instance

	external_leds->begin();
	external_leds->gpioPinMode(OUTPUT);
	for (uint8_t i = 0; i < 16; i++)
	{
		external_leds->gpioDigitalWrite(i, 0);
	}
}

void Flight::tick()
{
	static reason_t reason = (reason_t)0;
	
	RtcDateTime timestamp;

	uplink_t last_uplink = UPLINK_NONE;
	uint8_t uplink_val1 = 0;
	uint8_t uplink_val2 = 0;
	uint32_t time_since_last_uplink = 0;


	// read data from the GPS in the 'main loop'
	(void)GPS_main.read();



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
		watchdog.reset();
		servos.tick();
		adc.tick();
		
		// Poll sensors and UDP server and save to SD card each sampling period
		logger.tick(flight.phase, flight.ground, flight.inFlight, 
									flight.sampling, flight.finished, reason); 

		last_uplink = udp.getLastUplink(&uplink_val1, &uplink_val2);
		time_since_last_uplink = udp.timeSinceLastPing();

		timestamp = rtc.getTime().TotalSeconds();

		if (time_since_last_uplink < 2)
		{
			// we have connection
			flight.last_uplink_timestamp = timestamp;
		}

		switch (last_uplink)
		{
			case UPLINK_SET_PUMP:
				if (uplink_val1 == 1)
				{
					pwms.set(PWM_PUMP_1, uplink_val2);
				}
				else if (uplink_val1 == 2)
				{
					pwms.set(PWM_PUMP_2, uplink_val2);
				}
				else if (uplink_val1 == 3)
				{
					pwms.set(PWM_HEATING, uplink_val2);
				}
				break;
			case UPLINK_SET_VALVE:
				// should we implement some logic to prevent 
				// manual opening and closing if we are in some specific state?
				if (uplink_val2 == 0)
				{
					//open
					servos.setOpen(uplink_val1);
				}
				else if (uplink_val2 == 1)
				{
					//close
					servos.setClosed(uplink_val1);
				}
				break;
			case UPLINK_SET_STATE:
				if ((uplink_val1 >= 0) && (uplink_val1 <= 4))
				{
					setPhase(uplink_val1);
					reason = REASON_UDP;
				}
				break;
			default:
				break;
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
				flight.valvesClosed = false;
				flight.valvesOpened = false;
			}

			// ONLY USE THE LOGIC BELOW TO CHANGE STATES IF WE ARE ==NOT==
			// IN MANUAL MODE (LONG TIME HAS PASSED SINCE LAST PING)
			if ((!rtc.getStatus() && (timestamp - flight.last_uplink_timestamp > 120)) ||
				(time_since_last_uplink > 120))
			{
				// Check if it's time to switch state
				if (sensors.altitude(0).isValid)
				{
					if (sensors.altitude(0).average > 1000)
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
			}

			break;
		case 1: // Flight
			if (flight.inFlight != true)
			{
				Serial.println("In Flight");

				flight.inFlight = true;
			}

			// ONLY USE THE LOGIC BELOW TO CHANGE STATES IF WE ARE ==NOT==
			// IN MANUAL MODE (LONG TIME HAS PASSED SINCE LAST PING)
			if ((!rtc.getStatus() && (timestamp - flight.last_uplink_timestamp > 120)) ||
				(time_since_last_uplink > 120))
			{
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
			}

			break;
		case 2:							  // Sampling ON
			if (flight.sampling == false) // run once
			{
				Serial.println(" Sampling ON");
				Serial.println("Open sequence start");

				flight.sampling = true;
			}

			if (flight.valvesOpened != true && state_cnt == 1)
			{
				servos.openSequence();
			}

			if (state_cnt >= 0)
			{
				pwms.set(PWM_PUMP_1, PUMP_SAMPLING_ON_PWM_VALUE);
			}
			if (state_cnt >= 1)
			{
				pwms.set(PWM_PUMP_2, PUMP_SAMPLING_ON_PWM_VALUE);
			}
			if (state_cnt >= 40)
			{
				// Actually check state of the servo machine?
				flight.valvesOpened = true;
			}

			// ONLY USE THE LOGIC BELOW TO CHANGE STATES IF WE ARE ==NOT==
			// IN MANUAL MODE (LONG TIME HAS PASSED SINCE LAST PING)
			if ((!rtc.getStatus() && (timestamp - flight.last_uplink_timestamp > 120)) ||
				(time_since_last_uplink > 120))
			{
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
			}

			break;
		case 3:							 // Sampling OFF
			if (flight.sampling == true) // run once
			{
				Serial.println(" Sampling OFF - Safing");
				Serial.println("Close sequence start");
				

				flight.sampling = false;
			}

			//if (flight.valvesClosed != true && state_cnt == 1)
			//{
			//	servos.closeSequence();
			//}

			if (flight.valvesClosed != true && state_cnt == 2)
			{
				servos.setClosed(7);
			}
			if (flight.valvesClosed != true && state_cnt == 7)
			{
				servos.setClosed(6);
			}
			if (flight.valvesClosed != true && state_cnt == 12)
			{
				servos.setClosed(5);
			}
			if (flight.valvesClosed != true && state_cnt == 17)
			{
				servos.setClosed(4);
			}
			if (flight.valvesClosed != true && state_cnt == 22)
			{
				servos.setClosed(3);
			}
			if (flight.valvesClosed != true && state_cnt == 27)
			{
				servos.setClosed(2);
			}
			if (flight.valvesClosed != true && state_cnt == 32)
			{
				servos.setClosed(1);
				pwms.set(PWM_PUMP_1, PUMP_SAMPLING_OFF_PWM_VALUE);
			}
			if (flight.valvesClosed != true && state_cnt == 37)
			{
				flight.valvesClosed = true;
				pwms.set(PWM_PUMP_2, PUMP_SAMPLING_OFF_PWM_VALUE);
			}
			
			// ONLY USE THE LOGIC BELOW TO CHANGE STATES IF WE ARE ==NOT==
			// IN MANUAL MODE (LONG TIME HAS PASSED SINCE LAST PING)
			if ((!rtc.getStatus() && (timestamp - flight.last_uplink_timestamp > 120)) ||
				(time_since_last_uplink > 120))
			{
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
			}

			break;
		case 4: // After Launch
			if (flight.finished != flight.finished)
			{
				Serial.println(" Mission complete");
			}
			break;
		}

		if (state_cnt != UINT32_MAX)
		{
			state_cnt++;
		}

		this->saveFlightToEEPROM();

		// controlling external LEDs

		external_leds->gpioDigitalWrite(0, (bool)(state_cnt & 0x01));
		// LED1, LED2 and LED3 inform about current experiment state
		external_leds->gpioDigitalWrite(1, (bool)(flight.phase & 0x01));
		external_leds->gpioDigitalWrite(2, (bool)(flight.phase & 0x02));
		external_leds->gpioDigitalWrite(3, (bool)(flight.phase & 0x04));
		// LED4 is UDP status
		external_leds->gpioDigitalWrite(4, (bool)(udp.getStatus() == 0 ? 1 : 0));
		// LED5 is time since last ping
		external_leds->gpioDigitalWrite(5, (bool)(time_since_last_uplink < 120 ? 1 : 0));
		// LED6 is GPS status
		MyAltitude alt = sensors.altitude(0);
		external_leds->gpioDigitalWrite(6, alt.isValid);
		// LED7 is pressure 1 status
		alt = sensors.altitude(1);
		external_leds->gpioDigitalWrite(7, alt.isValid);
		// LED8 is pressure 2 status
		alt = sensors.altitude(2);
		external_leds->gpioDigitalWrite(8, alt.isValid);

		bool sd_status = logger.get_sd_status();
		external_leds->gpioDigitalWrite(9, !sd_status);

		if (sd_status != 0)
		{
			// try to reinit sd card
			flash.init(rtc.dateString(rtc.getTime()), rtc.timeString(rtc.getTime()));
		}

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
		state_cnt = 0;
	}
	else
	{
		Serial.println("TooSoon");
	}
}

void Flight::prevPhase()
{
	flight.phase--;
	state_cnt = 0;
	Serial.print("\n Advancing to Phase: ");
	Serial.println(flight.phase);
}

void Flight::setPhase(int phase)
{
	flight.phase = phase;
	state_cnt = 0;
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
	flight.valvesOpened = EEPROM.read(sizeof(int) + sizeof(bool) * 4);
	flight.valvesClosed = EEPROM.read(sizeof(int) + sizeof(bool) * 5);

	flight.last_uplink_timestamp = 0;

	flight.last_uplink_timestamp |= (uint32_t)EEPROM.read(sizeof(int) + sizeof(bool) * 6);
	flight.last_uplink_timestamp |= (uint32_t)((uint32_t)EEPROM.read(sizeof(int) + sizeof(bool) * 6 + 1) << 8LU);
	flight.last_uplink_timestamp |= (uint32_t)((uint32_t)EEPROM.read(sizeof(int) + sizeof(bool) * 6 + 2) << 16LU);
	flight.last_uplink_timestamp |= (uint32_t)((uint32_t)EEPROM.read(sizeof(int) + sizeof(bool) * 6 + 3) << 24LU);
}

void Flight::saveFlightToEEPROM()
{
	EEPROM.update(0, flight.phase);
	EEPROM.update(sizeof(int), flight.ground);
	EEPROM.update(sizeof(int) + sizeof(bool), flight.inFlight);
	EEPROM.update(sizeof(int) + sizeof(bool) * 2, flight.sampling);
	EEPROM.update(sizeof(int) + sizeof(bool) * 3, flight.finished);
	EEPROM.update(sizeof(int) + sizeof(bool) * 4, flight.valvesOpened);
	EEPROM.update(sizeof(int) + sizeof(bool) * 5, flight.valvesClosed);

	EEPROM.update(sizeof(int) + sizeof(bool) * 6, (uint8_t)flight.last_uplink_timestamp);
	EEPROM.update(sizeof(int) + sizeof(bool) * 6 + 1, (uint8_t)(flight.last_uplink_timestamp >> 8LU));
	EEPROM.update(sizeof(int) + sizeof(bool) * 6 + 2, (uint8_t)(flight.last_uplink_timestamp >> 16LU));
	EEPROM.update(sizeof(int) + sizeof(bool) * 6 + 3, (uint8_t)(flight.last_uplink_timestamp >> 24LU));
}