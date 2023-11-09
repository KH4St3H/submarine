#include <Arduino.h>
#include <ArduinoJson.h>
#include <Servo.h>

#include <GY_85.h>
#include <motor.h>

Motor thrustRight = Motor(27);
Motor thrustLeft = Motor(26);

Motor liftLeft0 = Motor(24);
Motor liftLeft1 = Motor(22);

Motor liftRight0 = Motor(25);
Motor liftRight1 = Motor(23);

Motor motors[] = {thrustRight, thrustLeft, liftLeft0, liftLeft1, liftRight0, liftRight1};

float pi = PI;

const int tilt_power = 200;
const int base = 1500;
boolean tilted = false;
boolean lifted = false;
boolean rotated = false;

GY_85 GY85;

boolean receiving = false;

void resetMotors();

void setup()
{
    GY85.init();
    Serial.begin(29000);
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

void stabilize()
{
    int offset = 4;
    int powerStep = 10;
    int maxTime = 5000;
    float *gyroReadings, gx = offset+5, gy = offset+5, gz;
    int starttime = millis();
    while (gx > offset && gy > offset)
    {
        resetMotors();
        gyroReadings = GY85.readGyro();
        gx = GY85.gyro_x(gyroReadings);
        gy = GY85.gyro_y(gyroReadings);
        gz = GY85.gyro_z(gyroReadings);
        if (abs(gx) < 1)
            gx = 0;
        if (abs(gy) < 1)
            gy = 0;
        if (abs(gz) < 1)
            gz = 0;
        
        if(gx < -offset){
            liftLeft0.increaseBasePower(powerStep);
            liftRight0.increaseBasePower(powerStep);
            continue;
        } else if(gx > offset){
            liftLeft1.increaseBasePower(powerStep);
            liftRight1.increaseBasePower(powerStep);
        }
        if(gy > offset){
            liftLeft1.increaseBasePower(powerStep);
            liftLeft0.increaseBasePower(powerStep);
        } else if(gy < -offset){
            liftRight0.increaseBasePower(powerStep);
            liftRight1.increaseBasePower(powerStep);
        }

        delay(50);
        if(millis()-starttime > maxTime){
            return;  // it freezes so we have a max time
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
        liftLeft0.setPower(0);
        liftLeft1.setPower(0);
        liftRight0.setPower(tilt_power);
        liftRight1.setPower(tilt_power);
        return;
    }
    else if (x < -0.9)
    {
        liftRight0.setPower(0);
        liftRight1.setPower(0);
        liftLeft0.setPower(tilt_power);
        liftLeft1.setPower(tilt_power);
        return;
    }
    else if (y > 0.9)
    {
        liftLeft1.setPower(0);
        liftRight1.setPower(0);
        liftLeft0.setPower(tilt_power);
        liftRight0.setPower(tilt_power);
        return;
    }
    else if (y < -0.9)
    {
        liftLeft1.setPower(0);
        liftRight1.setPower(0);
        liftLeft0.setPower(-tilt_power);
        liftRight0.setPower(-tilt_power);
        return;
    }

    float theta;
    if (x > 0.1)
    {
        theta = atan2(y, x) - pi / 4;
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
    liftLeft1.setPower(0);
    liftRight1.setPower(0);
}

void log(String str){
    Serial.println("#debug: "+ str);
}

boolean operateServo()
{
    if (!newData)
        return false;

    newData = false;
    log("data = " + (String) receivedChars);

    // deseralize json data to DJD
    DynamicJsonDocument doc(2048);
    DeserializationError d = deserializeJson(doc, receivedChars);
    if(d.code() != DeserializationError::Code::Ok)
        return false;

    if(doc["check"] != 15){
        return false;
    }

    double lj_y = doc["lj"][1];

    // ascending and descending
    if (doc["hats"][1] == 1)
    {
        log("lift up");
        if (lifted)
            return true;
        liftUp(); // ascend
    }
    else if (doc["hats"][1] == -1)
    {
        if (lifted)
            return true;
        liftDown();
    } 
    else if (lifted)
    {
        resetMotors(); // reset back to default
    }
    log("otherwise");
    log(receivedChars);

    bool buttons[13];
    for(int i=0; i<13; i++){
        buttons[i] = false;
        buttons[i] += (1<<i) & (int) doc["B"];
    }

    // rotation with L1 and R1
    if(buttons[12]){
        stabilize();
    }
    if ((buttons[4] || buttons[5]))
    {
        if (rotated)
            return true;
        // buttons[4] -> L1, buttons[5] -> R1
        rotate(buttons[4]);
        return true;
    }
    else if (rotated)
    {
        resetMotors();
    }

    float rj_x = doc["rj"][0];
    float rj_y = doc["rj"][1];

    // tilt with right joystick
    if ((rj_x > 0.3) || (rj_y > 0.3) || (rj_x < -0.3) || (rj_y < -0.3))
    {
        tilt(rj_x, rj_y);
        return true;
    } // tilt will be checked below

    // tilt with x/o/+/■
    if (buttons[0])
    {
        tilt(0.5, 1); // it's reversed
        return true;
    }
    else if (buttons[1])
    {
        tilt(1, 0.5);
        return true;
    }
    else if (buttons[2])
    {
        tilt(0.5, -1);
        return true;
    }
    else if (buttons[3])
    {
        tilt(-1, 0.5);
        return true;
    }
    else if (tilted)
    {
        resetMotors();
    }

    if (abs(lj_y) < 0.1)
    {
        lj_y = 0;
        thrustLeft.setPower(0);
        thrustRight.setPower(0);
        return true;
    }
    int escValue = map(lj_y * -100, -100, 100, 1000, 2000); // junk that might come in handy
    escValue = lj_y * -500;
    thrustLeft.setPower(escValue);
    thrustRight.setPower(escValue);

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