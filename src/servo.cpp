#include "servo.h"

// pin  / servo_pointer value (index in array)  / setValve telecommand value
// J8 - 13 - 14
// J9 - 12 - 13
// J10 - 11 - 12
// J11 - 10 - 11
// J12 - 9 - 10
// J13 - 8 - 9
// J14 - 7 - 8

MyServo::MyServo(unsigned int number)
{
	pwm = new Adafruit_PWMServoDriver(ADDRESS);
	gpio_expander = new pca9555(GPIO_EXP_ADDRESS); //instance
	servoNumber = number;
	lastOperation = millis();
	state = SERVO_IDLE;
	servo_pointer = 0;
};

void MyServo::init()
{

	Serial.println(" INITIALIZING SERVOS ");

	pinMode(SERVO_DC, OUTPUT);

	delay(100);
	pwm->begin();
	pwm->setOscillatorFrequency(27000000);
	pwm->setPWMFreq(SERVO_FREQ);

	gpio_expander->begin();
	gpio_expander->gpioPinMode(INPUT);
	//gpio_expander->gpioPinMode(OUTPUT);

	state = SERVO_IDLE;
	servo_pointer = 0;
}

void MyServo::reset()
{
	Serial.println(" PWM RESET ");
	for (uint8_t i = 0; i <= servoNumber; ++i)
	{
		pwm->setPWM(i, 0, 0);
	}

	digitalWrite(SERVO_DC, HIGH);
	Serial.println(" POWER RESET ");
	delay(100);
	digitalWrite(SERVO_DC, LOW);
}

void MyServo::setOpen(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1)
		return;

	servos[servo - 1].desired = 1;

	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Open");
}

void MyServo::setClosed(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1)
		return;

	servos[servo - 1].desired = 0;
	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Closed");
}

bool MyServo::getStatus(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1)
		return 0;

	//gpio_expander->gpioPort(0);
	//Serial.print(gpio_expander->readGpioPort(), HEX);
	//Serial.print(" ");

	//mcp.gpioDigitalWrite(i, LOW);

	// TODO: change to reading digital
	// inputs from krancowki
	return servos[servo - 1].status;
}

void MyServo::tick()
{
	if (servo_pointer > servoNumber - 1)
		servo_pointer = 0;

	switch (state)
	{
	case SERVO_IDLE:
		if (servos[servo_pointer].desired != servos[servo_pointer].status)
		{
			for (uint8_t j = 0; j <= servoNumber; ++j)
			{
				pwm->setPWM(j, 0, 0);
			}
			digitalWrite(SERVO_DC, HIGH);
			state = SERVO_RESETTING;
			lastOperation = millis();
			break;
		}
		else
		{
			servo_pointer++;
		}
		break;

	case SERVO_RESETTING:
		if (millis() - lastOperation > SERVO_RESET_TIME)
		{
			digitalWrite(SERVO_DC, LOW);
			pwm->setPWM(servo_pointer + 1, 0, servos[servo_pointer].desired ? SERVOMIN : SERVOMAX);
			state = SERVO_MOVING;
			lastOperation = millis();
		}
		break;

	case SERVO_MOVING:
		if (millis() - lastOperation > SERVO_SAMPLING_TIME)
		{
			state = SERVO_IDLE;
			servos[servo_pointer].status = servos[servo_pointer].desired;
			Serial.print(servos[servo_pointer].desired ? "Opened " : "Closed ");
		}
		break;
	default:
		break;
	}
}

bool MyServo::ready()
{
	for (uint8_t i = 0; i <= servoNumber; ++i)
	{
		if (servos[i].desired != servos[i].status)
			return 0;
	}
	return 1;
}

void MyServo::openSequence()
{
	init();

	for (uint8_t i = 1; i <= servoNumber; ++i)
	{
		setOpen(i);
	}
}

void MyServo::closeSequence()
{
	init();

	for (uint8_t i = 1; i <= servoNumber; ++i)
	{
		setClosed(i);
	}
}