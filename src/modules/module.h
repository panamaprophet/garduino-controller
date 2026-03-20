#pragma once

#include <ArduinoJson.h>
#include <core/context/context.h>

namespace modules {
    class Module {
        public:
            virtual const char* name() const = 0;
            virtual void start(const JsonObject& config, core::Context& context) = 0;
            virtual void getStatus(JsonObject& status) const = 0;
            virtual ~Module() = default;
    };
}
