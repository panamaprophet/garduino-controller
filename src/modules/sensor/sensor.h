#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <DHT.h>
#include <modules/module.h>

namespace modules {
    class Sensor : public Module {
        public:
            Sensor(core::EventBus& eventBus, int pin);

            const char* name() const override;
            void apply(const JsonObject& config) override;
            JsonDocument getStatus() const override;

        private:
            unsigned int pin;
            Ticker ticker;
            DHT11 sensor;

            unsigned int retryCount = 5;
            unsigned int retryCounter = 0;
            unsigned int interval = 30 * 1000;
            unsigned int thresholdTemperature = 0;

            float temperature = 0;
            float humidity = 0;
            int stabilityFactor = 0;
    };
}
