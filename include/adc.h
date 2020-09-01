#pragma once
#include <Arduino.h>

#define INA1 A1
#define INA2 A3
#define INA3 A5
#define INA4 A7

class MyADC {

    private:

    int adc[4] = {0,0,0,0};

    public:
    void init() {
        pinMode(INA1, INPUT);
        pinMode(INA2, INPUT);
        pinMode(INA3, INPUT);
        pinMode(INA4, INPUT);
    }

    void update() {
        adc[0] = analogRead(INA1);
        adc[1] = analogRead(INA2);
        adc[2] = analogRead(INA3);
        adc[3] = analogRead(INA4);
    }

    int getADC(int number) {
        return adc[number];
    }
};