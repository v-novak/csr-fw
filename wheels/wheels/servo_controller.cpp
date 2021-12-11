#include "servo_controller.h"


void ServoController::setup(int pin, int zero_point)
{
    _servo.attach(pin);
    _zero_point = zero_point;
    _servo.write(zero_point);
}

void ServoController::rotate(int angle)
{
    _servo.write(angle + _zero_point);
}
