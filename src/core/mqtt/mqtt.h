#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <string>
#include <PubSubClient.h>

namespace core {
    using MqttCallback = std::function<void(byte* payload, unsigned int length)>;
    using ConnectCallback = std::function<void()>;

    class Mqtt {
        private:
            PubSubClient client;
            std::map<std::string, MqttCallback> callbacks;
            ConnectCallback connectCallback;
            const char* id = nullptr;
            unsigned long lastAttempt = 0;

            void reconnect();
            void resubscribe();

        public:
            Mqtt(Client& networkClient);

            void subscribe(const char* topic, MqttCallback callback);
            void onConnect(ConnectCallback callback);
            void connect(const char* host, const char* id, uint16 port = 8883);
            void publish(const char* topic, const char* payload = "{}");
            void loop();
    };
}
