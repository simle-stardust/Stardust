#include "logger.h"

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

	Serial.print("RTC:			");
		this->add(sensors->getDate());
		Serial.print(",	");
		this->add(sensors->getTime());
		Serial.print(",		");
		this->add(sensors->temperature(0));
		Serial.print(" *C, errcode: ");
		this->add(sensors->isDateTimeValid());
		Serial.println(";");

	Serial.print("Pressure [main]:	");
		struct MyPressure pressure = sensors->pressure(1);
		this->add(pressure.value);
		Serial.print("hpa	[avg:");
		this->add(pressure.average);
		Serial.print("hpa]	");
		this->add(sensors->temperature(1));
		Serial.print(" *C	true:");
		this->add(pressure.isValid);
		Serial.println(";");

	Serial.print("Pressure [sens]:	");
		pressure = sensors->pressure(2);
		this->add(pressure.value);
		Serial.print("hpa	[avg:");
		this->add(pressure.average);
		Serial.print("hpa]	");
		this->add(sensors->temperature(2));
		Serial.print(" *C	true:");
		this->add(pressure.isValid);
		Serial.println(";");

	Serial.print("Altitude [GPS]:		");
		struct MyAltitude altitude = sensors->altitude(0);
		this->add(altitude.value);
		Serial.print("m		[avg:");
		this->add(altitude.average);
		Serial.print("m]	true:");
		this->add(altitude.isValid);
		Serial.println(";");

	Serial.print("Altitude [press_sens]:	");
		altitude = sensors->altitude(2);
		this->add(altitude.value);
		Serial.print("m		[avg:");
		this->add(altitude.average);
		Serial.print("m]	true:");
		this->add(altitude.isValid);
		Serial.println(";");

	Serial.print("Altitude [press_main]:	");
		altitude = sensors->altitude(1);
		this->add(altitude.value);
		Serial.print("m		[avg:");
		this->add(altitude.average);
		Serial.print("m]	true:");
		this->add(altitude.isValid);
		Serial.println(";");

	Serial.print("DHT22 main:		");
		this->add(sensors->temperature(3));
		Serial.print(" *C,	");
		this->add(sensors->humidity(3));
		Serial.print("RH%,	errCode: ");
		this->add(sensors->temperatureStatus(3));
		Serial.println(";");

	Serial.print("DHT22 sens:		");
		this->add(sensors->temperature(4));
		Serial.print(" *C,	");
		this->add(sensors->humidity(4));
		Serial.print(" RH%,	errCode: ");
		this->add(sensors->temperatureStatus(4));
		Serial.println(";");

	// Serial.print("DHT22 mech:		");
	//	this->add(sensors->temperature(5));
	//	Serial.print(" *C,	");
	//	this->add(sensors->humidity(5));
	//	Serial.print(" RH%,	errCode: ");
	//	this->add(sensors->temperatureStatus(5));
	//	Serial.println(";");

	for (uint8_t i = 10; i < 20; i++)
	{
		//  Serial.print("DS18B20 main["); 
		//  Serial.print(i - 10);
		//  Serial.print("]:				");
		// 	this->add(sensors->temperature(i));
		// 	Serial.println(" *C;");
	}

	for (uint8_t i = 20; i < 30; i++)
	{
		//  Serial.print("DS18B20 sens["); 
		//  Serial.print(i - 20);
		//  Serial.print("]:				");
		// 	this->add(sensors->temperature(i));
		// 	Serial.println(" *C;");
	}

	Serial.print("Pump 1: ");
	this->add(pwms->read(PWM_PUMP_1));
	Serial.print(",	Pump 2: ");
	this->add(pwms->read(PWM_PUMP_2));
	Serial.print(",	Heaters: ");
	this->add(pwms->read(PWM_HEATING));
	Serial.print(",	Phase:		");
	this->add(phase);
	Serial.println(";");

	for (uint8_t i = 0; i < NB_OF_SERVOS; i++)
	{
		Serial.print("Servo ");
		Serial.print(i);
		Serial.print(": ");

		this->add(servos->getStatus(i + 1));
		Serial.print(",  ");
	}
	Serial.println("");

	Serial.print("ADC [1]:	");
		this->add(adc->getADC(0));
		Serial.println();
	Serial.print("ADC [2]:	");
		this->add(adc->getADC(1));
		Serial.println();
	Serial.print("ADC [3]:	");
		this->add(adc->getADC(2));
		Serial.println();
	
	// Current SED has only 3 values (2 x pumps and heating system)
	//Serial.print("ADC [4]:	");
	//	this->add(adc->getADC(3));
	//	Serial.println();

	Serial.print("Reason of change:   ");
		this->add(reason);
		Serial.println();

	Serial.print("Time since last ping: ");
		this->add(udp->timeSinceLastPing());
		Serial.println();

	this->save();

	str_from_udp = udp->tick();

	if (str_from_udp != "")
	{
		this->write_to_sd(str_from_udp);
	}
};

void Logger::add(String value)
{
	Serial.print(value);
	log += value + ",";
}

void Logger::add(float value)
{
	Serial.print(value);
	log += String(value) + ",";
}

void Logger::add(uint32_t value)
{
	Serial.print(value);
	log += String(value) + ",";
}

void Logger::add(int value)
{
	Serial.print(value);
	log += String(value) + ",";
}

void Logger::add(int32_t value)
{
	Serial.print(value);
	log += String(value) + ",";
}

void Logger::add(reason_t value)
{
	switch (value)
	{
		case REASON_PRESSURE1:
			Serial.print("PRESSURE1");
			log += "PRESSURE1,";
			break;
		case REASON_PRESSURE2:
			Serial.print("PRESSURE2");
			log += "PRESSURE2,";
			break;
		case REASON_ALTITUDE:
			Serial.print("ALTITUDE");
			log += "ALTITUDE,";
			break;
		case REASON_LORA:
			Serial.print("LORA");
			log += "LORA,";
			break;
		case REASON_UDP:
			Serial.print("UDP");
			log += "UDP,";
			break;
		default:
			Serial.print("UNKNOWN");
			log += "UNKNOWN,";
			break;
	}
}

void Logger::save()
{
	log += "\n";
	flash->writeLine(log);
	// TODO: uncomment
	//udp->writeLine(log);
	log = "";
}

void Logger::write_to_sd(String value)
{
	flash->writeLine(value);
}