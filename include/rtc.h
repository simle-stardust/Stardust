#pragma once

#include <RtcDS3231.h>
#include <Wire.h>

class MyRTC
{
private:
	RtcDS3231<TwoWire> *RTC;

public:
	void init();

	void printDateTime(const RtcDateTime &dt);

	int getStatus();

	RtcDateTime getTime();

	RtcTemperature getTemp();
};
