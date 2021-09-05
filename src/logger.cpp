#include "logger.h"

#define LOGGER_DEBUG

void Logger::init(MySD *_flash, MySensors *_sensors, MyUDP *_udp, MyADC*_adc, MyServo *_servos, MyPWMs * _pwms)
{
	flash = _flash;
	sensors = _sensors;
	udp = _udp;
	adc = _adc;
	servos = _servos;
	pwms = _pwms;

	//flash->writeLine("Date,Time,RTC_Temp,DHT_Humid,DHT_Temp,DS_Temp,Pressure,Temperature");
};

void Logger::tick(int phase, bool ground, bool inFlight, bool sampling, bool finished, reason_t reason)
{
	String str_from_udp;

	sensors->readSensors();

	log_udp_len = 0;
	RtcDateTime date = sensors->time();

#ifdef LOGGER_DEBUG
	Serial.print("RTC:			");
#endif

	this->add(sensors->getDate());
	this->add_udp(date.Year());
	this->add_udp(date.Month());
	this->add_udp(date.Day());
	this->add_udp(date.Hour());
	this->add_udp(date.Minute());
	this->add_udp(date.Second());

#ifdef LOGGER_DEBUG
	Serial.print(",	");
#endif

	this->add(sensors->getTime());

#ifdef LOGGER_DEBUG
	Serial.print(",		");
#endif

	this->add(sensors->temperature(0));
	this->add_udp(sensors->temperature(0));

#ifdef LOGGER_DEBUG
	Serial.print(" *C, errcode: ");
#endif

	this->add(sensors->isDateTimeValid());
	this->add_udp(sensors->isDateTimeValid());

#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("Pressure [main]:	");
#endif

	struct MyPressure pressure = sensors->pressure(1);
	this->add(pressure.value);
	this->add_udp(pressure.value);

#ifdef LOGGER_DEBUG
	Serial.print("hpa	[avg:");
#endif

	this->add(pressure.average);
	this->add_udp(pressure.average);

#ifdef LOGGER_DEBUG
	Serial.print("hpa]	");
#endif

	this->add(sensors->temperature(1));
	this->add_udp(sensors->temperature(1));

#ifdef LOGGER_DEBUG
	Serial.print(" *C	true:");
#endif

	this->add(pressure.isValid);
	this->add_udp((uint8_t)pressure.isValid);

#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("Pressure [sens]:	");
#endif

	pressure = sensors->pressure(2);
	this->add(pressure.value);
	this->add_udp(pressure.value);

#ifdef LOGGER_DEBUG
	Serial.print("hpa	[avg:");
#endif

	this->add(pressure.average);
	this->add_udp(pressure.average);

#ifdef LOGGER_DEBUG
	Serial.print("hpa]	");
#endif

	this->add(sensors->temperature(2));
	this->add_udp(sensors->temperature(2));

#ifdef LOGGER_DEBUG
	Serial.print(" *C	true:");
#endif

	this->add(pressure.isValid);
	this->add_udp((uint8_t)pressure.isValid);

#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("Altitude [GPS]:		");
#endif

	struct MyAltitude altitude = sensors->altitude(0);
	this->add(altitude.value);
	this->add_udp(altitude.value);

#ifdef LOGGER_DEBUG
	Serial.print("m		[avg:");
#endif

	this->add(altitude.average);
	this->add_udp(altitude.average);

#ifdef LOGGER_DEBUG
	Serial.print("m]	true:");
#endif

	this->add(altitude.isValid);
	this->add_udp((uint8_t)altitude.isValid);
		
#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("Altitude [press_sens]:	");
#endif

	altitude = sensors->altitude(2);
	this->add(altitude.value);
	this->add_udp(altitude.value);

#ifdef LOGGER_DEBUG
	Serial.print("m		[avg:");
#endif

	this->add(altitude.average);
	this->add_udp(altitude.average);
	
#ifdef LOGGER_DEBUG
	Serial.print("m]	true:");
#endif

	this->add(altitude.isValid);
	this->add_udp((uint8_t)altitude.isValid);
	
#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("Altitude [press_main]:	");
#endif

	altitude = sensors->altitude(1);
	this->add(altitude.value);
	this->add_udp(altitude.value);
	
#ifdef LOGGER_DEBUG
	Serial.print("m		[avg:");
#endif

	this->add(altitude.average);
	this->add_udp(altitude.average);
	
#ifdef LOGGER_DEBUG
	Serial.print("m]	true:");
#endif

	this->add(altitude.isValid);
	this->add_udp((uint8_t)altitude.isValid);
	
#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("DHT22 main:		");
#endif

	this->add(sensors->temperature(3));
	this->add_udp(sensors->temperature(3));
	
#ifdef LOGGER_DEBUG
	Serial.print(" *C,	");
#endif

	this->add(sensors->humidity(3));
	this->add_udp(sensors->humidity(3));
	
#ifdef LOGGER_DEBUG
	Serial.print("RH%,	errCode: ");
#endif

	this->add(sensors->temperatureStatus(3));
	this->add_udp(sensors->temperatureStatus(3));
	
#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("DHT22 sens:		");
#endif

	this->add(sensors->temperature(4));
	this->add_udp(sensors->temperature(4));
	
#ifdef LOGGER_DEBUG
	Serial.print(" *C,	");
#endif

	this->add(sensors->humidity(4));
	this->add_udp(sensors->humidity(4));
	
#ifdef LOGGER_DEBUG
	Serial.print(" RH%,	errCode: ");
#endif

	this->add(sensors->temperatureStatus(4));
	this->add_udp(sensors->temperatureStatus(4));
	
#ifdef LOGGER_DEBUG
	Serial.println(";");
	Serial.print("DHT22 mech:		");
#endif

	this->add(sensors->temperature(5));
	this->add_udp(sensors->temperature(5));
	
#ifdef LOGGER_DEBUG
	Serial.print(" *C,	");
#endif

	this->add(sensors->humidity(5));
	this->add_udp(sensors->humidity(5));
	
#ifdef LOGGER_DEBUG
	Serial.print(" RH%,	errCode: ");
#endif

	this->add(sensors->temperatureStatus(5));
	this->add_udp(sensors->temperatureStatus(5));
	
#ifdef LOGGER_DEBUG
	Serial.println(";");
#endif


	for (uint8_t i = 10; i < 20; i++)
	{
	
#ifdef LOGGER_DEBUG
		Serial.print("DS18B20 main["); 
		Serial.print(i - 10);
		Serial.print("]:				");
#endif

		this->add(sensors->temperature(i));
		this->add_udp(sensors->temperature(i));
	
#ifdef LOGGER_DEBUG
		Serial.println(" *C;");
#endif

	}

	for (uint8_t i = 20; i < 30; i++)
	{
	
#ifdef LOGGER_DEBUG
		Serial.print("DS18B20 sens["); 
		Serial.print(i - 20);
		Serial.print("]:				");
#endif

		this->add(sensors->temperature(i));
		this->add_udp(sensors->temperature(i));
	
#ifdef LOGGER_DEBUG
		Serial.println(" *C;");
#endif

	}

	
#ifdef LOGGER_DEBUG
	Serial.print("Pump 1: ");
#endif

	this->add(pwms->read(PWM_PUMP_1));
	this->add_udp(pwms->read(PWM_PUMP_1));
	
#ifdef LOGGER_DEBUG
	Serial.print(",	Pump 2: ");
#endif

	this->add(pwms->read(PWM_PUMP_2));
	this->add_udp(pwms->read(PWM_PUMP_2));
	
#ifdef LOGGER_DEBUG
	Serial.print(",	Heaters: ");
#endif

	this->add(pwms->read(PWM_HEATING));
	this->add_udp(pwms->read(PWM_HEATING));
	
#ifdef LOGGER_DEBUG
	Serial.print(",	Phase:		");
#endif

	this->add(phase);
	this->add_udp(phase);
	
#ifdef LOGGER_DEBUG
	Serial.println(";");
#endif


	for (uint8_t i = 0; i < NB_OF_SERVOS; i++)
	{
	
#ifdef LOGGER_DEBUG
		Serial.print("Servo ");
		Serial.print(i + 1);
		Serial.print(": ");
#endif

		this->add(servos->getStatus(i + 1));
		this->add_udp((uint8_t)servos->getStatus(i + 1));
	
#ifdef LOGGER_DEBUG
		Serial.print(",  ");
#endif

	}
	
#ifdef LOGGER_DEBUG
	Serial.println("");
	Serial.print("ADC [1]:	");
#endif

	this->add(adc->getADC(0));
	this->add_udp(adc->getADC(0));
	
#ifdef LOGGER_DEBUG
	Serial.println();
	Serial.print("ADC [2]:	");
#endif

	this->add(adc->getADC(1));
	this->add_udp(adc->getADC(1));
	
#ifdef LOGGER_DEBUG
	Serial.println();
	Serial.print("ADC [3]:	");
#endif

	this->add(adc->getADC(2));
	this->add_udp(adc->getADC(2));
	
#ifdef LOGGER_DEBUG
	Serial.println();
	Serial.print("ADC [4]:	");
#endif

	this->add(adc->getADC(3));
	this->add_udp(adc->getADC(3));
	
#ifdef LOGGER_DEBUG
	Serial.println();
	Serial.print("Reason of change:   ");
#endif

	this->add(reason);
	
#ifdef LOGGER_DEBUG
	Serial.println();
	Serial.print("Time since last ping: ");
#endif

	this->add(udp->timeSinceLastPing());
	this->add_udp(udp->timeSinceLastPing());
	
#ifdef LOGGER_DEBUG
	Serial.println();
#endif

	this->save();

	str_from_udp = udp->tick();

	if (str_from_udp != "")
	{
		this->write_to_sd(str_from_udp);
	}
};

