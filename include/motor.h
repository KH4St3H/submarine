#ifndef MOTOR_H
#define MOTOR_H

#pragma once
#include <Servo.h>

class Motor{
    public:
        Servo servo;
        int pin;
        int power;
        int base_power = 1500;
        void setServo(Servo src);
        void setPower(int n);
        void attach(int n);
};
#endif