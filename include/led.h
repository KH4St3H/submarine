#pragma once
#include "led.h"
#include<Arduino.h>

extern bool ledState;
extern bool blink;

void blinkLED();
void blinker();
void flipLED();