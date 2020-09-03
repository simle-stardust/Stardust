#include <Arduino.h>
#include "director.h"

#define BUTTON_PIN 46

Flight flight;

struct debounce
{
	int state = 0;
	int lastState = 0;
	unsigned long time = 0;
};
struct debounce button;

unsigned long loopTimer;

void setup()
{
	pinMode(13, OUTPUT);
	pinMode(BUTTON_PIN, INPUT);

	Serial.begin(115200);
	Serial.print("compiled: ");
	Serial.println(__DATE__);
	Serial.println(__TIME__);

	flight.init();
}

void loop()
{

	loopTimer = millis();
	digitalWrite(13, HIGH);

	button.state = digitalRead(BUTTON_PIN);
	if (button.state != button.lastState)
	{
		if (button.state == LOW)
		{
			// clicked
			Serial.println("ON");
			button.time = millis();
		}
		if (button.state == HIGH)
		{
			// unclicked
			Serial.println("OFF");
			button.time -= millis();
			button.time *= -1;
			Serial.print("On for ");
			Serial.println(button.time);
			
			if (button.time > 10000)
			{
				flight.setPhase(0);
			}

			if (button.time > 5000 && button.time < 10000)
			{
				flight.setPhase(1);
			}

			if (button.time > 500 && button.time <5000)
			{
				flight.nextPhase();
			}
		}
		button.lastState = button.state;
	}

	flight.tick();

	loopTimer -= millis();
	digitalWrite(13, LOW);
	//Serial.print("Loop time: ");
	//Serial.println(loopTimer * -1);
	//delay(1000 + loopTimer); // Slow down the loop.
}