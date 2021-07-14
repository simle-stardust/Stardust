#include "pwm.h"

#define PUMP_1_ANALOG_PIN   44
#define PUMP_2_ANALOG_PIN   45
#define HEATING_ANALOG_PIN  46

MyPWMs::MyPWMs()
{
    pinMode(PUMP_1_ANALOG_PIN, OUTPUT);
    pinMode(PUMP_2_ANALOG_PIN, OUTPUT);
    pinMode(HEATING_ANALOG_PIN, OUTPUT);
}

void MyPWMs::set(pwm_type_t pwm, uint8_t val)
{
    switch (pwm)
    {
    case PWM_HEATING:
        currentVals[PWM_HEATING] = val;
        analogWrite(HEATING_ANALOG_PIN, val);
        break;
    case PWM_PUMP_1:
        currentVals[PWM_PUMP_1] = val;
        analogWrite(PUMP_1_ANALOG_PIN, val);
        break;
    case PWM_PUMP_2:
        currentVals[PWM_PUMP_2] = val;
        analogWrite(PUMP_2_ANALOG_PIN, val);
        break;
    default:
        break;
    }
}

uint8_t MyPWMs::read(pwm_type_t pwm)
{
    return currentVals[(uint8_t)pwm];
}