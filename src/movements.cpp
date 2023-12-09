#include<movements.h>
#include<motor.h>
#include<config.h>
#include<Arduino.h>

Motor thrustRight = Motor(27);
Motor thrustLeft = Motor(26);

Motor liftLeft0 = Motor(24);
Motor liftLeft1 = Motor(22, true, (4/3));

Motor liftRight0 = Motor(25);
Motor liftRight1 = Motor(23, false, (4/3));

bool rotating = false, tilting=false, lifting=false;


// TODO: add stablization functionality

void resetMotors()
{
    liftLeft0.setPower(0);
    liftLeft1.setPower(0);
    liftRight0.setPower(0);
    liftRight1.setPower(0);
    thrustLeft.setPower(0);
    thrustRight.setPower(0);
    lifting = false;
    rotating= false;
    tilting = false;

}

void tilt(float x, float y)
{
    // joystick correction
    if(abs(x)< 0.2 && abs(y) < 0.2){
        tilting = false;
        return;
    }
    tilting = true;
    y = -y; // right joystick y axis is flipped

    // tilt to right, left, up and down respectively
    if (y > 0.9)
    {
        liftLeft1.setPower(0);
        liftRight1.setPower(0);
        liftLeft0.setPower(-tiltPower);
        liftRight0.setPower(-tiltPower);
        return;
    }
    else if (y < -0.9)
    {
        liftLeft1.setPower(0);
        liftRight1.setPower(0);
        liftLeft0.setPower(tiltPower);
        liftRight0.setPower(tiltPower);
        return;
    }

    float theta;
    if (x > 0.07)
        theta = atan2(y, x) - PI / 4;
    else // x is low so we assume it's zero
        theta = PI / 2;

    float coef = sqrt(pow(x, 2) + pow(y, 2)); // pythagorean formula to calculate how much to tilt it
    coef = max(coef, 1);                      // safety measure

    // calculate cos(theta-pi/2) for right motor
    liftRight0.setPower((int)-tiltPower * cos(theta) * coef);
    // calculate sin(theta-pi/2) for left motor
    liftLeft0.setPower((int)-tiltPower * sin(theta) * coef);
    liftLeft1.setPower(0);
    liftRight1.setPower(0);
}

void lift(Direction dir){
    if(dir == UP){
        liftLeft0.setPower(liftSpeed);
        liftLeft1.setPower(liftSpeed);
        liftRight0.setPower(liftSpeed);
        liftRight1.setPower(liftSpeed);
        lifting = true;
    }else if(dir == DOWN){
        liftLeft0.setPower(-liftSpeed);
        liftLeft1.setPower(-liftSpeed);
        liftRight0.setPower(-liftSpeed);
        liftRight1.setPower(-liftSpeed);
        lifting = true;
    }else if(lifting){
        resetMotors();
        lifting = false;
    }
}

// rotating in yaw axis
void rotate(bool left, bool right)
{
    if (left && right) // if R1 and L1 activate both thrusters
    {
        thrustRight.setPower(rotateSpeed);
        thrustLeft.setPower(rotateSpeed);
    } else if(right) // if only R1 we rotate to right
    {
        thrustRight.setPower(rotateSpeed);
        thrustLeft.setPower(-rotateSpeed);
    }else if(left) // if only L1 we rotate to left
    {
        thrustRight.setPower(-rotateSpeed);
        thrustLeft.setPower(rotateSpeed);
    }else{ // cancel rotation if none  TODO: should not effect other functionalities
        thrustLeft.setPower(0);
        thrustRight.setPower(0);
        rotating = false;
        return;
    }
    rotating = true; // indicate we are rotating
}

// activates both thursters with the same power
bool thrust(float val)
{
    // -1 < val < 1
    // corrects joystick error and counts abs(val)<0.1 as zero
    if (abs(val) < 0.1)
    {
        // turning motors off
        thrustLeft.setPower(0);
        thrustRight.setPower(0);
        return false;
    }
    // we multiply it by 500 to get a power between -500 and 500
    // and * -1 beacuse left joystick is reversed
    int escValue = val * -500; // -500 < val < 500
    thrustLeft.setPower(escValue);
    thrustRight.setPower(escValue);
    return true;
}