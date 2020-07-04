#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN  100 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60 // Analog servos run at ~50 Hz updates
#define TIME_DEADZONE 2000

#define ADDRESS 0x40

#define SERVO_DC 47

struct servo_status {
	bool status = 0;
	bool desired = 0;
};


class MyServo {
	public:

	Adafruit_PWMServoDriver *pwm;

	unsigned int servoNumber = 0;
	struct servo_status servos[16];

	uint8_t servo_pointer = 0;

	unsigned long lastOperation = 0;

	MyServo(unsigned int number) {
		pwm = new Adafruit_PWMServoDriver(ADDRESS);
		servoNumber = number;
		lastOperation = millis();
	};

	void init() {

		pinMode(SERVO_DC, OUTPUT);
		digitalWrite(SERVO_DC, LOW);

		pwm->begin();

		pwm->setOscillatorFrequency(27000000);
  		pwm->setPWMFreq(SERVO_FREQ); 
	}

	void reset() {
		for (uint8_t servo = 0; servo < servoNumber; servo++) {
			pwm->setPin(servo, 0);
		}
	}

	void setOpen(uint8_t servo) {
		servos[servo-1].desired = 1;
		Serial.print("Set ");
		Serial.print(servo);
		Serial.println(" to Open");
	}

	void setClosed(uint8_t servo) {
		servos[servo-1].desired = 0;
		Serial.print("Set ");
		Serial.print(servo);
		Serial.println(" to Closed");
	}

	bool getStatus(uint8_t servo) {
		return servos[servo-1].status;
	}

	void tick() {		
		if(servo_pointer > servoNumber-1) servo_pointer = 0;
		if(millis() - lastOperation > TIME_DEADZONE) {
			reset();
			digitalWrite(SERVO_DC, LOW);

			if(servos[servo_pointer].desired != servos[servo_pointer].status) {
				if(servos[servo_pointer].desired == 1) open(servo_pointer + 1);
				if(servos[servo_pointer].desired == 0) close(servo_pointer + 1);
				servo_pointer++;
			} else {
				servo_pointer++;
				tick();
			}
		}
	}

	bool ready() {
		for(uint8_t i = 0; i < servoNumber; ++i) {
			if(servos[i].desired != servos[i].status) return 0;
		}
		return 1;
	}

	private:

	void open(uint8_t servo) {
		digitalWrite(SERVO_DC, HIGH);
		delay(10);
		pwm->setPWM(servo, 0, SERVOMAX);
		servos[servo-1].status = 1;
		lastOperation = millis();

		Serial.print("Opened ");
		Serial.println(servo);
	}

	void close(uint8_t servo) {
		digitalWrite(SERVO_DC, HIGH);
		delay(10);
		pwm->setPWM(servo, 0, SERVOMIN);
		servos[servo-1].status = 0;
		lastOperation = millis();

		Serial.print("Closed ");
		Serial.println(servo);
	}
};