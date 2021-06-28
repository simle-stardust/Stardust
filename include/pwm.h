#pragma once
#include <Arduino.h>

typedef enum {
    PWM_PUMP_1 = 0,
    PWM_PUMP_2 = 1,
    PWM_HEATING = 2
} pwm_type_t;

class MyPWMs {
private:
    uint8_t currentVals[3];
public:
    MyPWMs();
    void set(pwm_type_t pwm, uint8_t val);
    uint8_t read(pwm_type_t pwm);
};