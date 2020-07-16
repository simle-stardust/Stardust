#include "sensors.h"

MySensors::MySensors() : onewire_main(ONE_WIRE_main), onewire_sens(ONE_WIRE_sens), dht_main(DHT22_main), dht_sens(DHT22_sens), pressure_main(PRESSURE_SS_main, -15.0, 15.0), pressure_sens(PRESSURE_SS_sens, -15.0, 15.0)
{
}

void MySensors::init(MyRTC *_rtc, MySD *_flash)
{
	rtc = _rtc;
	flash = _flash;
	temp_main.init(onewire_main);
	temp_sens.init(onewire_sens);
	pressure_main.begin();
	pressure_sens.begin();
	logger.init(flash);

	lastOperation = millis();
}

void MySensors::tick()
{
	if (millis() - lastOperation > SAMPLING_TIME)
	{
		rtc->getStatus();
		Serial.print("RTC:		");
		logger.add(rtc->dateString(rtc->getTime()));
		Serial.print(",");
		logger.add(rtc->timeString(rtc->getTime()));
		Serial.print(",	");
		logger.add(rtc->getTemp().AsFloatDegC());
		Serial.println(" *C;");

		Serial.print("DHT22 main:	");
		logger.add(dht_main.getHumidity());
		Serial.print(" RH%,		");

		logger.add(dht_main.getTemperature());
		Serial.println(" *C;");

		Serial.print("DHT22 sens:	");
		logger.add(dht_sens.getHumidity());
		Serial.print(" RH%,		");

		logger.add(dht_sens.getTemperature());
		Serial.println(" *C;");

		pressure_main.readSensor();
		Serial.print("Pressure main:	");
		logger.add(pressure_main.pressure());
		Serial.print(" psi,		");

		logger.add(pressure_main.temperature());
		Serial.println(" *C;");

		pressure_sens.readSensor();
		Serial.print("Pressure sens:	");
		logger.add(pressure_sens.pressure());
		Serial.print(" psi,		");

		logger.add(pressure_sens.temperature());
		Serial.println(" *C;");

		// // TODO: Why you no work ? :(
		// temp_main.update();
		// for (int i = 0; i < temp_main.getNumberOfDevices(); i++)
		// {
		// 	Serial.print("DS18B20 main [");
		// 	Serial.print(i);
		// 	Serial.print("]:				");
		// 	logger.add(temp_main.getTemperature(i));
		// 	Serial.println(" *C;");
		// }

		// temp_sens.update();
		// for (int i = 0; i < temp_sens.getNumberOfDevices(); i++)
		// {
		// 	Serial.print("DS18B20 sens [");
		// 	Serial.print(i);
		// 	Serial.print("]:				");
		// 	logger.add(temp_sens.getTemperature(i));
		// 	Serial.println(" *C;");
		// }
		logger.save();
		lastOperation = millis();
	}
}