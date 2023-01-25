#include <wifi/wifi.h>


bool WirelessNetwork::connect(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);

    Serial.println();
    Serial.print("connecting to wifi");

    unsigned int timeout = 5000;
    unsigned int step = 500;

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');

      delay(step);
      timeout -= step;

      if (timeout <= 0) {
        Serial.println();
        Serial.println("connection timeout");

        return false;
      }
    }

    Serial.println();
    Serial.print("ip = ");
    Serial.println(WiFi.localIP());

    return true;
}

void WirelessNetwork::setTrustAnchors(const char* certificate) {
    const BearSSL::X509List* cert = new BearSSL::X509List(certificate);

    client.setTrustAnchors(cert);
}

void WirelessNetwork::setClientCertificate(const char* certificate, const char* privateKey) {
    const BearSSL::X509List* clientCertificate = new BearSSL::X509List(certificate);
    const BearSSL::PrivateKey* key = new BearSSL::PrivateKey(privateKey);

    client.setClientRSACert(clientCertificate, key);
}

BearSSL::WiFiClientSecure& WirelessNetwork::getClient() {
    return client;
}
