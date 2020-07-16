#pragma once
#include <RtcDS3231.h>
#include <Wire.h>

class MyRTC
{
private:
	RtcDS3231<TwoWire> *RTC;

public:
	void init();

	String dateString(const RtcDateTime &dt);
	String timeString(const RtcDateTime &dt);

	int getStatus();

	RtcDateTime getTime();

	RtcTemperature getTemp();

	unsigned int getTimestamp();
};
