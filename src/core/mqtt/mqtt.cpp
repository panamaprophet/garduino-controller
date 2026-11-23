#include <core/mqtt/mqtt.h>
#include <core/logger/logger.h>

namespace {
    constexpr uint16_t BUFFER_SIZE = 512;
    constexpr unsigned long RECONNECT_INTERVAL_MS = 5 * 1000;
}

core::Mqtt::Mqtt(Client& networkClient) {
    client.setClient(networkClient);
    client.setBufferSize(BUFFER_SIZE);

    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        Logger::info("mqtt", "%s: %.*s", topic, static_cast<int>(length), reinterpret_cast<const char*>(payload));

        auto result = callbacks.find(topic);

        if (result == callbacks.end()) {
            Logger::error("mqtt", "no handler for %s", topic);
            return;
        }

        result->second(payload, length);
    });
};

void core::Mqtt::subscribe(const char* topic, MqttCallback callback) {
    Logger::info("mqtt", "subscribing to %s", topic);

    callbacks[std::string(topic)] = callback;
    client.subscribe(topic);
}

void core::Mqtt::onConnect(ConnectCallback callback) {
    connectCallback = callback;
}

void core::Mqtt::connect(const char* host, const char* id, uint16 port) {
    this->id = id;

    client.setServer(host, port);

    reconnect();
};

void core::Mqtt::reconnect() {
    if (
        lastAttempt > 0 &&
        millis() - lastAttempt < RECONNECT_INTERVAL_MS
    ) {
        return;
    }

    lastAttempt = millis();

    Logger::info("mqtt", "connecting...");

    if (!client.connect(id)) {
        Logger::error("mqtt", "connection failed (rc=%d), retrying in %lu s", client.state(), RECONNECT_INTERVAL_MS / 1000);
        return;
    }

    Logger::info("mqtt", "connected");

    resubscribe();

    if (connectCallback) {
        connectCallback();
    }
}

void core::Mqtt::resubscribe() {
    for (auto const& item : callbacks) {
        client.subscribe(item.first.c_str());
    }
}

void core::Mqtt::publish(const char* topic, const char* payload) {
    if (!client.connected()) {
        reconnect();
    }

    if (!client.connected()) {
        Logger::error("mqtt", "not connected, publish to %s dropped", topic);
        return;
    }

    Logger::info("mqtt", "publish %s: %s", topic, payload);

    if (!client.publish(topic, payload)) {
        Logger::error("mqtt", "publish to %s failed", topic);
    }
};

void core::Mqtt::loop() {
    if (!client.connected()) {
        reconnect();
        return;
    }

    client.loop();
};
