#include <core/event-bus/event-bus.h>

void core::EventBus::on(const char* event, EventHandler handler) {
    listeners[std::string(event)].push_back(handler);
}

void core::EventBus::emit(const char* event, const char* data) {
    auto result = listeners.find(std::string(event));

    if (result == listeners.end()) {
        return;
    }

    for (auto handler : result->second) {
        handler(data);
    }
}
