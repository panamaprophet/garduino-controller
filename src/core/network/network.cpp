#include <core/network/network.h>

core::Network::Network() {
    LittleFS.begin();
    loadRootCertificate();
    loadClientCertificate();
};

void core::Network::loadRootCertificate() {
    Serial.printf("[network] loading root certificate...\n");

    auto file = LittleFS.open("root.crt", "r");
    auto size = file.size();
    auto buffer = std::make_unique<char[]>(size);

    file.readBytes(buffer.get(), size);
    file.close();

    rootCertificate = std::make_unique<BearSSL::X509List>(buffer.get());
    client.setTrustAnchors(rootCertificate.get());
};

void core::Network::loadClientCertificate() {
    Serial.printf("[network] loading client certificate...\n");

    auto certFile = LittleFS.open("controller.cert.pem", "r");
    auto certSize = certFile.size();
    auto certBuffer = std::make_unique<char[]>(certSize);

    certFile.readBytes(certBuffer.get(), certSize);
    certFile.close();

    auto keyFile = LittleFS.open("controller.private.key", "r");
    auto keySize = keyFile.size();
    auto keyBuffer = std::make_unique<char[]>(keySize);

    keyFile.readBytes(keyBuffer.get(), keySize);
    keyFile.close();

    clientCertificate = std::make_unique<BearSSL::X509List>(certBuffer.get());
    privateKey = std::make_unique<BearSSL::PrivateKey>(keyBuffer.get());

    client.setClientRSACert(clientCertificate.get(), privateKey.get());
};

void core::Network::connect(const char* ssid, const char* password) {
    WiFi.setAutoReconnect(true);

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    Serial.printf("[network] connecting ");

    unsigned int timeout = 15000;
    unsigned int step = 500;
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.printf(".");

        delay(step);
        timeout -= step;

        if (timeout <= 0) {
            Serial.printf(" timeout");
        }
    }

    Serial.printf(" %s\n", WiFi.status() == WL_CONNECTED ? "success" : "fail");
    Serial.printf("[network] ip = %s\n", WiFi.localIP().toString().c_str());
};
