#pragma once

#include <ArduinoJson.h>
#include <core/mqtt/mqtt.h>

namespace core {
    class MessageBus {
        public:
            MessageBus(Mqtt& mqtt, const char* controllerId);

            void publish(const char* channel, JsonDocument& payload);
            void subscribe(const char* channel, MqttCallback handler);

        private:
            Mqtt& mqtt;
            const char* controllerId;

            void buildTopic(char* buffer, size_t bufferSize, const char* channel);
    };
}
