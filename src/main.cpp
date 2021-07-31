#include <Arduino.h>
#include "director.h"

#define BUTTON_PIN_1 A1
#define BUTTON_PIN_2 A2

Flight flight;

struct debounce
{
	int state = 0;
	int lastState = 0;
	unsigned long time = 0;
};
struct debounce button_1;
struct debounce button_2;

unsigned long loopTimer;

void setup()
{
	pinMode(13, OUTPUT);
	pinMode(BUTTON_PIN_1, INPUT);
	pinMode(BUTTON_PIN_2, INPUT);

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

	button_1.state = digitalRead(BUTTON_PIN_1);
	if (button_1.state != button_1.lastState)
	{
		if (button_1.state == HIGH)
		{
			// clicked
			Serial.println("Button ON");
			button_1.time = millis();
		}
		if (button_1.state == LOW)
		{
			// unclicked
			Serial.println("Button OFF");
			button_1.time -= millis();
			button_1.time *= -1;
			Serial.print("On for ");
			Serial.println(button_1.time);

			if (button_1.time > 10000)
			{
				flight.setPhase(0);
			}

			if (button_1.time > 5000 && button_1.time < 10000)
			{
				flight.setPhase(1);
			}

			if (button_1.time > 500 && button_1.time < 5000)
			{
				flight.nextPhase();
			}
		}
		button_1.lastState = button_1.state;
	}

	button_2.state = digitalRead(BUTTON_PIN_2);
	if (button_2.state != button_2.lastState)
	{
		if (button_2.state == HIGH)
		{
			// clicked
			Serial.println("Button ON");
			button_2.time = millis();
		}
		if (button_2.state == LOW)
		{
			// unclicked
			Serial.println("Button OFF");
			button_2.time -= millis();
			button_2.time *= -1;
			Serial.print("On for ");
			Serial.println(button_2.time);

			if (button_2.time > 10000)
			{
				flight.setPhase(2);
			}

			if (button_2.time > 5000 && button_2.time < 10000)
			{
				flight.setPhase(3);
			}

			if (button_2.time > 500 && button_2.time < 5000)
			{
				flight.prevPhase();
			}
		}
		button_2.lastState = button_2.state;
	}

	flight.tick();
	//Serial.println("tick");

	loopTimer -= millis();
	digitalWrite(13, LOW);
	//Serial.print("Loop time: ");
	//Serial.println(loopTimer * -1);
	//delay(1000 + loopTimer); // Slow down the loop.
}