#include <core/network/network.h>

core::Network::Network() {
    LittleFS.begin();
    setRootCertificate();
    setClientCertificate();
};

void core::Network::setRootCertificate() {
    Serial.printf("[network] loading root certificate...\n");

    auto rootCertificateFile = LittleFS.open("root.crt", "r");
    auto rootCertificateSize = rootCertificateFile.size();
    char *rootCertificateBuffer = new char[rootCertificateSize];

    rootCertificateFile.readBytes(rootCertificateBuffer, rootCertificateSize);
    rootCertificateFile.close();

    const BearSSL::X509List* cert = new BearSSL::X509List(rootCertificateBuffer);

    client.setTrustAnchors(cert);
};

void core::Network::setClientCertificate() {
    Serial.printf("[network] loading client certificate...\n");

    auto clientCertificateFile = LittleFS.open("controller.cert.pem", "r");
    auto clientCertificateSize = clientCertificateFile.size();

    char *clientCertificateBuffer = new char[clientCertificateSize];

    clientCertificateFile.readBytes(clientCertificateBuffer, clientCertificateSize);
    clientCertificateFile.close();

    auto privateKeyFile = LittleFS.open("controller.private.key", "r");
    auto privateKeySize = privateKeyFile.size();

    char *privateKeyBuffer = new char[privateKeySize];

    privateKeyFile.readBytes(privateKeyBuffer, privateKeySize);
    privateKeyFile.close();

    const BearSSL::X509List* clientCertificate = new BearSSL::X509List(clientCertificateBuffer);
    const BearSSL::PrivateKey* key = new BearSSL::PrivateKey(privateKeyBuffer);

    client.setClientRSACert(clientCertificate, key);
};

void core::Network::connect(std::string ssid, std::string password) {
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.printf("[network] connecting ");

    unsigned int timeout = 15000;
    unsigned int step = 500;

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');

        delay(step);
        timeout -= step;

        if (timeout <= 0) {
            Serial.print(" timeout");
        }
    }

    Serial.printf(" %s\n", WiFi.status() == WL_CONNECTED ? "success" : "fail");
    Serial.print("[network] ip = ");
    Serial.println(WiFi.localIP());
};
