#include "sonar.h"
#include <Arduino.h>

Sonar::Sonar(int pin_trig, int pin_echo): trig(pin_trig), echo(pin_echo)
{
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

int Sonar::measure()
{
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  //delay(30);
  int duration = pulseIn(echo, HIGH);
  return duration;// / 58;
}
