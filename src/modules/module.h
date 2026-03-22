#pragma once

#include <ArduinoJson.h>
#include <core/event-bus/event-bus.h>

namespace modules {
    class Module {
        public:
            Module(core::EventBus& eventBus) : eventBus(eventBus) {}

            virtual const char* name() const = 0;
            virtual void apply(const JsonObject& config) = 0;
            virtual JsonDocument getStatus() const = 0;
            virtual ~Module() = default;

        protected:
            core::EventBus& eventBus;
    };
}
