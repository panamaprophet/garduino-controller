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
            void onChange(modules::Module::ChangeListener listener);
            JsonDocument getStatus() const;
            std::vector<const char*> names() const;

        private:
            std::vector<modules::Module*> modules;
            std::vector<modules::Module::ChangeListener> listeners;
    };
}
