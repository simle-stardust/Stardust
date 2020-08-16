#include "logger.h"

void Logger::init(MySD *_flash, MySensors *_sensors, MyUDP *_udp)
{
	flash = _flash;
	sensors = _sensors;
	udp = _udp;

	//flash->writeLine("Date,Time,RTC_Temp,DHT_Humid,DHT_Temp,DS_Temp,Pressure,Temperature");
};

void Logger::tick(int phase, bool ground, bool inFlight, bool sampling, bool finished, reason_t reason)
{
	udp->tick();
	sensors->readSensors(phase);

	Serial.print("RTC:			");
		this->add(sensors->getDate());
		Serial.print(",	");
		this->add(sensors->getTime());
		Serial.print(",		");
		this->add(sensors->temperature(0));
		Serial.println(" *C;");

	Serial.print("Phase:		");
		this->add(phase);
		Serial.print("	ground: ");
		this->add(ground);
		Serial.print("	flight: ");
		this->add(inFlight);
		Serial.print("	sampling: ");
		this->add(sampling);
		Serial.print("	finished: ");
		this->add(finished);
		Serial.println();

	Serial.print("Altitude [GPS]:		");

		struct MyAltitude altitude = sensors->altitude(0);

		this->add(altitude.value);
		Serial.print("m		[avg:");
		this->add(altitude.average);
		Serial.print("m]				true:");
		this->add(altitude.isValid);
		Serial.println(";");

	Serial.print("Altitude [Pressure#1]:	");

		altitude = sensors->altitude(1);

		this->add(altitude.value);
		Serial.print("m		[avg:");
		this->add(altitude.average);
		Serial.print("m]				true:");
		this->add(altitude.isValid);
		Serial.println(";");

	Serial.print("Altitude [Pressure#2]:	");

		altitude = sensors->altitude(2);

		this->add(altitude.value);
		Serial.print("m		[avg:");
		this->add(altitude.average);
		Serial.print("m]				true:");
		this->add(altitude.isValid);
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

	Serial.print("DHT22 main:		");
		this->add(sensors->humidity(3));
		Serial.print(" RH%,				");
		this->add(sensors->temperature(3));
		Serial.println(" *C;");

	Serial.print("DHT22 sens:		");
		this->add(sensors->humidity(4));
		Serial.print(" RH%,				");
		this->add(sensors->temperature(4));
		Serial.println(" *C;");

	Serial.print("GPS status from main:		");
		this->add(sensors->getGPSStatus());
		Serial.println();
	
	Serial.print("Stardust status from main:   ");
		this->add(sensors->getStardustFlightStatus());
		Serial.println();

	Serial.print("LoRa status from main:    ");
		this->add(sensors->getLoRaStatus());
		Serial.println();
	
	Serial.print("Liftoff status from main:  ");
		this->add(sensors->getLiftoffStatus());
		Serial.println();

	Serial.print("Reason of change:   ");
		this->add(reason);
		Serial.println();

	// // TODO: Why you no work ? :(
	// temp_main.update();
	// for (int i = 0; i < temp_main.getNumberOfDevices(); i++)
	// {
	// 	Serial.print("DS18B20 main [");
	// 	Serial.print(i);
	// 	Serial.print("]:				");
	// 	this->add(temp_main.getTemperature(i));
	// 	Serial.println(" *C;");
	// }

	// temp_sens.update();
	// for (int i = 0; i < temp_sens.getNumberOfDevices(); i++)
	// {
	// 	Serial.print("DS18B20 sens [");
	// 	Serial.print(i);
	// 	Serial.print("]:				");
	// 	this->add(temp_sens.getTemperature(i));
	// 	Serial.println(" *C;");
	// }
	this->save();
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
	udp->writeLine(log);
	log = "";
}