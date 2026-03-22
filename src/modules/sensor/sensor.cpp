#include <modules/sensor/sensor.h>

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
            Serial.printf("[module:sensor] threshold reached\n");
            this->eventBus.emit(EVENT_TEMPERATURE_HIGH);
        }

        humidity = newHumidity;
        temperature = newTemperature;
    });

    sensor.onError([this](uint8_t error) {
        retryCounter++;

        if (retryCounter <= retryCount) {
            sensor.read();
        } else {
            retryCounter = 0;
            Serial.printf("[module:sensor] reading error = %s\n", sensor.getError());
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

    Serial.printf("[module:sensor] started. polling interval = %d seconds\n", interval / 1000);

    ticker.attach_ms(interval, [this]() {
        sensor.read();
    });

    sensor.read();
}

JsonDocument modules::Sensor::getStatus() const {
    JsonDocument status;
    status["temperature"] = temperature;
    status["humidity"] = humidity;
    status["stabilityFactor"] = stabilityFactor;
    return status;
}
