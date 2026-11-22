#include <modules/fan/fan.h>
#include <core/logger/logger.h>

namespace {
    constexpr unsigned int PWM_FREQUENCY = 150;
}

modules::Fan::Fan(core::EventBus& eventBus, int pin): Module(eventBus), pin(pin) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, minSpeed);
}

const char* modules::Fan::name() const {
    return "fan";
}

void modules::Fan::apply(const JsonObject& config) {
    defaultSpeed = config["defaultSpeed"] | defaultSpeed;

    setSpeed(defaultSpeed);
}

JsonDocument modules::Fan::getStatus() const {
    JsonDocument status;

    status["speed"] = currentSpeed;

    return status;
}

void modules::Fan::setSpeed(unsigned int speed) {
    auto previousSpeed = currentSpeed;

    currentSpeed = std::min(std::max(minSpeed, speed), maxSpeed);

    core::Logger::info("fan", "speed %d (requested %d)", currentSpeed, speed);

    analogWriteFreq(PWM_FREQUENCY);
    analogWrite(pin, currentSpeed);

    if (currentSpeed != previousSpeed) {
        changed();
    }
}

void modules::Fan::stepUp() {
    setSpeed(currentSpeed + step);
}

void modules::Fan::reset() {
    setSpeed(defaultSpeed);
}
