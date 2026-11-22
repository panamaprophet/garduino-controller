#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <core/message-bus/message-bus.h>
#include <core/registry/registry.h>
#include <core/firmware/firmware.h>
#include <core/scheduler/scheduler.h>
#include <core/protocol/protocol.h>

namespace core {
    class ProtocolV2 : public Protocol {
        public:
            static constexpr int version = 2;

            ProtocolV2(MessageBus& messageBus, ModuleRegistry& registry, Firmware& firmware, Scheduler& scheduler);

            void begin() override;
            void onConnected() override;
            void publishTelemetry() override;
            void publishChange(const char* name) override;

        private:
            MessageBus& messageBus;
            ModuleRegistry& registry;
            Firmware& firmware;
            Scheduler& scheduler;
            bool isBootPublished = false;

            void publish(const char* channel, JsonDocument& payload);
            void publishEvent(const char* type, JsonDocument payload = JsonDocument());
            void publishBoot();
            void publishFirmwareUpdate(const char* status, const char* message = nullptr);
            void requestSettings();

            void onSettings(byte* payload, unsigned int length);
            void onCommand(byte* payload, unsigned int length);

            void updateFirmware(const JsonDocument& command);
            void reboot();
    };
}
