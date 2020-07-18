#include "sensors.h"

MySensors::MySensors() : onewire_main(ONE_WIRE_main), onewire_sens(ONE_WIRE_sens), dht_main(DHT22_main), dht_sens(DHT22_sens), pressure_main(PRESSURE_SS_main, 0, 15.0), pressure_sens(PRESSURE_SS_sens, 0, 15.0)
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
	Serial3.println("@MarcinGetWysokosc!");
	altitude.ret = Serial3.readBytes(&altitude.buf[0], 6);
	Serial3.flush();

	if (altitude.ret == 6 && altitude.buf[0] == '@' && altitude.buf[5] == '!')
	{
		uint32_t new_altitude = (uint32_t)(((uint32_t)altitude.buf[1] << 24) + ((uint32_t)altitude.buf[2] << 16) + ((uint32_t)altitude.buf[3] << 8) + (altitude.buf[4]));
		// Update value and array, only if new, and within bounds
		if (new_altitude != altitude.value && new_altitude >= 0 && new_altitude < 40000)
		{
			altitude.value = new_altitude;

			// add value to array in order to calculate avg. alt.
			altitude.history[altitude.pointer] = altitude.value;
			altitude.pointer++;
			if (altitude.pointer > (altitude.measurment_num - 1) || altitude.pointer < 0)
			{
				altitude.pointer = 0;
			}

			altitude.average = 0;
			int avg_num = 0;
			for (uint8_t i = 0; i < altitude.measurment_num; i++)
			{
				if (altitude.history[i] != 0)
				{
					altitude.average += altitude.history[i];
					avg_num++;
				}
			}
			altitude.average /= avg_num;

			if (millis() - altitude.timestamp < 60000 && altitude.timestamp != 0 && avg_num > 5)
				altitude.isValid = true;

			altitude.ret = 0;
			altitude.timestamp = millis();
		}
	}

	if (millis() - altitude.timestamp > 60000)
	{
		// altitude is stale when time since last unique acqusition is more than 60 * 1000 milliseconds.
		altitude.isValid = false;
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
	if (new_pressure != pressure_0->value && new_pressure > 0)
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