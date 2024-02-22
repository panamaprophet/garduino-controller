#include <Arduino.h>
#include <Ticker.h>
#include <DHT.h>
#include <stdint.h>
#include <algorithm>

namespace modules {
    typedef void (*thresholdCallback)(float temperature);

    class Sensor {
        private:
            Ticker ticker;
            DHT11 sensor;
            unsigned int retryCount = 5;
            unsigned int retryCounter = 0;
            unsigned int interval = 30 * 1000;
            thresholdCallback onThresholdCallback;

        public:
            unsigned int pin;

            unsigned int thresholdTemperature;

            float temperature;
            float humidity;

            int stabilityFactor = 0;

            Sensor(int _pin);

            void run();
            void onThreshold(thresholdCallback callback);
    };
};
