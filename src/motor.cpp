#include <Servo.h>
#include <Arduino.h>
#include<motor.h>

Motor::Motor(int pin, boolean flip, float multiplyer){
    Servo s;
    s.attach(pin);
    this->servo = s;
    this->flip = flip;
    this->multiplyer = multiplyer;
}

void Motor::setPower(int n)
{
    if(this->flip)
        n = -n;
    int tb = n * this->multiplyer;
    if(tb > 499){
        tb = 500;
    }else if(tb < -499){
        tb = -500;
    }
    this->power = this->base_power + tb;
    Serial.println("power: " + power);

    servo.write(power);
}

void Motor::increaseBasePower(int n){
    if(this->flip)
        n = -n;
    this->base_power += n*this->multiplyer;
}