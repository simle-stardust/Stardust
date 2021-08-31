#include "servo.h"

// pin  / servo_pointer value (index in array)  / setValve telecommand value
// J8 - 13 - 14
// J9 - 12 - 13
// J10 - 11 - 12
// J11 - 10 - 11
// J12 - 9 - 10
// J13 - 8 - 9
// J14 - 7 - 8

#define SERVOMIN  300 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)

#define SERVO_DEBUG

const servo_configuration_t servos_configuration[NB_OF_SERVOS] = 
{
	{
		"J14",
		SERVOMIN,
		SERVOMAX,
		8,
		14
	},
	{
		"J13",
		SERVOMIN,
		SERVOMAX,
		9,
		13
	},
	{
		"J12",
		SERVOMIN,
		SERVOMAX,
		10,
		12
	},
	{
		"J11",
		SERVOMIN,
		SERVOMAX,
		11,
		11
	},
	{
		"J10",
		SERVOMIN,
		SERVOMAX,
		12,
		10
	},
	{
		"J9",
		SERVOMIN,
		SERVOMAX,
		13,
		9
	},
	{
		"J8",
		SERVOMIN,
		SERVOMAX,
		14,
		8
	}
};


MyServo::MyServo()
{
	pwm = new Adafruit_PWMServoDriver(ADDRESS);
	gpio_expander = new pca9555(GPIO_EXP_ADDRESS);//instance

	for (uint8_t i = 0; i < NB_OF_SERVOS; i++)
	{
		memcpy(&servos[i].config, &servos_configuration[i], sizeof(servo_configuration_t));
	}

	lastOperation = millis();
	state = SERVO_IDLE;
	servo_pointer = 0;
};

void MyServo::init()
{

#ifdef SERVO_DEBUG
	Serial.println(" INITIALIZING SERVOS ");
#endif

	pinMode(SERVO_DC, OUTPUT);

	delay(100);
	pwm->begin();
	pwm->setOscillatorFrequency(27000000);
	pwm->setPWMFreq(SERVO_FREQ);

	gpio_expander->begin();
	gpio_expander->gpioPinMode(INPUT);
	//gpio_expander->gpioPinMode(OUTPUT);
	
	digitalWrite(SERVO_DC, HIGH);

	state = SERVO_IDLE;
	servo_pointer = 0;
}

void MyServo::setOpen(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1) return; 

	servos[servo - 1].desired = 1;

#ifdef SERVO_DEBUG
	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Open");
#endif
}

void MyServo::setClosed(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1) return; 

	servos[servo - 1].desired = 0;
	
#ifdef SERVO_DEBUG
	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Closed");
#endif
}

bool MyServo::getStatus(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1) return 0;

	//gpio_expander->gpioPort(0);
	//Serial.print(gpio_expander->readGpioPort(), HEX);
	//Serial.println("   ");
	//Serial.print(" ");

	
    //mcp.gpioDigitalWrite(i, LOW);

	// TODO: change to reading digital 
	// inputs from krancowki
	//return servos[servo - 1].status;
	return (bool)gpio_expander->gpioDigitalRead(servos[servo - 1].config.endswitch_index);
}

void MyServo::tick()
{
	switch (state)
	{
		case SERVO_IDLE:
			for (uint8_t i = 0; i < NB_OF_SERVOS; ++i)
			{
				if (servos[i].desired != servos[i].status)
				{
					for (uint8_t j = 0; j < NB_OF_SERVOS; ++j)
					{
						pwm->setPWM(servos[j].config.PWM_driver_index, 0, 0);
					}
					digitalWrite(SERVO_DC, HIGH);
					state = SERVO_RESETTING;
					servo_pointer = i;
					lastOperation = millis();
#ifdef SERVO_DEBUG
					Serial.println("State = SERVO_RESETTING");
					Serial.println("servo_pointer = " + String(servo_pointer));
					Serial.println("driver index = " + String(servos[servo_pointer].config.PWM_driver_index));
#endif
					break;
				}
			}
			break;

		case SERVO_RESETTING:
			if (millis() - lastOperation > SERVO_RESET_TIME)
			{
				digitalWrite(SERVO_DC, LOW);
				pwm->setPWM(servos[servo_pointer].config.PWM_driver_index, 0, servos[servo_pointer].desired ? servos[servo_pointer].config.open_position : servos[servo_pointer].config.close_position);
				state = SERVO_MOVING;
#ifdef SERVO_DEBUG
				Serial.println("State = SERVO_MOVING");
				Serial.println("Setting PWM to = " + String(servos[servo_pointer].desired ? servos[servo_pointer].config.open_position : servos[servo_pointer].config.close_position));
#endif
				lastOperation = millis();
			}
			break;

		case SERVO_MOVING:
			if (millis() - lastOperation > SERVO_SAMPLING_TIME)
			{
#ifdef SERVO_DEBUG
				Serial.println("State = SERVO_IDLE");
				Serial.print(servos[servo_pointer].desired ? "Opened " : "Closed ");
#endif
				state = SERVO_IDLE;
				servos[servo_pointer].status = servos[servo_pointer].desired;
				digitalWrite(SERVO_DC, HIGH);
			}
			break;
		default:
			break;
	}
}

bool MyServo::ready()
{
	for (uint8_t i = 0; i < NB_OF_SERVOS; ++i)
	{
		if (servos[i].desired != servos[i].status)
			return 0;
	}
	return 1;
}

void MyServo::openSequence()
{
	init();

	for (uint8_t i = 1; i < NB_OF_SERVOS + 1; ++i)
	{
		setOpen(i);
	}
}

void MyServo::closeSequence()
{
	init();

	for (uint8_t i = 1; i < NB_OF_SERVOS + 1; ++i)
	{
		setClosed(i);
	}
}