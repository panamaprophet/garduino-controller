#include <ArduinoJson.h>
#include <string>
#include <map>
#include <time.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <stdarg.h>
#include <Ticker.h>

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

    class Time {
        public:
            time_t sync();
    };

    typedef void (*schedulerCallback)();

    class Scheduler {
        private:
            Ticker ticker;

        public:
            void schedule(unsigned long interval, schedulerCallback callback);
    };
}
