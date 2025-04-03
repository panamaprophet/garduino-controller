#include <modules/fan/fan.h>

modules::Fan::Fan(int _pin) {
    pin = _pin;
    pinMode(pin, OUTPUT);
    analogWrite(pin, minSpeed);
};

void modules::Fan::setSpeed(unsigned int speed) {
    currentSpeed = std::min(std::max(minSpeed, speed), maxSpeed);

    Serial.printf("[module:fan] set speed to %d (requested %d)\n", currentSpeed, speed);

    analogWriteFreq(150);
    analogWrite(pin, currentSpeed);
}

void modules::Fan::run() {
    Serial.printf("[module:fan] run\n");

    setSpeed(defaultSpeed);
};

void modules::Fan::stepUp() {
    setSpeed(currentSpeed + step);
};

void modules::Fan::reset() {
    setSpeed(defaultSpeed);
};
