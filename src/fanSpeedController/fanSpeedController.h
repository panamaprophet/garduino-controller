#include <Arduino.h>

class FanSpeedController {
  public:
    int pin = -1;

    unsigned int currentSpeed = 128;
    unsigned int defaultSpeed = 128;

    unsigned int step = 50;
    unsigned int max = 255;
    unsigned int min = 0;

    void setup(int _pin, unsigned int speed);
    void setSpeed(unsigned int speed);
    void reset();
    void stepUp();
    void stepDown();
};
