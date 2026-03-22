#include <modules/fan/fan.h>
#include <core/logger/logger.h>

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
}

JsonDocument modules::Fan::getStatus() const {
    JsonDocument status;
    status["currentSpeed"] = currentSpeed;
    return status;
}

void modules::Fan::setSpeed(unsigned int speed) {
    currentSpeed = std::min(std::max(minSpeed, speed), maxSpeed);

    core::Logger::info("fan", "speed %d (requested %d)", currentSpeed, speed);

    analogWriteFreq(PWM_FREQUENCY);
    analogWrite(pin, currentSpeed);
}

void modules::Fan::stepUp() {
    setSpeed(currentSpeed + step);
}

void modules::Fan::reset() {
    setSpeed(defaultSpeed);
}
