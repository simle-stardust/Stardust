#include "servo.h"

MyServo::MyServo(unsigned int number)
{
	pwm = new Adafruit_PWMServoDriver(ADDRESS);
	gpio_expander = new pca9555(GPIO_EXP_ADDRESS); //instance
	servoNumber = number;
	lastOperation = millis();
};

void MyServo::init()
{

	Serial.println(" INITIALIZING SERVOS ");

	pinMode(SERVO_DC, OUTPUT);
	digitalWrite(SERVO_DC, LOW);
	delay(100);
	pwm->begin();
	pwm->setOscillatorFrequency(27000000);
	pwm->setPWMFreq(SERVO_FREQ);

	gpio_expander->begin();
	gpio_expander->gpioPinMode(INPUT);
	//gpio_expander->gpioPinMode(OUTPUT);
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

	if (millis() - lastOperation > SERVO_SAMPLING_TIME)
	{
		if (servos[servo_pointer].desired != servos[servo_pointer].status)
		{
			reset();
			move(servo_pointer + 1);
			servo_pointer++;
		}
		else if (!ready())
		{
			servo_pointer++;
			tick();
		}
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

void MyServo::move(uint8_t servo)
{
	if (servo >= NB_OF_SERVOS + 1)
		return;

	pwm->setPWM(servo, 0, servos[servo - 1].desired ? SERVOMIN : SERVOMAX);
	servos[servo - 1].status = servos[servo - 1].desired;
	lastOperation = millis();
	Serial.print(servos[servo - 1].desired ? "Opened " : "Closed ");
	Serial.println(servo);
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