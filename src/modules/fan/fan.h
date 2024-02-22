#include <Arduino.h>
#include <Ticker.h>
#include <DHT.h>
#include <stdint.h>
#include <algorithm>

namespace modules {
    class Fan {
        private:
            Ticker ticker;

            void setSpeed(unsigned int speed);

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
};
