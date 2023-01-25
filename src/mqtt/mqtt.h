#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <map>
#include <string>

typedef std::function<void(const char* topic, const char* payload)> callback;

class Mqtt {
    private:
        PubSubClient client;
        std::map<std::string, const callback> callbacks;
    public:
        void connect(WiFiClientSecure& wifiClient, const char* host, const char* id, uint16_t port = 8883);
        void subscribe(const char* topic, const callback& cb);
        void publish(const char* topic, const char* message = "{}");
        void loop();
};
