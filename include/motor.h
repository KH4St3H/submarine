#ifndef MOTOR_H
#define MOTOR_H

#pragma once
#include <Servo.h>

class Motor{
    public:
        Motor(int pin);
        void setPower(int n);
        void attach(int n);
        void increaseBasePower(int n);
    private:
        Servo servo;
        int pin;
        int power;
        int base_power = 1500;
};
#endif