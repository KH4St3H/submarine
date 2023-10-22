#include <Servo.h>
#include <Arduino.h>

class Motor{
    public:
        Servo servo;
        int pin;
        int power;
        int base_power = 1500;

        void attach(int n);
        void setServo(Servo srv);
        void setPower(int n);
};

void Motor::attach(int n){
    servo.attach(n);
}

void Motor::setServo(Servo srv)
{
    servo = srv;
}

void Motor::setPower(int n)
{
    power = base_power + n;
    Serial.println("power: " + power);

    servo.write(power);
}