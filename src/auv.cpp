#include <Arduino.h>
#include "auv.h"
#include "movements.h"
#include "utils.h"


void forward(int seconds){
    log("forwarding");
    log(seconds);
    thrust(-0.9);
    delay(seconds*1000);
    thrust(0);
}
