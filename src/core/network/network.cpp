#include <core/network/network.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* ROOT_CERT_PATH = "root.crt";
    constexpr const char* CLIENT_CERT_PATH = "controller.cert.pem";
    constexpr const char* PRIVATE_KEY_PATH = "controller.private.key";
    constexpr unsigned int CONNECTION_TIMEOUT_MS = 15000;
    constexpr unsigned int CONNECTION_POLL_MS = 500;
}

core::Network::Network() {
    LittleFS.begin();
    loadRootCertificate();
    loadClientCertificate();
};

void core::Network::loadRootCertificate() {
    Logger::info("network", "loading root certificate");

    auto file = LittleFS.open(ROOT_CERT_PATH, "r");
    auto size = file.size();
    auto buffer = std::make_unique<char[]>(size);

    file.readBytes(buffer.get(), size);
    file.close();

    rootCertificate = std::make_unique<BearSSL::X509List>(buffer.get());
    client.setTrustAnchors(rootCertificate.get());
};

void core::Network::loadClientCertificate() {
    Logger::info("network", "loading client certificate");

    auto certFile = LittleFS.open(CLIENT_CERT_PATH, "r");
    auto certSize = certFile.size();
    auto certBuffer = std::make_unique<char[]>(certSize);

    certFile.readBytes(certBuffer.get(), certSize);
    certFile.close();

    auto keyFile = LittleFS.open(PRIVATE_KEY_PATH, "r");
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

    Logger::info("network", "connecting...");

    unsigned int elapsed = 0;

    while (WiFi.status() != WL_CONNECTED) {
        delay(CONNECTION_POLL_MS);
        elapsed += CONNECTION_POLL_MS;

        if (elapsed >= CONNECTION_TIMEOUT_MS) {
            Logger::error("network", "connection timeout");
            return;
        }
    }

    Logger::info("network", "connected (%s)", WiFi.localIP().toString().c_str());
};
