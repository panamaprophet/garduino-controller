#pragma once

#include <Arduino.h>
#include <algorithm>
#include <modules/module.h>

namespace modules {
    class Fan : public Module {
        public:
            Fan(core::EventBus& eventBus, int pin);

            const char* name() const override;
            void apply(const JsonObject& config) override;
            JsonDocument getStatus() const override;

            void stepUp();
            void reset();

        private:
            unsigned int pin;

            unsigned int currentSpeed = 0;
            unsigned int maxSpeed = 255;
            unsigned int minSpeed = 0;
            unsigned int defaultSpeed = 125;
            unsigned int step = 50;

            void setSpeed(unsigned int speed);
    };
}
