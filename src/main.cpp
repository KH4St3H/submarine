#include <Arduino.h>
#include <ArduinoJson.h>
#include <Servo.h>

class Motor{
    public:
        Servo servo;
        int pin;
        int power;
        int base_power = 1500;
        void attach(int n){
            servo.attach(n);
            pin = n;

        }

        void setServo(Servo srv){
            servo = srv;
        }

        void setPower(int n){
            power = base_power + n;
            Serial.println("power: "+power);
            
            servo.write(power);
        }
};

Motor thrustRight;
Motor thrustLeft;

Motor liftLeft0;
Motor liftLeft1;

Motor liftRight0;
Motor liftRight1;

Motor motors[] = {thrustRight, thrustLeft, liftLeft0, liftLeft1, liftRight0, liftRight1};

float pi = 3.14159265359;

const int tilt_power = 200;
const int base = 1500;
boolean titled = false;
boolean lifted = false;
boolean rotated = false;


void setup() {

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

  Serial.begin(9600);

  Serial1.begin(38400);
  Serial.println("init");

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

void rotate(boolean left){
    if(left){
        thrustRight.setPower(tilt_power);
    }
    else
    {
        thrustLeft.setPower(tilt_power);
    }
    rotated = true;
}

void liftUp(){
    liftLeft0.setPower(tilt_power);
    liftLeft1.setPower(tilt_power);
    liftRight0.setPower(tilt_power);
    liftRight1.setPower(tilt_power);
    lifted = true;
}
void liftDown(){
    liftLeft0.setPower(-tilt_power);
    liftLeft1.setPower(-tilt_power);
    liftRight0.setPower(-tilt_power);
    liftRight1.setPower(-tilt_power);
    lifted = true;
}
void resetMotors(){
    liftLeft0.setPower(0);
    liftLeft1.setPower(0);
    liftRight0.setPower(0);
    liftRight1.setPower(0);
    lifted = false;
    rotated = false;
    titled = false;
}

void tilt(float x, float y){
    titled = true;
    if(x > 0.9){
        liftRight0.setPower(tilt_power);
        liftRight1.setPower(tilt_power);
        return;
    }else if (x < -0.9){
        liftLeft0.setPower(tilt_power);
        liftLeft1.setPower(tilt_power);
        return;
    } else if (y > 0.9){
        liftLeft0.setPower(tilt_power);
        liftRight0.setPower(tilt_power);
        return;
    }else if (y < -0.9){
        liftLeft0.setPower(-tilt_power);
        liftRight0.setPower(-tilt_power);
        return;
    }
    y = -y;
    float theta;
    if(x>0.2){
        theta = atan(y/x) - pi/2;
    }else{
        theta = pi / 2;
    }
    float coef = sqrt(pow(x, 2)+pow(y, 2));
    coef = max(coef, 1);
    Serial.println(tilt_power*cos(theta)*coef);
    liftRight0.setPower((int) tilt_power*cos(theta)*coef);
    liftLeft0.setPower((int) tilt_power*sin(theta)*coef);

}

void operateServo(){
  if (newData == true){
    newData = false;
    DynamicJsonDocument doc(2048);
    Serial.println(receivedChars);
    deserializeJson(doc, receivedChars);
    String d = doc["left_joystick"];
    double lj_y = doc["left_joystick"][1];

    if(doc["hats"][1] == 1){
        liftUp();
    }else if(doc["hats"][1] == -1){
        liftDown();
    }else if(lifted){
        resetMotors();
    }
    // boolean buttons[13] = doc["buttons"];
    if(doc["buttons"][4] or doc["buttons"][5]){
        rotate(doc["buttons"][4]);
        return;
    } else if(rotated){
        resetMotors();
    }

    float rj_x = doc["right_joystick"][0];
    float rj_y = doc["right_joystick"][1];

    if((rj_x>0.3) || (rj_y>0.3) || (rj_x < -0.3) || (rj_y < -0.3)){
        tilt(rj_x, rj_y);
        return;
    } // tilt will be checked below

    if(doc["buttons"][0]){
        tilt(0.5, 1);  // it's reversed
        return;
    } else if(doc["buttons"][1]){
        tilt(1, 0.5);
        return;
    } else if(doc["buttons"][2]){
        tilt(0.5, -1);
        return;
    } else if(doc["buttons"][3]){
        tilt(-1, 0.5);
        return;
    } else if(titled){
        resetMotors();
    }




    float thrust_left = doc["right_joystick"][1];
    int escValue = map(lj_y*-100, -100, 100, 1000, 2000);
    // servo.write(escValue);
    thrustLeft.setPower(escValue);
    thrustRight.setPower(escValue);
    Serial.println(escValue);
  }
}
void loop() {
    recvWithEndMarker();
    operateServo();

}