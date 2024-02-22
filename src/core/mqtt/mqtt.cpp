#include <core/mqtt/mqtt.h>

core::Mqtt::Mqtt(Client& networkClient) {
    client.setClient(networkClient);
};

void core::Mqtt::resubscribe() {
    for (auto const& item: callbacks) {
        client.subscribe(item.first.c_str());
    }
}

void core::Mqtt::subscribe(std::string topic, mqttCallback callback) {
    Serial.printf("[mqtt] subscribing to %s\n", topic.c_str());

    callbacks.insert_or_assign(topic, callback);
    client.subscribe(topic.c_str());
}

void core::Mqtt::connect(std::string host, std::string id, uint16 port) {
    _host = host;
    _id = id;

    Serial.printf("[mqtt] connecting ... ");

    client.setServer(host.c_str(), port);

    client.setCallback([&](char* _topic, byte* payload, unsigned int length) {
        const std::string topic(_topic);
        auto result = callbacks.find(topic);

        payload[length] = '\0';

        Serial.printf("[mqtt] message from %s: %s\n", _topic, payload);

        if (result != callbacks.end()) {
            result -> second(payload, length);
        }
    });

    client.connect(id.c_str());

    Serial.printf("%s\n", client.connected() ? "success" : "fail");

    if (!client.connected()) {
        Serial.printf("[mqtt] connection failed. error code = %d\n", client.state());
        return;
    }

    resubscribe();
};

void core::Mqtt::publish(std::string topic, std::string payload) {
    Serial.printf("[mqtt] publishing to %s: %s\n", topic.c_str(), payload.c_str());
    client.publish(topic.c_str(), payload.c_str());
};

void core::Mqtt::loop() {
    if (!client.connected()) {
        Serial.printf("[mqtt] connection lost. attempting to reconnect");
        connect(_host, _id);
    }

    client.loop();
};

