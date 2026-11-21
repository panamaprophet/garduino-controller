#pragma once

#include <ArduinoJson.h>
#include <core/event-bus/event-bus.h>

namespace modules {
    class Module {
        public:
            static constexpr const char* CHANGED_EVENT = "module:changed";

            Module(core::EventBus& eventBus) : eventBus(eventBus) {}

            virtual const char* name() const = 0;
            virtual void apply(const JsonObject& config) = 0;
            virtual JsonDocument getStatus() const = 0;
            virtual ~Module() = default;

        protected:
            core::EventBus& eventBus;

            void changed() {
                eventBus.emit(CHANGED_EVENT, name());
            }
    };
}
