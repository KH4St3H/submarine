#include <Arduino.h>

#include<utils.h>

const int numChars = 1000;
char receivedChars[1000]; // an array to store the received data

bool receiving = false;
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
