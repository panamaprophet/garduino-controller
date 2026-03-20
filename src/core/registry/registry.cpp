#include <core/registry/registry.h>

core::ModuleRegistry::ModuleRegistry(std::initializer_list<modules::Module*> modules)
    : modules(modules) {}

void core::ModuleRegistry::apply(const JsonObject& config, Context& context) {
    for (auto* module : modules) {
        JsonObject moduleConfig = config[module->name()];
        module->start(moduleConfig, context);
    }
}

void core::ModuleRegistry::getStatusAll(JsonObject& status) const {
    for (auto* module : modules) {
        JsonObject moduleStatus = status[module->name()].to<JsonObject>();
        module->getStatus(moduleStatus);
    }
}
