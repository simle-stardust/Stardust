#include <Arduino.h>
#include "logger.h"
#include "rtc.h"
#include "dht22.h"
#include "ds18b20.h"
#include <HoneywellTruStabilitySPI.h>

#define DHT22_main 33
#define DHT22_mech 27
#define DHT22_sens 41

#define ONE_WIRE_main 35
#define ONE_WIRE_mech 31
#define ONE_WIRE_sens 37

#define PRESSURE_SS_main 25
#define PRESSURE_SS_sens 39

#define SAMPLING_TIME 1000 // Czas próbkowania

class MySensors
{
private:
	unsigned long lastOperation = 0;

	String log = "";

	MySD *flash;
	MyRTC *rtc;

public:
	Logger logger;

	OneWire onewire_main;
	OneWire onewire_sens;

	MyDHT dht_main;
	MyDHT dht_sens;
	MyDS18B20 temp_main;
	MyDS18B20 temp_sens;
	TruStabilityPressureSensor pressure_main;
	TruStabilityPressureSensor pressure_sens;

	MySensors();

	void init(MyRTC *rtc, MySD *flash);

	void tick();
};
