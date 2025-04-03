#include <Arduino.h>
#include <map>
#include <string>
#include <PubSubClient.h>

namespace core {
    typedef void (*mqttCallback)(byte* payload, unsigned int length);

    class Mqtt {
        private:
            PubSubClient client;
            std::map<std::string, mqttCallback> callbacks;
            char* _host;
            char* _id;

            void resubscribe();

        public:
            Mqtt(Client& networkClient);

            void subscribe(const char* topic, mqttCallback callback);
            void connect(const char* host, const char* id, uint16 port = 8883);
            void publish(const char* topic, const char* payload = "{}");
            void loop();
    };
}
