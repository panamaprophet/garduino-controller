#include <core/mqtt/mqtt.h>

core::Mqtt::Mqtt(Client& networkClient) {
    client.setClient(networkClient);
};

void core::Mqtt::resubscribe() {
    for (auto const& item : callbacks) {
        client.subscribe(item.first.c_str());
    }
}

void core::Mqtt::subscribe(const char* topic, mqttCallback callback) {
    Serial.printf("[mqtt] subscribing to %s\n", topic);

    callbacks[std::string(topic)] = callback;
    client.subscribe(topic);
}

void core::Mqtt::connect(const char* host, const char* id, uint16 port) {
    _host = strdup(host);
    _id = strdup(id);

    Serial.printf("[mqtt] connecting ... ");

    client.setServer(host, port);

    client.setCallback([&](char* _topic, byte* payload, unsigned int length) {
        std::string topic(_topic);

        auto result = callbacks.find(topic);

        payload[length] = '\0';

        Serial.printf("[mqtt] message from %s: %s\n", _topic, payload);

        if (result != callbacks.end()) {
            result->second(payload, length);
        }

        if (result == callbacks.end()) {
            Serial.printf("[mqtt] callback not found for %s\n", _topic);
        }
        });

    client.connect(id);

    Serial.printf("%s\n", client.connected() ? "success" : "fail");

    if (!client.connected()) {
        Serial.printf("[mqtt] connection failed. error code = %d\n", client.state());
        return;
    }

    resubscribe();
};

void core::Mqtt::publish(const char* topic, const char* payload) {
    Serial.printf("[mqtt] publishing to %s: %s\n", topic, payload);
    client.publish(topic, payload);
};

void core::Mqtt::loop() {
    if (!client.connected()) {
        Serial.printf("[mqtt] connection lost. attempting to reconnect");
        connect(_host, _id);
    }

    client.loop();
};

