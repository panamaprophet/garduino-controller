#include <modules/fan/fan.h>

namespace {
    constexpr unsigned int PWM_FREQUENCY = 150;
}

modules::Fan::Fan(core::EventBus& eventBus, int pin)
    : Module(eventBus), pin(pin) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, minSpeed);
}

const char* modules::Fan::name() const {
    return "fan";
}

void modules::Fan::apply(const JsonObject& config) {
    if (config["defaultSpeed"].is<unsigned int>()) {
        defaultSpeed = config["defaultSpeed"].as<unsigned int>();
    }

    setSpeed(defaultSpeed);

    Serial.printf("[module:fan] started with speed %d\n", defaultSpeed);
}

JsonDocument modules::Fan::getStatus() const {
    JsonDocument status;
    status["currentSpeed"] = currentSpeed;
    return status;
}

void modules::Fan::setSpeed(unsigned int speed) {
    currentSpeed = std::min(std::max(minSpeed, speed), maxSpeed);

    Serial.printf("[module:fan] set speed to %d (requested %d)\n", currentSpeed, speed);

    analogWriteFreq(PWM_FREQUENCY);
    analogWrite(pin, currentSpeed);
}

void modules::Fan::stepUp() {
    setSpeed(currentSpeed + step);
}

void modules::Fan::reset() {
    setSpeed(defaultSpeed);
}
