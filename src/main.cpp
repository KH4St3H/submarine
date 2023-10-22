#include <Arduino.h>
#include <ArduinoJson.h>
#include <Servo.h>

Servo servo;

void setup() {

  // put your setup code here, to run once:
  servo.attach(9);

  servo.write(180);
  delay(1000); // delay to allow the ESC to recognize the stopped signal.
  servo.write(91);
  delay(2000);

  servo.write(0);
  delay(1000);
  servo.write(91);
  delay(2000);

  Serial.begin(9600);
  Serial1.begin(38400);
  Serial.println("arming motor");

  // for (int i = 80; i < 90; i++)
  // {
  //   servo.write(i);
  //   Serial.println(i);
  //   delay(500);
  // }
  // servo.write(91);
}

const int numChars = 1024;
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;

void log(String str){
  Serial.print(str);
  Serial.print('\t');
  Serial.println(receivedChars);
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial1.available() > 0 && newData == false) {
        rc = Serial1.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        Serial.print("This just in ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}

void operateServo(){
  if (newData == true){
    newData = false;
    DynamicJsonDocument doc(2048);
    Serial.println(receivedChars);
    deserializeJson(doc, receivedChars);
    String d = doc["left_joystick"];
    double lj_y = doc["left_joystick"][1];
    boolean btn = doc["buttons"][0];

    float thrust_left = doc["right_joystick"][1];
    int escValue = map(thrust_left*100, -100, 100, 1000, 2000);
    servo.write(escValue);
    Serial.println(escValue);
  }
}
void loop() {
    recvWithEndMarker();
    operateServo();
    // showNewData();


    // servo.write(val);
    // delay(300);

}
// void loop() {
//   char str[1024];
//   int i = 0;
//   while(Serial.available() > 0){
//     str[i] = Serial.read();
//     // Serial.println(str[i]);
//     if(str[i]=='q')
//       break;
//     i++;
//     if(i>1022){
//       break;
//     }
    // DynamicJsonDocument doc(2048);
    // Serial1.println(1);
    // deserializeJson(doc, str); 
    // Serial1.println(2);
    // String rj = doc["right_joystick"];
    // Serial1.println(rj);
    


    // float lj_y = doc["left_joystick"][0];
    // Serial1.println(lj_y);
    // boolean btn = doc["buttons"][0];
    // int val = 90 + 90*lj_y;
    // Serial1.println(val);
    // Serial.println(val);
    // servo.write(val);
//     delay(300);



//   }
//   str[i] = '\0';
//   Serial.println(str);
// }
  
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }

// #include<Arduino.h>

// #include <Servo.h>

// Servo motor;

// void setup ()
// {
// motor.attach(9);
// Serial.begin(9600);
// int i = 0;

// //Give some time before you start anything like switching on your ESC / Motor

// Serial.print("Arming Test Starts in ");
// for(i =10; i > 0; i--)
// {
// Serial.print(i);
// Serial.print(".. ");
// }
// // delay(1000);
// Serial.println();

// // Watch for the tone when the ESC gets armed

// for(i = 60; i < 80; i++)
// {
// motor.write(i);
// Serial.println(i);
// delay(500);
// }
// }

// void loop()
// {
//   for(int i=60; i<120; i++){
//     motor.write(i);
//     Serial.println(i);
//     delay(1000);
//   }
// motor.write(80); delay(1000);
// motor.write(91); delay(2000);

// }