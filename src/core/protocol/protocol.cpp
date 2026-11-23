#include <core/protocol/protocol.h>
#include <core/logger/logger.h>

core::Protocol::Protocol(int version, MessageBus& messageBus, ModuleRegistry& registry, Firmware& firmware, Scheduler& scheduler)
    : version(version), messageBus(messageBus), registry(registry), firmware(firmware), scheduler(scheduler) {}

void core::Protocol::publish(const char* channel, JsonDocument& payload) {
    payload["v"] = version;
    messageBus.publish(channel, payload);
}

void core::Protocol::subscribe(const char* channel, MessageHandler handler) {
    messageBus.subscribe(channel, [channel, handler](byte* payload, unsigned int length) {
        JsonDocument message;
        DeserializationError error = deserializeJson(message, payload, length);

        if (error) {
            Logger::error("protocol", "invalid json on %s: %s", channel, error.c_str());
            return;
        }

        handler(message);
    });
}
