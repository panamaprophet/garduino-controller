#include <modules/sensor/sensor.h>
#include <core/logger/logger.h>

modules::Sensor::Sensor(core::EventBus& eventBus, int pin): Module(eventBus), pin(pin) {
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

        overheating = newTemperature >= thresholdTemperature && newTemperature > temperature;

        humidity = newHumidity;
        temperature = newTemperature;
        hasReading = true;
        retryCounter = 0;

        bool isRecovered = isFailing;

        isFailing = false;

        if (isRecovered) {
            core::Logger::info("sensor", "readings recovered");
        }

        if (overheating) {
            core::Logger::info("sensor", "temperature threshold reached");
        }

        if (isRecovered || overheating) {
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
        overheating = false;
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

bool modules::Sensor::isOverheating() const {
    return overheating;
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
