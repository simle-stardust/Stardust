#include "servo.h"

MyServo::MyServo(unsigned int number)
{
	pwm = new Adafruit_PWMServoDriver(ADDRESS);
	servoNumber = number;
	lastOperation = millis();
};

void MyServo::init()
{

	Serial.println(" INITIALIZING SERVOS ");

	pinMode(SERVO_DC, OUTPUT);
	digitalWrite(SERVO_DC, HIGH);
	delay(100);
	pwm->begin();
	pwm->setOscillatorFrequency(27000000);
	pwm->setPWMFreq(SERVO_FREQ);
}

void MyServo::reset()
{
	Serial.println(" PWM RESET ");
	for (uint8_t i = 0; i <= servoNumber; ++i)
	{
		pwm->setPWM(i, 0, 0);
	}

	digitalWrite(SERVO_DC, LOW);
	Serial.println(" POWER RESET ");
	delay(500);
	digitalWrite(SERVO_DC, HIGH);
}

void MyServo::setOpen(uint8_t servo)
{
	servos[servo - 1].desired = 1;

	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Open");
}

void MyServo::setClosed(uint8_t servo)
{
	servos[servo - 1].desired = 0;
	Serial.print("Set ");
	Serial.print(servo);
	Serial.println(" to Closed");
}

bool MyServo::getStatus(uint8_t servo)
{
	return servos[servo - 1].status;
}

void MyServo::tick()
{
	if (servo_pointer > servoNumber - 1)
		servo_pointer = 0;
	if (millis() - lastOperation > SERVO_SAMPLING_TIME)
	{
		reset();
		if (servos[servo_pointer].desired != servos[servo_pointer].status)
		{
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