#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

class WirelessNetwork {
    private:
        BearSSL::WiFiClientSecure client;
    public:
        bool connect(const char* ssid, const char* password);
        void setTrustAnchors(const char* certificate);
        void setClientCertificate(const char* certificate, const char* privateKey);
        BearSSL::WiFiClientSecure& getClient();
};
