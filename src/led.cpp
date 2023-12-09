#include<Arduino.h>
#include "led.h"

uint ledUpdated = millis();
bool ledState = false;
bool blink = false;

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