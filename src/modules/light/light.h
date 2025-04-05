#include <Arduino.h>
#include <Ticker.h>
#include <DHT.h>
#include <stdint.h>
#include <algorithm>

namespace modules {
    typedef void (*switchCallback)(bool isOn, unsigned long switchIn);

    const unsigned int PIN_ON = 255;
    const unsigned int PIN_OFF = 0;

    class Light {
        private:
            Ticker ticker;
            unsigned long checkInterval = 5 * 1000;
            switchCallback onSwitchCallback;

        public:
            unsigned int pin;

            bool isOn;
            unsigned long duration;
            long switchIn;

            Light(int _pin);

            void run();
            void onSwitch(switchCallback callback);
    };
};
