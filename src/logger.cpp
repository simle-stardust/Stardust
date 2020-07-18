#include "logger.h"

void Logger::init(MySD *_flash, MySensors *_sensors)
{
	flash = _flash;
	sensors = _sensors;
	//flash->writeLine("Date,Time,RTC_Temp,DHT_Humid,DHT_Temp,DS_Temp,Pressure,Temperature");
};

void Logger::tick(int phase)
{
	if (millis() - lastOperation > SAMPLING_TIME)
	{

		sensors->readSensors();

		Serial.print("Phase:		");
			this->add(phase);
			Serial.println();

		Serial.print("RTC:			");
			this->add(sensors->getDate());
			Serial.print(",	");
			this->add(sensors->getTime());
			Serial.print(",		");
			this->add(sensors->temperature(0));
			Serial.println(" *C;");

		Serial.print("Altitude [GPS]:		");
			struct serial_altitude altitude = sensors->altitude;
			this->add(altitude.value);
			Serial.print("m		[avg:");
			this->add(altitude.average);
			Serial.print("m]				true:");
			this->add(altitude.isValid);
			Serial.println(";");

		Serial.print("Altitude [Pressure#1]:	");
			struct MyPressure pressure = sensors->pressure(1);
			this->add(sensors->pressureToAltitude(1015.0, pressure.value, sensors->temperature(1)));
			Serial.print("m		[avg:");
			this->add(sensors->pressureToAltitude(1015.0, pressure.average, sensors->temperature(1)));
			Serial.print("m]				true:");
			this->add(pressure.isValid);
			Serial.println(";");

		Serial.print("Altitude [Pressure#2]:	");
			pressure = sensors->pressure(2);
			this->add(sensors->pressureToAltitude(1015.0, pressure.value, sensors->temperature(2)));
			Serial.print("m		[avg:");
			this->add(sensors->pressureToAltitude(1015.0, pressure.average, sensors->temperature(2)));
			Serial.print("m]				true:");
			this->add(pressure.isValid);
			Serial.println(";");

		Serial.print("Pressure [main]:	");
			pressure = sensors->pressure(1);
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
		lastOperation = millis();
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

void Logger::add(int value)
{
	Serial.print(value);
	log += String(value) + ",";
}

void Logger::add(uint32_t value)
{
	Serial.print(value);
	log += String(value) + ",";
}

void Logger::save()
{
	log += "\n";
	flash->writeLine(log);
	log = "";
}