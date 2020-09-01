#include "sensors.h"

//#define DUPA

MySensors::MySensors() : onewire_main(ONE_WIRE_main), onewire_sens(ONE_WIRE_sens), dht_main(DHT22_main), dht_sens(DHT22_sens), dht_mech(DHT22_mech), pressure_main(PRESSURE_SS_main, 0, 15.0), pressure_sens(PRESSURE_SS_sens, 0, 15.0)
{
}

void MySensors::init(MyRTC *_rtc)
{
	rtc = _rtc;
	temp_main.init(onewire_main);
	temp_sens.init(onewire_sens);
	pressure_main.begin();
	pressure_sens.begin();
}

void MySensors::readSensors()
{
	rtc->getStatus();

	// Temperature 
	temp_sens.update();
	temp_main.update();

	// Pressure
	getPressure(1);
	getPressure(2);

	// Altitude
	getAltitude();
}

RtcDateTime MySensors::time()
{
	return rtc->getTime();
}

String MySensors::getDate()
{
	return rtc->dateString(rtc->getTime());
}

String MySensors::getTime()
{
	return rtc->timeString(rtc->getTime());
}

void MySensors::getAltitude()
{
	// Pressure Based [1, 2]

	altitude_1.value = pressureToAltitude(LIFTOFF_PRESSURE, pressure_1.value, temperature(1));
	altitude_1.average = pressureToAltitude(LIFTOFF_PRESSURE, pressure_1.average, temperature(1));
	altitude_1.isValid = pressure_1.isValid;
	altitude_1.timestamp = pressure_1.timestamp;

	altitude_2.value = pressureToAltitude(LIFTOFF_PRESSURE, pressure_2.value, temperature(2));
	altitude_2.average = pressureToAltitude(LIFTOFF_PRESSURE, pressure_2.average, temperature(2));
	altitude_2.isValid = pressure_2.isValid;
	altitude_2.timestamp = pressure_2.timestamp;

	// GPS Based [0]

	if (millis() - altitude_0.timestamp >= 5000 )
	{
		// TODO: Implement VA GPS
		Serial3.print("@MarcinGetWysokosc!");
		altitude_0.ret = Serial3.readBytes(&altitude_0.buf[0], 8);
		Serial3.flush();

		if (altitude_0.ret == 8 && altitude_0.buf[0] == '@' && altitude_0.buf[7] == '!')
		{
			int32_t new_altitude = (int32_t)(((uint32_t)altitude_0.buf[1] << 24) + ((uint32_t)altitude_0.buf[2] << 16) + ((uint32_t)altitude_0.buf[3] << 8) + (altitude_0.buf[4]));
			
			
			// Update value and array, only if new, and within bounds
			if (new_altitude != altitude_0.value && new_altitude >= 0 && new_altitude < 40000)
			{
				altitude_0.value = new_altitude;

				// add value to array in order to calculate avg. alt.
				altitude_0.history[altitude_0.pointer] = altitude_0.value;
				altitude_0.pointer++;
				if (altitude_0.pointer > (altitude_0.measurment_num - 1) || altitude_0.pointer < 0)
				{
					altitude_0.pointer = 0;
				}

				altitude_0.average = 0;
				int avg_num = 0;
				for (uint8_t i = 0; i < altitude_0.measurment_num; i++)
				{
					if (altitude_0.history[i] != 0)
					{
						altitude_0.average += altitude_0.history[i];
						avg_num++;
					}
				}
				altitude_0.average /= avg_num;

				if (millis() - altitude_0.timestamp < 60000 && altitude_0.timestamp != 0 && avg_num > 5)
					altitude_0.isValid = true;

				altitude_0.ret = 0;
				altitude_0.timestamp = millis();
			}
		}
	}

	if (millis() - altitude_0.timestamp > 60000)
	{
		// altitude is stale when time since last unique acqusition is more than 60 * 1000 milliseconds.
		altitude_0.isValid = false;
	}
}

