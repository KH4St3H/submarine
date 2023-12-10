#include <Arduino.h>
#include <ArduinoJson.h>

#include "GY_85.h"
#include "motor.h"
#include "movements.h"
#include "led.h"
#include "utils.h"

#include "auv.h"

GY_85 GY85;

void setup()
{
    Serial.begin(29000);
    GY85.init();

    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
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
        return false;  // could not decode json

    if(doc["check"] != 15)
        return false;
    
    if(doc["forward"]){
        log("here");
        forward((int) doc["forward"]);
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

    if(buttons[8])
        blinker();

    if(buttons[9])
        flipLED();


    float rj_x = doc["rj"][0];
    float rj_y = doc["rj"][1];

    // up and down tilt with x/△
    if (buttons[0])
        rj_x = 0.5, rj_y = 1;
    else if (buttons[2])
        rj_x = 0.5, rj_y = -1;

    tilt(rj_x, rj_y);
    thrust(lj_y);
    rotate(buttons[4], buttons[5]);

    return true;
}

void loop()
{
    recvWithEndMarker();
    operateServo();
    if(blink)
        blinkLED();
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