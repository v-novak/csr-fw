#include <Wire.h>
#include <Servo.h>

#include "common/comm.h"
#include "common/util.h"
#include "adjustment.h"
#include "sonar.h"

const int STEERING_IN_PIN = 2;
const int THROTTLE_IN_PIN = 3;

const int CYCLE_DELAY_MS = 50;

Servo servo_right, servo_left;
const int ANGLE_CENTER_L = 90;
const int ANGLE_CENTER_R = 90;
const int ANGLE_MIN = -33;
const int ANGLE_MAX = 33;
const int MAX_DELTA_ANGLE = 80; // max degrees per second
const int MAX_DELTA_ANGLE_PER_CYCLE = MAX_DELTA_ANGLE * CYCLE_DELAY_MS / 1000;
const int ANGLE_THRESHOLD = 5;

const int SPEED_MIN = -128;
const int SPEED_MAX = 128;
const int MAX_DELTA_SPEED = 500; // max delta speed per second
const int MAX_DELTA_SPEED_PER_CYCLE = MAX_DELTA_SPEED * CYCLE_DELAY_MS / 1000;
const int SPEED_THRESHOLD = 60;

template <typename T, size_t sz>
class window_avg
{
  
  T _win[sz] = {0};
  T _sum = 0;
  T _n = 0;
  
public:
  window_avg() {}
  
  T pulse(T value) {
    _sum -= _win[_n % sz];
    _win[_n % sz] = value;
    _sum += value;
    
    ++_n;
    
    if (_n < sz) return value;
    else return _sum / sz;
  }
};



void setup()
{
  // I2C
  // pinMode(20, INPUT_PULLUP);
  // pinMode(21, INPUT_PULLUP);
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
Sonar sonar_left(22, 23);
Sonar sonar_right(24, 25);

int I2C_ClearBus() 
{
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif
  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}

void send_packet(const I2C_packet& pkt, I2C_addr addr)
{
  if (digitalRead(SDA) == LOW) {
    if (I2C_ClearBus() != 0) {
      Serial.println("ERROR: I2C bus is stuck");
      return;
    }
  }
  
  Wire.beginTransmission(addr);
  Wire.write((const char*)&pkt, sizeof(I2C_packet));
  Wire.endTransmission();
  //delayMicroseconds(10);
}

void loop()
{
  smoothened<int8_t> angle(0, MAX_DELTA_ANGLE_PER_CYCLE);
  int target_angle = 0;
  smoothened<int16_t> speed(0, MAX_DELTA_SPEED_PER_CYCLE);
  int16_t speed_left, speed_right;

  recalc_params params;

  int distance_left;
  int distance_right;

  window_avg<long, 8> avg_angle, avg_speed;
  
  while (1)
  {
    //distance_left = sonar_left.measure();
    distance_right = sonar_right.measure();

    // Serial.print("Left distance: ");
    //Serial.print(distance_left);
    //Serial.print(" ");
    Serial.println(distance_right);
      
    int angle_raw = pulseIn(STEERING_IN_PIN, HIGH);
    target_angle = map(angle_raw, 1013, 1790, ANGLE_MIN, ANGLE_MAX);
    //Serial.print(angle_raw);
    //Serial.print(" "); 
    target_angle = avg_angle.pulse(restrict(target_angle, ANGLE_MIN, ANGLE_MAX, ANGLE_THRESHOLD));
    //Serial.print(target_angle);
    //Serial.print(" "); 

    /*if (target_angle >= -ANGLE_THRESHOLD && target_angle <= ANGLE_THRESHOLD) target_angle = 0;
    if (target_angle > ANGLE_MAX) target_angle = ANGLE_MAX;
    if (target_angle < ANGLE_MIN) target_angle = ANGLE_MIN;*/

    angle.set(target_angle);

    int speed_raw = pulseIn(THROTTLE_IN_PIN, HIGH);
    speed_raw = avg_speed.pulse(speed_raw);
    speed_raw = map(speed_raw, 1020, 1800, SPEED_MIN, SPEED_MAX);

    /*if (speed_raw >= -SPEED_THRESHOLD && speed_raw <= SPEED_THRESHOLD) speed_raw = 0;
    if (speed_raw > SPEED_MAX) speed_raw = SPEED_MAX;
    if (speed_raw < SPEED_MIN) speed_raw = SPEED_MIN;*/

    speed_raw = restrict(speed_raw, SPEED_MIN, SPEED_MAX, SPEED_THRESHOLD);

    speed.set(speed_raw);

    //Serial.print("ANGLE RAW: ");
    
    //Serial.print(angle.get());
    //Serial.print(" ");
    //Serial.println(speed_raw);
    /*
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

    //send_packet(packet, i2c_addr_motor_board);
    /*Wire.beginTransmission(i2c_addr_motor_board);
    Wire.write((const char*)&packet, sizeof(I2C_packet));
    Wire.endTransmission();*/

    /*packet.command_payload.cmd_type = i2c_command_led_control;
    led_direction dir = LED_DIRECTION_STOP;
    if (speed > 0) dir = LED_DIRECTION_FWD;
    else if (speed < 0) dir = LED_DIRECTION_BACK;
    i2c_led_control_set_direction(packet, dir);

    //send_packet(packet, i2c_addr_led_control);*/
    /*Wire.beginTransmission(i2c_addr_led_control);
    Wire.write((const char*)&packet, sizeof(I2C_packet));
    Wire.endTransmission();*/

    delay(CYCLE_DELAY_MS);
  }
}
