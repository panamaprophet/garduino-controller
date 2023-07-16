#include <mqtt/mqtt.h>

void Mqtt::connect(WiFiClientSecure& wifiClient, const char* host, const char* id, uint16_t port) {
    client.setClient(wifiClient);
    client.setServer(host, port);

    client.setCallback([&](char* topic, byte* payload, unsigned int length) {
        payload[length] = '\0';

        const char* payloadString = (char*)payload;
        const auto& cb = callbacks.find(topic);

        Serial.printf("received from %s: %s\n", topic, payloadString);

        if (cb == callbacks.end()) {
            Serial.println("no callbacks found");
            return;
        }

        cb -> second(topic, payloadString);
    });

    Serial.println("mqtt connecting");

    client.connect(id);

    if (!client.connected()) {
        Serial.printf("mqtt connection failed. error code = %d\r\n", client.state());

        char sslErrorMessage[80];
	    int sslErrorCode = wifiClient.getLastSSLError(sslErrorMessage, sizeof(sslErrorMessage));

        Serial.printf("ssl error: %d: %s\r\n", sslErrorCode, sslErrorMessage);

        return;
    }

    Serial.println("mqtt connected");
};

void Mqtt::subscribe(const char* topic, const callback& cb) {
    Serial.printf("subscribe to %s\n", topic);

    client.subscribe(topic);

    callbacks.insert({ topic, cb });
};

void Mqtt::publish(const char* topic, const char* message) {
    Serial.printf("publish to %s: %s\n", topic, message);

    client.publish(topic, message);
};

void Mqtt::loop() {
    client.loop();
}
