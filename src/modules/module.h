#pragma once

#include <functional>
#include <ArduinoJson.h>

namespace modules {
    class Module {
        public:
            using ChangeListener = std::function<void(Module& module)>;

            virtual const char* name() const = 0;
            virtual void apply(const JsonObject& config) = 0;
            virtual JsonDocument getStatus() const = 0;
            virtual ~Module() = default;

            void onChange(ChangeListener listener) {
                this->listener = listener;
            }

        protected:
            void changed() {
                if (listener) {
                    listener(*this);
                }
            }

        private:
            ChangeListener listener;
    };
}
