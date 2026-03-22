#include <core/registry/registry.h>

core::ModuleRegistry::ModuleRegistry(std::initializer_list<modules::Module*> modules)
    : modules(modules) {}

void core::ModuleRegistry::apply(const JsonObject& config) {
    for (auto* module : modules) {
        module->apply(config[module->name()]);
    }
}

JsonDocument core::ModuleRegistry::getStatus(std::initializer_list<const char*> names) const {
    JsonDocument status;

    bool hasFilter = names.size() > 0;

    for (auto* module : modules) {
        if (hasFilter) {
            bool matched = false;

            for (auto* name : names) {
                if (strcmp(module->name(), name) == 0) {
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                continue;
            }
        }

        status[module->name()].set(module->getStatus());
    }

    return status;
}
