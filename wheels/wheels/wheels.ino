#include <Servo.h>
#include <Wire.h>

#include "motor_controller.h"
#include "common/comm.h"

/*

  Front left  |-------| Front right
                  |
                  |
    Rear left \---|---\ Rear right

  Angle > 0 => Going to the right => Right is inward, turned to -angle
                                     Left is outward, turned to -adjusted_angle
                                     Front right is slowed down

  Angle < 0 => Going to the left =>  Left is inward, turned to -angle
                                     Right is outward, turned to -adjusted_angle
                                     Front right is slowed down

*/

MotorController motor_right, motor_left;
int speed_left, speed_right;

void i2c_handle(int bytes_available)
{
  Serial.println("recv ");
  //Serial.println(bytes_available);

  I2C_packet input_packet;
  int input_offset = 0;
  for (int i = 0; /*input_offset < sizeof(I2C_packet) && */i < bytes_available; ++input_offset, ++i)
  {
    char c = Wire.read();
    //Serial.print((int)c);
    //Serial.print(" ");
    ((char*)&input_packet)[input_offset] = c;
  }

  if (input_offset < sizeof(I2C_packet)) {
    return;
  }

  input_offset = 0;

  // parse the packet
  switch (input_packet.type) {
    case i2c_packet_command:
        switch (input_packet.command_payload.cmd_type) {
          case i2c_command_wheel_control:
            speed_left = *((int16_t*)&input_packet.command_payload.cmd_args[0]);
            speed_right = *((int16_t*)&input_packet.command_payload.cmd_args[2]) ;

            break;
          default:
            break;
        }
      break;

    default:
      // unknown packet type
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  motor_right.setup(3, 2, 4, 5);
  motor_left.setup(9, 7, 8, 10);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  Wire.begin(i2c_addr_motor_board);
  Wire.onReceive(i2c_handle); // register event
}

void loop()
{
  while (1) {

    Serial.print("Speed: ");
    Serial.print(speed_left);
    Serial.print(", ");
    Serial.println(speed_right);

    if (speed_left >= 0) {
      motor_left.forward(speed_left);
    } else {
      motor_left.reverse(-speed_left);
    }

    if (speed_right >= 0) {
      motor_right.forward(speed_right);
    } else {
      motor_right.reverse(-speed_right);
    }

    delay(100);
  }
}
