#include "motor_controller.h"
#import <Arduino.h>

void MotorController::setup(int pin1, int pin2, int pin3, int pin4)
{
  _p1 = pin1;
  _p2 = pin2;
  _p3 = pin3;
  _p4 = pin4;

  pinMode(_p1, OUTPUT);
  digitalWrite(_p1, 0);
  pinMode(_p2, OUTPUT);
  digitalWrite(_p2, 0);
  pinMode(_p3, OUTPUT);
  digitalWrite(_p3, 0);
  pinMode(_p4, OUTPUT);
  digitalWrite(_p4, 0);
}

void MotorController::forward(int speed)
{
  digitalWrite(_p2, 0);
  digitalWrite(_p4, 0);
  delayMicroseconds(5);
  digitalWrite(_p3, 1);
  analogWrite(_p1, speed);
}

void MotorController::reverse(int speed)
{
  digitalWrite(_p1, 0);
  digitalWrite(_p3, 0);
  delayMicroseconds(5);
  digitalWrite(_p2, 1);
  analogWrite(_p4, speed);
}
