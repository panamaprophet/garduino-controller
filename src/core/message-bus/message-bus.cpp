#include <core/message-bus/message-bus.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* TOPIC_FORMAT = "controllers/%s/%s";
    constexpr size_t TOPIC_BUFFER_SIZE = 128;
    constexpr size_t PAYLOAD_BUFFER_SIZE = 384;
}

core::MessageBus::MessageBus(Mqtt& mqtt, const char* controllerId)
    : mqtt(mqtt), controllerId(controllerId) {}

void core::MessageBus::buildTopic(char* buffer, size_t bufferSize, const char* channel) {
    snprintf(buffer, bufferSize, TOPIC_FORMAT, controllerId, channel);
}

void core::MessageBus::publish(const char* channel, JsonDocument& payload) {
    payload["v"] = protocolVersion;

    if (measureJson(payload) >= PAYLOAD_BUFFER_SIZE) {
        Logger::error("bus", "payload for %s exceeds %u bytes, dropped", channel, PAYLOAD_BUFFER_SIZE);
        return;
    }

    char topic[TOPIC_BUFFER_SIZE];
    char buffer[PAYLOAD_BUFFER_SIZE];

    buildTopic(topic, sizeof(topic), channel);
    serializeJson(payload, buffer, sizeof(buffer));

    mqtt.publish(topic, buffer);
}

void core::MessageBus::subscribe(const char* channel, MqttCallback handler) {
    char topic[TOPIC_BUFFER_SIZE];
    buildTopic(topic, sizeof(topic), channel);
    mqtt.subscribe(topic, handler);
}
