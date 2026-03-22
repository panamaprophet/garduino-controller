#include <core/message-bus/message-bus.h>

namespace {
    constexpr const char* TOPIC_FORMAT = "controllers/%s/%s";
    constexpr size_t TOPIC_BUFFER_SIZE = 128;
}

core::MessageBus::MessageBus(Mqtt& mqtt, const char* controllerId)
    : mqtt(mqtt), controllerId(controllerId) {}

void core::MessageBus::buildTopic(char* buffer, size_t bufferSize, const char* channel) {
    snprintf(buffer, bufferSize, TOPIC_FORMAT, controllerId, channel);
}

void core::MessageBus::publish(const char* channel, const char* payload) {
    char topic[TOPIC_BUFFER_SIZE];
    buildTopic(topic, sizeof(topic), channel);
    mqtt.publish(topic, payload);
}

void core::MessageBus::subscribe(const char* channel, MqttCallback handler) {
    char topic[TOPIC_BUFFER_SIZE];
    buildTopic(topic, sizeof(topic), channel);
    mqtt.subscribe(topic, handler);
}
