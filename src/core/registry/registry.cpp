#include <core/registry/registry.h>

core::ModuleRegistry::ModuleRegistry(std::initializer_list<modules::Module*> modules)
    : modules(modules) {
    for (auto* module : this->modules) {
        module->onChange([this](modules::Module& changed) {
            for (auto& listener : listeners) {
                listener(changed);
            }
        });
    }
}

void core::ModuleRegistry::apply(const JsonObject& config) {
    for (auto* module : modules) {
        module->apply(config[module->name()]);
    }
}

void core::ModuleRegistry::onChange(modules::Module::ChangeListener listener) {
    listeners.push_back(listener);
}

JsonDocument core::ModuleRegistry::getStatus() const {
    JsonDocument status;

    for (auto* module : modules) {
        status[module->name()].set(module->getStatus());
    }

    return status;
}

std::vector<const char*> core::ModuleRegistry::names() const {
    std::vector<const char*> result;

    for (auto* module : modules) {
        result.push_back(module->name());
    }

    return result;
}
