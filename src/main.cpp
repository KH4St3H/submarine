#include <Arduino.h>
#include <ArduinoJson.h>

#include <GY_85.h>
#include "motor.h"
#include "movements.h"

GY_85 GY85;

bool receiving = false;

bool ledState = false;
bool blink = false;
uint ledUpdated = millis();


void resetMotors();

void setup()
{
    Serial.begin(29000);
    GY85.init();

    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
}

const int numChars = 1024;
char receivedChars[numChars]; // an array to store the received data

bool newData = false;


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
            ndx = min(numChars-1, ndx);
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

void log(String str){
    Serial.println("#debug: "+ str);
}

void flipLED(){
    if(ledState){
        ledState = false;
        digitalWrite(13, LOW);
    }else{
        ledState = true;
        digitalWrite(13, HIGH);
    }
}

void blinker(){
    if(blink){
        blink = false;
    }
    else{
        blink = true;
    }
}

void blinkLED(){
    if(millis() - ledUpdated < 200)
        return;
    ledUpdated = millis();
    
    if(ledState){
        ledState = false;
        digitalWrite(13, LOW);
    }else{
        ledState = true;
        digitalWrite(13, HIGH);
    }
}

bool operateServo()
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
        lift(UP);
    else if (doc["hats"][1] == -1)
        lift(DOWN);
    else
        lift(OFF);


    // unpacking buttons from integer
    bool buttons[13];
    for(int i=0; i<13; i++){
        buttons[i] = false;
        buttons[i] += (1<<i) & (int) doc["B"];
    }

    if(buttons[8]){
        blinker();
    }

    if(buttons[9])
        flipLED();


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
    else if (buttons[2])
    {
        tilt(0.5, -1);
        return true;
    }

    if(thrust(lj_y)){
        return true;
    }
    rotate(buttons[4], buttons[5]);

    return true;
}
void loop()
{
    recvWithEndMarker();
    operateServo();
    if(!blink){
        blinkLED();
    }
    // delay(50);
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