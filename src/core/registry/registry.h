#pragma once

#include <vector>
#include <initializer_list>
#include <ArduinoJson.h>
#include <modules/module.h>

namespace core {
    class ModuleRegistry {
        public:
            ModuleRegistry(std::initializer_list<modules::Module*> modules);

            void apply(const JsonObject& config);
            JsonDocument getStatus(std::initializer_list<const char*> names = {}) const;

        private:
            std::vector<modules::Module*> modules;
    };
}
