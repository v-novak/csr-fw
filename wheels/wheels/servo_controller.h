#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Servo.h>

class ServoController
{
    int _zero_point;
    Servo _servo;

public:
    ServoController() {}
    ~ServoController() {}

    void setup(int pin, int zero_point=90);
    void rotate(int angle); // angle -> zero_point + angle, e.g. rotate(15) -> 105
};

#endif // SERVO_CONTROLLER_H
