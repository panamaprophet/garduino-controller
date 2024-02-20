#include <Arduino.h>
#include <Ticker.h>
#include <DHT.h>
#include <stdint.h>

namespace modules {
    class Fan {
        private:
            Ticker ticker;

        public:
            unsigned int pin;

            unsigned int currentSpeed = 0;
            unsigned int maxSpeed = 255;
            unsigned int minSpeed = 0;
            unsigned int defaultSpeed = 120;
            unsigned int step = 50;

            Fan(int _pin);

            void run();
            void stepUp();
            void reset();
    };

    class Light {
        private:
            Ticker ticker;
            unsigned long checkInterval = 5 * 1000;

        public:
            unsigned int pin;

            bool isOn;
            unsigned long duration;
            unsigned long switchIn;

            Light(int _pin);

            void run();
    };

    typedef void readCallback(float humidity, float temperature);
    typedef void errorCallback(unsigned int error);

    class Sensor {
        private:
            Ticker ticker;
            DHT11 sensor;
            unsigned int retryCount = 5;
            unsigned int retryCounter = 0;
            unsigned int interval = 30 * 1000;

        public:
            unsigned int pin;

            unsigned int thresholdTemperature;

            float temperature;
            float humidity;

            int stabilityFactor = 0;

            Sensor(int _pin);

            void run();
    };
};
