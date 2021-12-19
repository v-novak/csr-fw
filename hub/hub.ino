#include <Wire.h>
#include <Servo.h>

#include "common/comm.h"
#include "common/util.h"
#include "adjustment.h"

const int STEERING_IN_PIN = 2;
const int THROTTLE_IN_PIN = 3;

const int CYCLE_DELAY_MS = 50;

Servo servo_right, servo_left;
const int ANGLE_CENTER_L = 90;
const int ANGLE_CENTER_R = 90;
const int ANGLE_MIN = -35;
const int ANGLE_MAX = 35;
const int MAX_DELTA_ANGLE = 70; // max degrees per second
const int MAX_DELTA_ANGLE_PER_CYCLE = MAX_DELTA_ANGLE * CYCLE_DELAY_MS / 1000;
const int ANGLE_THRESHOLD = 5;

const int SPEED_MIN = -128;
const int SPEED_MAX = 128;
const int MAX_DELTA_SPEED = 500; // max delta speed per second
const int MAX_DELTA_SPEED_PER_CYCLE = MAX_DELTA_SPEED * CYCLE_DELAY_MS / 1000;
const int SPEED_THRESHOLD = 15;


void setup()
{
  // I2C
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  //Wire.setClock(100000);
  Wire.begin(); // join i2c bus (address optional for master)

  pinMode(13, OUTPUT);
  pinMode(STEERING_IN_PIN, INPUT);
  pinMode(THROTTLE_IN_PIN, INPUT);

  servo_left.attach(4);
  servo_right.attach(5);
  servo_right.write(ANGLE_CENTER_R);
  servo_left.write(ANGLE_CENTER_L);


  Serial.begin(115200);
}

I2C_packet packet;

void loop()
{
  smoothened<int8_t> angle(0, MAX_DELTA_ANGLE_PER_CYCLE);
  int target_angle = 0;
  smoothened<int16_t> speed(0, MAX_DELTA_SPEED_PER_CYCLE);
  int16_t speed_left, speed_right;

  recalc_params params;

  while (1)
  {
    int angle_raw = pulseIn(STEERING_IN_PIN, HIGH);
    target_angle = map(angle_raw, 1020, 2000, ANGLE_MIN, ANGLE_MAX);
    target_angle = restrict(target_angle, ANGLE_MIN, ANGLE_MAX, ANGLE_THRESHOLD);

    /*if (target_angle >= -ANGLE_THRESHOLD && target_angle <= ANGLE_THRESHOLD) target_angle = 0;
    if (target_angle > ANGLE_MAX) target_angle = ANGLE_MAX;
    if (target_angle < ANGLE_MIN) target_angle = ANGLE_MIN;*/

    angle.set(target_angle);

    int speed_raw = pulseIn(THROTTLE_IN_PIN, HIGH);
    speed_raw = map(speed_raw, 1013, 1873, SPEED_MIN, SPEED_MAX);

    /*if (speed_raw >= -SPEED_THRESHOLD && speed_raw <= SPEED_THRESHOLD) speed_raw = 0;
    if (speed_raw > SPEED_MAX) speed_raw = SPEED_MAX;
    if (speed_raw < SPEED_MIN) speed_raw = SPEED_MIN;*/

    speed_raw = restrict(speed_raw, SPEED_MIN, SPEED_MAX, SPEED_THRESHOLD);

    speed.set(speed_raw);

    /*Serial.print("ANGLE RAW: ");
    Serial.print(angle_raw);
    Serial.print(" ANGLE: ");
    Serial.print(angle);
    Serial.print(" ");
    Serial.print("THROTTLE RAW: ");
    Serial.print(speed_raw);
    Serial.print(" THROTTLE: ");
    Serial.println(speed);*/

    if (angle < 0) { // turn left
      adjust(-angle, params);
      servo_left.write(ANGLE_CENTER_L + 2 * angle);
      servo_right.write(ANGLE_CENTER_R - 2 * params.outer_angle);
      //Serial.println("left");
      //Serial.println(angle);
      //Serial.println(params.outer_angle);
      speed_right = speed;
      speed_left = speed / params.outer_speed_multiplier;
    } else if (angle > 0) { // turn right
      adjust(angle, params);
      servo_left.write(ANGLE_CENTER_L + 2 * params.outer_angle);
      servo_right.write(ANGLE_CENTER_R + 2 * angle);
      //Serial.println("right");
      //Serial.println(angle);
      //Serial.println(params.outer_angle);
      speed_left = speed;
      speed_right = speed / params.outer_speed_multiplier;
    } else { // go straight
      servo_left.write(ANGLE_CENTER_L);
      servo_right.write(ANGLE_CENTER_R);
      //Serial.println("center");
      speed_right = speed_left = speed;
    }

    packet.type = i2c_packet_command;
    packet.command_payload.cmd_type = i2c_command_wheel_control;
    /*Serial.print("Speed: ");
    Serial.print(speed_left);
    Serial.print(" ");
    Serial.println(speed_right); */
    *((int16_t*)&packet.command_payload.cmd_args[0]) = speed_left;
    *((int16_t*)&packet.command_payload.cmd_args[2]) = speed_right;

    Wire.beginTransmission(i2c_addr_motor_board);
    Wire.write((const char*)&packet, sizeof(I2C_packet));
    Wire.endTransmission();

    packet.command_payload.cmd_type = i2c_command_led_control;
    led_direction dir = LED_DIRECTION_STOP;
    if (speed > 0) dir = LED_DIRECTION_FWD;
    else if (speed < 0) dir = LED_DIRECTION_BACK;
    i2c_led_control_set_direction(packet, dir);

    Wire.beginTransmission(i2c_addr_led_control);
    Wire.write((const char*)&packet, sizeof(I2C_packet));
    Wire.endTransmission();

    delay(CYCLE_DELAY_MS);
  }
}
