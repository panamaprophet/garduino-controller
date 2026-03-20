#include <core/message-bus/message-bus.h>

core::MessageBus::MessageBus(Mqtt& mqtt, const char* controllerId)
    : mqtt(mqtt), controllerId(controllerId) {}

void core::MessageBus::buildTopic(char* buffer, size_t bufferSize, const char* channel) {
    snprintf(buffer, bufferSize, "controllers/%s/%s", controllerId, channel);
}

void core::MessageBus::publish(const char* channel, const char* payload) {
    char topic[128];
    buildTopic(topic, sizeof(topic), channel);
    mqtt.publish(topic, payload);
}

void core::MessageBus::subscribe(const char* channel, Handler handler) {
    char topic[128];
    buildTopic(topic, sizeof(topic), channel);

    mqtt.subscribe(topic, [handler](byte* payload, unsigned int length) {
        handler(payload, length);
    });
}
