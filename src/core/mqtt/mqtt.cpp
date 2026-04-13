#include <core/mqtt/mqtt.h>
#include <core/logger/logger.h>

core::Mqtt::Mqtt(Client& networkClient) {
    client.setClient(networkClient);
};

void core::Mqtt::resubscribe() {
    for (auto const& item : callbacks) {
        client.subscribe(item.first.c_str());
    }
}

void core::Mqtt::subscribe(const char* topic, MqttCallback callback) {
    Logger::info("mqtt", "subscribing to %s", topic);

    callbacks[std::string(topic)] = callback;
    client.subscribe(topic);
}

void core::Mqtt::connect(const char* host, const char* id, uint16 port) {
    _host = strdup(host);
    _id = strdup(id);

    Logger::info("mqtt", "connecting...");

    client.setServer(host, port);

    client.setCallback([this](char* _topic, byte* payload, unsigned int length) {
        std::string topic(_topic);

        auto result = callbacks.find(topic);

        payload[length] = '\0';

        Logger::info("mqtt", "%s: %s", _topic, payload);

        if (result != callbacks.end()) {
            result->second(payload, length);
            return;
        }

        Logger::error("mqtt", "no handler for %s", _topic);
    });

    client.connect(id);

    if (!client.connected()) {
        Logger::error("mqtt", "connection failed (rc=%d)", client.state());
        return;
    }

    Logger::info("mqtt", "connected");

    resubscribe();
};

void core::Mqtt::publish(const char* topic, const char* payload) {
    if (!client.connected()) {
        Logger::error("mqtt", "connection lost, reconnecting...");
        connect(_host, _id);
    }

    Logger::info("mqtt", "publish %s: %s", topic, payload);
    client.publish(topic, payload);
};

void core::Mqtt::loop() {
    if (!client.connected()) {
        Logger::error("mqtt", "connection lost, reconnecting...");
        connect(_host, _id);
    }

    client.loop();
};
