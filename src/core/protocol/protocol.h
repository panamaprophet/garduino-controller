#pragma once

#include <functional>
#include <ArduinoJson.h>
#include <core/message-bus/message-bus.h>
#include <core/registry/registry.h>
#include <core/firmware/firmware.h>
#include <core/scheduler/scheduler.h>

namespace core {
    class Protocol {
        public:
            using MessageHandler = std::function<void(JsonDocument& message)>;

            Protocol(int version, MessageBus& messageBus, ModuleRegistry& registry, Firmware& firmware, Scheduler& scheduler);
            virtual ~Protocol() = default;

            virtual void begin() = 0;
            virtual void onConnected() = 0;

        protected:
            const int version;

            MessageBus& messageBus;
            ModuleRegistry& registry;
            Firmware& firmware;
            Scheduler& scheduler;

            void publish(const char* channel, JsonDocument& payload);
            void subscribe(const char* channel, MessageHandler handler);
    };
}
