#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <core/protocol/protocol.h>

namespace core {
    class ProtocolV2 : public Protocol {
        public:
            ProtocolV2(
                MessageBus& messageBus,
                ModuleRegistry& registry,
                Firmware& firmware,
                Scheduler& scheduler
            );

            void begin() override;
            void onConnected() override;

        private:
            bool isBootPublished = false;

            void publishTelemetry();
            void publishChange(modules::Module& module);
            void publishEvent(const char* type, JsonDocument payload = JsonDocument());
            void publishBoot();
            void publishFirmwareUpdate(const char* status, const char* message = nullptr);
            void requestSettings();

            void onSettings(JsonDocument& settings);
            void onCommand(JsonDocument& command);

            void updateFirmware(const JsonDocument& command);
            void reboot();
    };
}
