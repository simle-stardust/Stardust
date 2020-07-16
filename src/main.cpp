#include <Arduino.h>
#include "sdcard.h"
#include "rtc.h"
#include "servo.h"
#include "sensors.h"

#include <EEPROM.h>

#define SD_CARD_SS 4
#define BUTTON_PIN 46

MySD flash(SD_CARD_SS);
MyRTC rtc;
MySensors sensors;
MyServo servo(2);

bool close = 0;

struct debounce
{
	int state = 0;
	int lastState = 0;
	int time = 0;
};
struct debounce button;

int loopTimer;

int flightPhase = 0;

String message;

void setup()
{
	pinMode(13, OUTPUT);
	pinMode(BUTTON_PIN, INPUT);

	Serial.begin(115200);
	Serial.print("compiled: ");
	Serial.println(__DATE__);
	Serial.println(__TIME__);

	servo.init();
	rtc.init();

	// Year/Month/Day directory with file named after init time.
	flash.init(rtc.dateString(rtc.getTime()), rtc.timeString(rtc.getTime()));
	sensors.init(&rtc, &flash);

	flightPhase = EEPROM.read(0);

	button.time = millis();
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

			if (button.time > 100 && button.time < 500)
			{
				flightPhase--;
				Serial.print("\n Advancing to Phase: ");
				Serial.println(flightPhase);
			}

			if (button.time > 500 && button.time < 2000)
			{
				flightPhase++;
				Serial.print("\n Advancing to Phase: ");
				Serial.println(flightPhase);
			}
		}
		button.lastState = button.state;
	}

	if (Serial.available())
	{
		char c = Serial.read(); //gets one byte from serial buffer
		if (c == '\n')			//looks for end of data packet marker
		{
			Serial.read();			 //gets rid of following \r
			Serial.println(message); //prints string to serial port out

			//do stuff with captured message
			if (message.indexOf("NextPhase") != -1)
				flightPhase++;
			if (message.indexOf("PrevPhase") != -1)
				flightPhase--;

			message = ""; //clears variable for new input
		}
		else
		{
			message += c; //makes the string message
		}
	}

	// Serial.print("Flight phase: ");
	// Serial.print(flightPhase);
	sensors.tick();
	EEPROM.update(0, flightPhase);

	switch (flightPhase)
	{
	default:
		if (flightPhase < 0)
			flightPhase = 0;
		if (flightPhase > 4)
			flightPhase = 4;
	case 0: // Pre-launch
		//Serial.println(" Awaiting liftoff");
		break;
	case 1: // Flight
		// Serial.println("In Flight");

		break;
	case 2: // Sampling ON
		// Serial.println(" Sampling ON");
		servo.openSequence();
		if (!servo.ready())
			servo.tick();

		break;
	case 3: // Sampling OFF
		// Serial.println(" Sampling OFF - Safing");
		servo.closeSequence();
		if (!servo.ready())
			servo.tick();
		break;
	case 4: // After Launch
		// Serial.println(" Mission complete");

		break;
	}

	loopTimer -= millis();
	digitalWrite(13, LOW);
	//Serial.print("Loop time: ");
	//Serial.println(loopTimer * -1);
	delay(100);
}