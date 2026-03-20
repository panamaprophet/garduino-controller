#pragma once

#include <Arduino.h>
#include <string>
#include <memory>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>

namespace core {
    class Network {
        private:
            std::unique_ptr<BearSSL::X509List> rootCertificate;
            std::unique_ptr<BearSSL::X509List> clientCertificate;
            std::unique_ptr<BearSSL::PrivateKey> privateKey;

            void loadRootCertificate();
            void loadClientCertificate();

        public:
            BearSSL::WiFiClientSecure client;

            Network();

            void connect(const char* ssid, const char* password);
    };
}
