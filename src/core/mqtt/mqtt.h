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
            std::string _host;
            std::string _id ;

            void resubscribe();

        public:
            Mqtt(Client& networkClient);

            void subscribe(std::string topic, mqttCallback callback);
            void connect(std::string host, std::string id, uint16 port = 8883);
            void publish(std::string topic, std::string payload = "{}");
            void loop();
    };
}
