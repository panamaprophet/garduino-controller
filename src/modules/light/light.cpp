#include <time.h>
#include <modules/light/light.h>
#include <modules/light/schedule.h>
#include <core/logger/logger.h>

namespace {
    constexpr unsigned int PWM_FREQUENCY = 1000;
    constexpr uint32_t MS_PER_SECOND = 1000;
}

modules::Light::Light(core::EventBus& eventBus, int pin)
    : Module(eventBus), pin(pin) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, PIN_OFF);
}

const char* modules::Light::name() const {
    return "light";
}

void modules::Light::apply(const JsonObject& config) {
    uint32_t parsedOnTime;

    if (schedule::parseTimeOfDay(config["onTime"], parsedOnTime)) {
        onTime = parsedOnTime;
    }

    if (config["durationMs"].is<uint32_t>()) {
        duration = config["durationMs"].as<uint32_t>() / MS_PER_SECOND;
    }

    auto hours = static_cast<unsigned int>(onTime / 3600);
    auto minutes = static_cast<unsigned int>(onTime % 3600 / 60);
    auto durationMinutes = static_cast<unsigned int>(duration / 60);

    core::Logger::info("light", "on at %02u:%02u UTC for %u min", hours, minutes, durationMinutes);

    ticker.attach_ms(checkInterval, [this]() { update(); });

    update(true);
}

void modules::Light::update(bool force) {
    auto now = static_cast<uint32_t>(time(nullptr));
    auto shouldBeOn = schedule::isActive(now, onTime, duration);
    auto isSwitched = shouldBeOn != on;

    if (!isSwitched && !force) {
        return;
    }

    on = shouldBeOn;

    analogWriteFreq(PWM_FREQUENCY);
    analogWrite(pin, on ? PIN_ON : PIN_OFF);

    if (isSwitched) {
        core::Logger::info("light", "switched %s", on ? "on" : "off");
        changed();
    }
}

bool modules::Light::isOn() const {
    return on;
}

JsonDocument modules::Light::getStatus() const {
    JsonDocument status;
    status["isOn"] = on;
    return status;
}
