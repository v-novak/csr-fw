class MotorController
{
  int _p1;
  int _p2;
  int _p3;
  int _p4;
  
public:
  MotorController() {}
  ~MotorController() {}
  
  void setup(int pin1, int pin2, int pin3, int pin4) 
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

  void forward(int speed) {
    digitalWrite(_p2, 0);
    digitalWrite(_p4, 0);
    delayMicroseconds(5);
    digitalWrite(_p3, 1);
    analogWrite(_p1, speed);
  }

  void reverse(int speed) {
    digitalWrite(_p1, 0);
    digitalWrite(_p3, 0);
    delayMicroseconds(5);
    digitalWrite(_p2, 1);
    analogWrite(_p4, speed);
  }
};

MotorController motor_right, motor_left;

void setup() {
  Serial.begin(115200);
  motor_right.setup(3, 2, 4, 5);
  motor_left.setup(6, 7, 8, 9);
}

void loop() {
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
    /*speed += dir;
    if (speed <= -255) {
      //delay(3000);
      dir = -dir;
      speed = -255;
    } else if (speed >= 255) {
      //delay(3000);
      dir = -dir;
      speed = 255;
    }*/
  }
}
