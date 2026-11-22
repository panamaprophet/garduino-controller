#include <modules/light/light.h>
#include <core/logger/logger.h>
#include <core/time/time.h>

namespace {
    constexpr unsigned int PWM_FREQUENCY = 1000;
    constexpr uint32_t MS_PER_SECOND = 1000;
}

modules::Light::Light(core::EventBus& eventBus, int pin): Module(eventBus), pin(pin) {
    pinMode(pin, OUTPUT);
    analogWrite(pin, PIN_OFF);
}

const char* modules::Light::name() const {
    return "light";
}

void modules::Light::apply(const JsonObject& config) {
    uint32_t parsedOnTime;

    if (core::Time::parseTimeToSeconds(config["onTime"], parsedOnTime)) {
        onTime = parsedOnTime;
    }

    if (config["durationMs"].is<uint32_t>()) {
        duration = config["durationMs"].as<uint32_t>() / MS_PER_SECOND;
    }

    auto hours = static_cast<unsigned int>(onTime / 3600);
    auto minutes = static_cast<unsigned int>(onTime % 3600 / 60);
    auto durationMinutes = static_cast<unsigned int>(duration / 60);

    core::Logger::info("light", "on at %02u:%02u UTC for %u min", hours, minutes, durationMinutes);

    ticker.attach_ms_scheduled(checkInterval, [this]() { update(); });

    update(true);
}

bool modules::Light::nextState() const {
    if (duration == 0) {
        return false;
    }

    if (duration >= core::Time::SECONDS_PER_DAY) {
        return true;
    }

    auto now = core::Time::secondsToday();
    auto end = (onTime + duration) % core::Time::SECONDS_PER_DAY;

    if (onTime < end) {
        return now >= onTime && now < end;
    }

    return now >= onTime || now < end;
}

void modules::Light::update(bool force) {
    auto next = nextState();
    auto isSwitched = next != on;

    if (!isSwitched && !force) {
        return;
    }

    on = next;

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
