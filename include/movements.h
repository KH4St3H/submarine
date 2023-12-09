#ifndef MOVEMENTS_H
#define MOVEMENTS_H
#include "movements.h"

#include "motor.h"

enum Direction{
    FORWARD, BACKWARD,
    LEFT, RIGHT,
    UP, DOWN,
    OFF
};

void lift(Direction);
void resetMotors();
void tilt(float, float);
void rotate(bool, bool);
bool thrust(float val);
#endif