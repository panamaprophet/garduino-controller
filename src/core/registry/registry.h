#pragma once

#include <vector>
#include <initializer_list>
#include <ArduinoJson.h>
#include <modules/module.h>
#include <core/context/context.h>

namespace core {
    class ModuleRegistry {
        public:
            ModuleRegistry(std::initializer_list<modules::Module*> modules);

            void apply(const JsonObject& config, Context& context);
            void getStatusAll(JsonObject& status) const;

        private:
            std::vector<modules::Module*> modules;
    };
}