void MySensors::getPressure(int sensor = 0)
{
	TruStabilityPressureSensor *pressure_active;
	struct MyPressure *pressure_0;
	switch (sensor)
	{
	case 1:
		pressure_main.readSensor();
		pressure_active = &pressure_main;
		pressure_0 = &pressure_1;
		break;
	case 2:
		pressure_sens.readSensor();
		pressure_active = &pressure_sens;
		pressure_0 = &pressure_2;
		break;
	default:
		return;
	}

	float new_pressure = pressure_active->pressure() * 68.9476;
	if (new_pressure != pressure_0->value && new_pressure > 0 && pressure_active->status() == 0)
	{
		pressure_0->value = new_pressure;

		// add value to array in order to calculate avg. alt.
		pressure_0->history[pressure_0->pointer] = pressure_0->value;
		pressure_0->pointer++;
		if (pressure_0->pointer > (pressure_0->measurment_num - 1) || pressure_0->pointer < 0)
		{
			pressure_0->pointer = 0;
		}

		pressure_0->average = 0;
		int avg_num = 0;
		for (uint8_t i = 0; i < pressure_0->measurment_num; i++)
		{
			if (pressure_0->history[i] != 0)
			{
				pressure_0->average += pressure_0->history[i];
				avg_num++;
			}
		}
		pressure_0->average /= avg_num;

		if (millis() - pressure_0->timestamp < 60000 && pressure_0->timestamp != 0 && avg_num > 5)
			pressure_0->isValid = true;

		pressure_0->timestamp = millis();
	}
}

struct MyAltitude MySensors::altitude(int sensor = 0) {
	switch (sensor)
	{
	case 0:
		return altitude_0;
	case 1:
		return altitude_1;
	case 2:
		return altitude_2;
	default:
		return {};
	}
}

struct MyPressure MySensors::pressure(int sensor = 0)
{
	switch (sensor)
	{
	case 1:
		return pressure_1;
	case 2:
		return pressure_2;
	default:
		return {};
	}
};

float MySensors::temperature(int sensor = 0)
{
	switch (sensor)
	{
	case 0:
		return rtc->getTemp().AsFloatDegC();
	case 1:
		pressure_main.readSensor();
		return pressure_main.temperature();
	case 2:
		pressure_sens.readSensor();
		return pressure_sens.temperature();
	case 3:
		return dht_main.getTemperature();
	case 4:
		return dht_sens.getTemperature();
	case 5:
		return dht_mech.getTemperature();
	case 10 ... 19:
		return temp_main.getTemperature(sensor - 10);
	case 20 ... 29:
		return temp_sens.getTemperature(sensor - 20);
	default:
		return -1;
	}
};

float MySensors::humidity(int sensor = 0)
{
	switch (sensor)
	{
	case 3:
		return dht_main.getHumidity();
	case 4:
		return dht_sens.getHumidity();
	case 5:
		return dht_mech.getHumidity();
	default:
		return -1;
	}
};

/*!
 *   @brief  Calculates the altitude (in meters) from the specified atmospheric
 *           pressure (in hPa), sea-level pressure (in hPa), and temperature (in
 * �C)
 *   @param  seaLevel      Sea-level pressure in hPa
 *   @param  atmospheric   Atmospheric pressure in hPa
 *   @param  temp          Temperature in degrees Celsius
 *   @return               Altitude value in meters
 */
float MySensors::pressureToAltitude(float seaLevel = 1013.0, float atmospheric = 1013.0, float temp = 25.0)
{
	/* Hyposometric formula:                      */
	/*                                            */
	/*     ((P0/P)^(1/5.257) - 1) * (T + 273.15)  */
	/* h = -------------------------------------  */
	/*                   0.0065                   */
	/*                                            */
	/* where: h   = height (in meters)            */
	/*        P0  = sea-level pressure (in hPa)   */
	/*        P   = atmospheric pressure (in hPa) */
	/*        T   = temperature (in *C)           */
	/* 	*68.9476 <- converts psi to hPa		   	*/
	return (((float)pow((seaLevel / atmospheric), 0.190223F) - 1.0F) *
			(temp + 273.15F)) /
		   0.0065F;
}