#pragma once

#include <map>
#include <string>
#include <vector>

namespace core {
    using EventHandler = void (*)(const char* data);

    class EventBus {
        public:
            void on(const char* event, EventHandler handler);
            void emit(const char* event, const char* data = nullptr);

        private:
            std::map<std::string, std::vector<EventHandler>> listeners;
    };
}
