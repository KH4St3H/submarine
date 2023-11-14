// #include<Arduino.h>

// int numChars

// void recvWithEndMarker(bool *newData, bool *receiving, char *receivedChars)
// {
//     static byte ndx = 0;
//     char endMarker = '\n';
//     char rc;

//     while (Serial.available() > 0 && !newData)
//     {
//         rc = Serial.read();

//         if (rc != endMarker)
//         {
//             *receiving = true;
//             receivedChars[ndx] = rc;
//             ndx++;
//             if (ndx >= numChars)
//             {
//                 ndx = numChars - 1;
//             }
//         }
//         else
//         {
//             receiving = false;
//             receivedChars[&ndx] = '\0'; // terminate the string
//             &ndx = 0;
//             newData = true;
//         }
//     }
// }