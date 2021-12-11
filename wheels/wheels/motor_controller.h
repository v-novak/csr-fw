#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

class MotorController
{
  int _p1;
  int _p2;
  int _p3;
  int _p4;

public:
  MotorController() {}
  ~MotorController() {}

  void setup(int pin1, int pin2, int pin3, int pin4);
  void forward(int speed);
  void reverse(int speed);
};

#endif // MOTOR_CONTROLLER_H
