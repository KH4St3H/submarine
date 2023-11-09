#include <Servo.h>
#include <Arduino.h>
#include<motor.h>

Motor::Motor(int pin){
    Servo s;
    s.attach(pin);
    this->servo = s;
}

void Motor::setPower(int n)
{
    this->power = this->base_power + n;
    Serial.println("power: " + power);

    servo.write(power);
}

void Motor::increaseBasePower(int n){
    this->base_power += n;
}