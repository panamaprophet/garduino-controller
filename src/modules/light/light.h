#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <modules/module.h>

namespace modules {
    class Light : public Module {
        public:
            Light(core::EventBus& eventBus, int pin);

            const char* name() const override;
            void apply(const JsonObject& config) override;
            JsonDocument getStatus() const override;

            bool isOn() const;

        private:
            static const unsigned int PIN_ON = 255;
            static const unsigned int PIN_OFF = 0;

            unsigned int pin;
            Ticker ticker;
            unsigned long checkInterval = 5 * 1000;

            bool on = false;
            uint32_t onTime = 12 * 3600;
            uint32_t duration = 12 * 3600;

            bool shouldBeOn() const;
            void update(bool force = false);
    };
}