void Logger::add(String value)
{
#ifdef LOGGER_DEBUG
	Serial.print(value);
#endif
	log += value + ",";
}

void Logger::add(float value)
{
#ifdef LOGGER_DEBUG
	Serial.print(value);
#endif
	log += String(value) + ",";
}

void Logger::add(uint32_t value)
{
#ifdef LOGGER_DEBUG
	Serial.print(value);
#endif
	log += String(value) + ",";
}

void Logger::add(int value)
{
#ifdef LOGGER_DEBUG
	Serial.print(value);
#endif
	log += String(value) + ",";
}

void Logger::add(int32_t value)
{
#ifdef LOGGER_DEBUG
	Serial.print(value);
#endif
	log += String(value) + ",";
}

void Logger::add_udp(uint8_t value)
{
	log_udp[log_udp_len++] = value;
}

void Logger::add_udp(uint16_t value)
{
	log_udp[log_udp_len++] = (uint8_t)value;
	log_udp[log_udp_len++] = (uint8_t)(value >> 8);
}

void Logger::add_udp(uint32_t value)
{
	log_udp[log_udp_len++] = (uint8_t)value;
	log_udp[log_udp_len++] = (uint8_t)(value >> 8);
	log_udp[log_udp_len++] = (uint8_t)(value >> 16);
	log_udp[log_udp_len++] = (uint8_t)(value >> 24);
}

