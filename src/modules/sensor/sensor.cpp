#include <modules/sensor/sensor.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* EVENT_TEMPERATURE_HIGH = "temperature:high";
}

modules::Sensor::Sensor(core::EventBus& eventBus, int pin)
    : Module(eventBus), pin(pin) {
    sensor.setup(pin);

    sensor.onData([this](float newHumidity, float newTemperature) {
        if (newTemperature < temperature) {
            stabilityFactor--;
        }

        if (newTemperature > temperature) {
            stabilityFactor++;
        }

        if (newTemperature == temperature) {
            stabilityFactor > 0 ? stabilityFactor-- : stabilityFactor++;
        }

        if (newTemperature >= thresholdTemperature && newTemperature > temperature) {
            core::Logger::info("sensor", "temperature threshold reached");
            this->eventBus.emit(EVENT_TEMPERATURE_HIGH);
        }

        humidity = newHumidity;
        temperature = newTemperature;
        hasReading = true;
        retryCounter = 0;

        if (isFailing) {
            isFailing = false;
            core::Logger::info("sensor", "readings recovered");
            changed();
        }
    });

    sensor.onError([this](uint8_t) {
        retryCounter++;

        if (retryCounter <= retryCount) {
            sensor.read();
            return;
        }

        retryCounter = 0;
        error = sensor.getError();

        core::Logger::error("sensor", "read failed: %s", error);

        if (!isFailing) {
            isFailing = true;
            changed();
        }
    });
}

const char* modules::Sensor::name() const {
    return "sensor";
}

void modules::Sensor::apply(const JsonObject& config) {
    if (config["thresholdTemperature"].is<unsigned int>()) {
        thresholdTemperature = config["thresholdTemperature"].as<unsigned int>();
    }

    core::Logger::info("sensor", "started, polling every %d s", interval / 1000);

    ticker.attach_ms(interval, [this]() {
        sensor.read();
    });

    sensor.read();
}

JsonDocument modules::Sensor::getStatus() const {
    JsonDocument status;

    if (isFailing || !hasReading) {
        status["error"] = error;
        return status;
    }

    status["temperature"] = temperature;
    status["humidity"] = humidity;
    status["stabilityFactor"] = stabilityFactor;
    return status;
}
