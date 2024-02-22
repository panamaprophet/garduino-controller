#include <modules/sensor/sensor.h>

modules::Sensor::Sensor(int _pin) {
    pin = _pin;
    sensor.setup(pin);

    sensor.onData([&](float newHumidity, float newTemperature) {
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
            onThresholdCallback(newTemperature);
        }

        humidity = newHumidity;
        temperature = newTemperature;
    });

    sensor.onError([&](uint8_t error) {
        retryCounter++;

        if (retryCounter <= retryCount) {
            sensor.read();
        } else {
            retryCounter = 0;
            Serial.printf("[module:sensor] reading error = %s\n", sensor.getError());
        }
    });
};

void modules::Sensor::run() {
    Serial.printf("[module:sensor] run. polling interval set to %d seconds\n", interval / 1000);

    ticker.attach_ms(interval, [&]() {
        sensor.read();
    });
};

void modules::Sensor::onThreshold(thresholdCallback callback) {
    onThresholdCallback = callback;
}
