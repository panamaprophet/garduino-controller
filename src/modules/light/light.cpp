#include <modules/light/light.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* EVENT_LIGHT_SWITCH = "light:switch";
    constexpr unsigned int PWM_FREQUENCY = 1000;
    constexpr unsigned long MS_PER_DAY = 86400000;
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
    isOn = config["isOn"].as<bool>();
    duration = config["duration"].as<unsigned long>();
    switchIn = config["switchIn"].as<unsigned long>();

    core::Logger::info("light", "started %s, switching in %lu h", isOn ? "on" : "off", switchIn / 1000 / 60 / 60);

    ticker.attach_ms(checkInterval, [this]() {
        switchIn -= checkInterval;

        if (switchIn >= 0) {
            return;
        }

        isOn = !isOn;
        switchIn = isOn ? duration : MS_PER_DAY - duration;

        core::Logger::info("light", "switched %s, next in %lu h", isOn ? "on" : "off", switchIn / 1000 / 60 / 60);

        analogWriteFreq(PWM_FREQUENCY);
        analogWrite(pin, isOn ? PIN_ON : PIN_OFF);

        eventBus.emit(EVENT_LIGHT_SWITCH, isOn ? "true" : "false");
    });

    analogWriteFreq(PWM_FREQUENCY);
    analogWrite(pin, isOn ? PIN_ON : PIN_OFF);
}

JsonDocument modules::Light::getStatus() const {
    JsonDocument status;
    status["isOn"] = isOn;
    return status;
}
