#pragma once

#include <functional>
#include <core/mqtt/mqtt.h>

namespace core {
    class MessageBus {
        public:
            using Handler = std::function<void(byte* payload, unsigned int length)>;

            MessageBus(Mqtt& mqtt, const char* controllerId);

            void publish(const char* channel, const char* payload = "{}");
            void subscribe(const char* channel, Handler handler);

        private:
            Mqtt& mqtt;
            const char* controllerId;

            void buildTopic(char* buffer, size_t bufferSize, const char* channel);
    };
}
