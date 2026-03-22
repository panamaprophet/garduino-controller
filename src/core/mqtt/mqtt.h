#pragma once

#include <Arduino.h>
#include <map>
#include <string>
#include <PubSubClient.h>

namespace core {
    typedef void (*MqttCallback)(byte* payload, unsigned int length);

    class Mqtt {
        private:
            PubSubClient client;
            std::map<std::string, MqttCallback> callbacks;
            char* _host;
            char* _id;

            void resubscribe();

        public:
            Mqtt(Client& networkClient);

            void subscribe(const char* topic, MqttCallback callback);
            void connect(const char* host, const char* id, uint16 port = 8883);
            void publish(const char* topic, const char* payload = "{}");
            void loop();
    };
}
