#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <time.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

namespace core {
    class Config {
        public:
            std::string ssid;
            std::string password;
            std::string host;
            std::string controllerId;

            Config();
    };

    class Network {
        private:
            void setRootCertificate();
            void setClientCertificate();

        public:
            BearSSL::WiFiClientSecure client;

            Network();

            void connect(std::string ssid, std::string password);
    };

    class Mqtt {
        private:
            PubSubClient client;

            std::vector<std::string> topics;

            std::string _host;
            std::string _id ;

            void resubscribe();

        public:
            Mqtt(Client& networkClient);

            void subscribe(std::string topic);
            void connect(std::string host, std::string id, uint16 port = 8883);
            void publish(std::string topic, std::string payload = "{}");
            void loop();
    };

    class Time {
        public:
            time_t sync();
    };
};
