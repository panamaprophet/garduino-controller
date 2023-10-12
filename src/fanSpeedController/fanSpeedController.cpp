#include <algorithm>
#include <fanSpeedController/fanSpeedController.h>

void FanSpeedController::setSpeed(unsigned int speed) {
    if (speed != currentSpeed){
        currentSpeed = speed;
        analogWrite(pin, currentSpeed);
    }
};

void FanSpeedController::setup(int _pin, unsigned int speed) {
    defaultSpeed = speed;
    currentSpeed = speed;

    pin = _pin;

    analogWrite(pin, currentSpeed);
};

void FanSpeedController::reset() {
    currentSpeed = defaultSpeed;
    analogWrite(pin, currentSpeed);
}

void FanSpeedController::stepUp() {
    setSpeed(std::min(max, currentSpeed + step));
}

void FanSpeedController::stepDown() {
    setSpeed(std::max(min, currentSpeed - step));
}
