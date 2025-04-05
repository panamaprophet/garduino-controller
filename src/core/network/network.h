#include <Arduino.h>
#include <string>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>

namespace core {
    class Network {
        private:
            void setRootCertificate();
            void setClientCertificate();

        public:
            BearSSL::WiFiClientSecure client;

            Network();

            void connect(const char* ssid, const char* password);
    };
}
