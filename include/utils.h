#pragma once
#include<utils.h>

extern char receivedChars[1000]; // an array to store the received data

extern bool receiving;
extern bool newData;

void recvWithEndMarker();
void log(String str);