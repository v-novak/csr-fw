#ifndef SONAR_H
#define SONAR_H

class Sonar
{
public:
    Sonar() = delete;
    Sonar(int pin_trig, int pin_echo);
    ~Sonar() {}

    int measure(); // returns distance in cm

private:
    int trig;
    int echo;
};

#endif // SONAR_H
