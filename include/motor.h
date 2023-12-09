#pragma once
#ifndef MOTOR_H
#define MOTOR_H
#include<motor.h>
#include <Servo.h>

class Motor{
    public:
        Motor(int pin, bool flip=false, float multiplyer=1);
        void setPower(int n);
        void attach(int n);
        void increaseBasePower(int n);
    private:
        Servo servo;
        int pin;
        float multiplyer;
        bool flip;
        int power;
        int base_power = 1500;
};
#endif