#include <Arduino.h>
#include <ArduinoJson.h>
#include <Servo.h>

#include <GY_85.h>
#include <motor.h>

Motor thrustRight;
Motor thrustLeft;

Motor liftLeft0;
Motor liftLeft1;

Motor liftRight0;
Motor liftRight1;

Motor motors[] = {thrustRight, thrustLeft, liftLeft0, liftLeft1, liftRight0, liftRight1};

float pi = PI;

const int tilt_power = 200;
const int base = 1500;
boolean tilted = false;
boolean lifted = false;
boolean rotated = false;

GY_85 GY85;

boolean receiving = false;

void setup()
{

    GY85.init();

    Servo srv0, srv1, srv2, srv3, srv4, srv5;
    thrustRight.setServo(srv0);
    thrustRight.attach(27);

    thrustLeft.setServo(srv1);
    thrustLeft.attach(26);

    liftLeft0.setServo(srv2);
    liftLeft0.attach(24);

    liftLeft1.setServo(srv3);
    liftLeft1.attach(22);

    liftRight0.setServo(srv4);
    liftRight0.attach(25);

    liftRight1.setServo(srv5);
    liftRight1.attach(23);

    Serial.begin(38400);
}

const int numChars = 1024;
char receivedChars[numChars]; // an array to store the received data

boolean newData = false;


void recvWithEndMarker()
{
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (Serial.available() > 0 && newData == false)
    {
        rc = Serial.read();

        if (rc != endMarker)
        {
            receiving = true;
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars)
            {
                ndx = numChars - 1;
            }
        }
        else
        {
            receiving = false;
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}


void rotate(boolean left)
{
    if (left)
    {
        thrustRight.setPower(tilt_power);
    }
    else
    {
        thrustLeft.setPower(tilt_power);
    }
    rotated = true;
}

void liftUp()
{
    liftLeft0.setPower(tilt_power);
    liftLeft1.setPower(tilt_power);
    liftRight0.setPower(tilt_power);
    liftRight1.setPower(tilt_power);
    lifted = true;
}
void liftDown()
{
    liftLeft0.setPower(-tilt_power);
    liftLeft1.setPower(-tilt_power);
    liftRight0.setPower(-tilt_power);
    liftRight1.setPower(-tilt_power);
    lifted = true;
}
void resetMotors()
{
    liftLeft0.setPower(0);
    liftLeft1.setPower(0);
    liftRight0.setPower(0);
    liftRight1.setPower(0);
    lifted = false;
    rotated = false;
    tilted = false;
}

void tilt(float x, float y)
{
    tilted = true;
    y = -y; // it comes reversed from right joystick so we flip it

    // tilt to right, left, up and down respectively
    if (x > 0.9)
    {
        liftRight0.setPower(tilt_power);
        liftRight1.setPower(tilt_power);
        return;
    }
    else if (x < -0.9)
    {
        liftLeft0.setPower(tilt_power);
        liftLeft1.setPower(tilt_power);
        return;
    }
    else if (y > 0.9)
    {
        liftLeft0.setPower(tilt_power);
        liftRight0.setPower(tilt_power);
        return;
    }
    else if (y < -0.9)
    {
        liftLeft0.setPower(-tilt_power);
        liftRight0.setPower(-tilt_power);
        return;
    }

    float theta;
    if (x > 0.1)
    {
        theta = atan(y / x) - pi / 2;
    }
    else
    {
        theta = pi / 2;
    }

    // using math formula to calculate value of each motor
    float coef = sqrt(pow(x, 2) + pow(y, 2)); // pythagorean formula to calculate how much to tilt it
    coef = max(coef, 1);                      // safety measure

    // calculate cos(theta-pi/2) for right motor
    liftRight0.setPower((int)tilt_power * cos(theta) * coef);
    // calculate sin(theta-pi/2) for left motor
    liftLeft0.setPower((int)tilt_power * sin(theta) * coef);
}

boolean operateServo()
{
    if (!newData)
    {
        return false;
    }
        newData = false;

        // deseralize json data to DJD
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, receivedChars);

        double lj_y = doc["left_joystick"][1];

        // ascending and descending
        if (doc["hats"][1] == 1)
        {
            liftUp(); // ascend
        }
        else if (doc["hats"][1] == -1)
        {
            liftDown(); // descend
        }
        else if (lifted)
        {
            resetMotors(); // reset back to default
        }

        // rotation with L1 and R1
        if (doc["buttons"][4] or doc["buttons"][5])
        {
            // buttons[4] -> L1, buttons[5] -> R1
            rotate(doc["buttons"][4]);
            return true;
        }
        else if (rotated)
        {
            resetMotors();
        }

        float rj_x = doc["right_joystick"][0];
        float rj_y = doc["right_joystick"][1];

        // tilt with right joystick
        if ((rj_x > 0.3) || (rj_y > 0.3) || (rj_x < -0.3) || (rj_y < -0.3))
        {
            tilt(rj_x, rj_y);
            return true;
        } // tilt will be checked below

        // tilt with x/o/+/■
        if (doc["buttons"][0])
        {
            tilt(0.5, 1); // it's reversed
            return true;
        }
        else if (doc["buttons"][1])
        {
            tilt(1, 0.5);
            return true;
        }
        else if (doc["buttons"][2])
        {
            tilt(0.5, -1);
            return true;
        }
        else if (doc["buttons"][3])
        {
            tilt(-1, 0.5);
            return true;
        }
        else if (tilted)
        {
            resetMotors();
        }

        // junk that might come in handy
        int escValue = map(lj_y * -100, -100, 100, 1000, 2000);
        thrustLeft.setPower(escValue);
        thrustRight.setPower(escValue);

        // DynamicJsonDocument *sensorData;
        // sensorData = GY85.toJson();
        // String output;
        // serializeJson(*sensorData, output);
        // Serial1.println(output);
    return true;
}
void loop()
{
    recvWithEndMarker();
    boolean op = operateServo();
    delay(50);
    if (!receiving)
    {
        DynamicJsonDocument *sensorData;
        sensorData = GY85.toJson();
        String output;
        serializeJson(*sensorData, output);
        Serial.println(output);
        delay(100);
    }
}