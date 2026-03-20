#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace core {
    class EventBus {
        public:
            using Handler = std::function<void(const char* data)>;

            void on(const char* event, Handler handler);
            void emit(const char* event, const char* data = nullptr);

        private:
            std::map<std::string, std::vector<Handler>> listeners;
    };
}