void Logger::add_udp(int8_t value)
{
	log_udp[log_udp_len++] = (uint8_t)value;
}

void Logger::add_udp(int16_t value)
{
	log_udp[log_udp_len++] = (uint8_t)value;
	log_udp[log_udp_len++] = (uint8_t)((uint8_t)value >> 8);
}

void Logger::add_udp(int32_t value)
{
	log_udp[log_udp_len++] = (uint8_t)value;
	log_udp[log_udp_len++] = (uint8_t)((uint32_t)value >> 8);
	log_udp[log_udp_len++] = (uint8_t)((uint32_t)value >> 16);
	log_udp[log_udp_len++] = (uint8_t)((uint32_t)value >> 24);
}

void Logger::add_udp(float value)
{
	int32_t val_multiplied = (int32_t)(value * 100.0f);
	log_udp[log_udp_len++] = (uint8_t)val_multiplied;
	log_udp[log_udp_len++] = (uint8_t)((uint32_t)val_multiplied >> 8);
	log_udp[log_udp_len++] = (uint8_t)((uint32_t)val_multiplied >> 16);
	log_udp[log_udp_len++] = (uint8_t)((uint32_t)val_multiplied >> 24);
}

void Logger::add(reason_t value)
{
	switch (value)
	{
		case REASON_PRESSURE1:
#ifdef LOGGER_DEBUG
			Serial.print("PRESSURE1");
#endif
			log += "PRESSURE1,";
			break;
		case REASON_PRESSURE2:
#ifdef LOGGER_DEBUG
			Serial.print("PRESSURE2");
#endif
			log += "PRESSURE2,";
			break;
		case REASON_ALTITUDE:
#ifdef LOGGER_DEBUG
			Serial.print("ALTITUDE");
#endif
			log += "ALTITUDE,";
			break;
		case REASON_LORA:
#ifdef LOGGER_DEBUG
			Serial.print("LORA");
#endif
			log += "LORA,";
			break;
		case REASON_UDP:
#ifdef LOGGER_DEBUG
			Serial.print("UDP");
#endif
			log += "UDP,";
			break;
		default:
#ifdef LOGGER_DEBUG
			Serial.print("UNKNOWN");
#endif
			log += "UNKNOWN,";
			break;
	}
}

void Logger::save()
{
	log += "\n";

	//Serial.println("UDP_LEN 666 = " + String(log_udp_len));
	flash->writeLine(log);
	udp->writeLine(&log_udp[0], log_udp_len);

	log = "";
}

void Logger::write_to_sd(String value)
{
	flash->writeLine(value);
}