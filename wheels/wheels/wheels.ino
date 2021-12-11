#include "adjustment.h"
#include "motor_controller.h"
#include "servo_controller.h"

MotorController motor_right, motor_left;
ServoController servo_right, servo_left;

void setup()
{
  Serial.begin(115200);
  motor_right.setup(3, 2, 4, 5);
  motor_left.setup(6, 7, 8, 9);
}

void loop()
{
  int speed = 0;
  int target_speed = 0;
  int dir = -10;
  int sensorValue;

  int max_acc = 10;

  while (1) {
    sensorValue = analogRead(A7);
    target_speed = map(sensorValue, 0, 1023, -255, 255);

    if (speed - target_speed > max_acc ) {
      speed -= max_acc;
    } else if (target_speed - speed > max_acc) {
      speed += max_acc;
    } else {
      speed = target_speed;
    }

    Serial.print(speed);
    Serial.write("\n");

    if (speed < 0) {
      motor_right.forward(-speed);
      motor_left.forward(-speed);
    } else {
      motor_right.reverse(speed);
      motor_left.reverse(speed);
    }

    delay(50);
  }
}
