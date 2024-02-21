
#include <core.h>


core::Config::Config() {
    LittleFS.begin();

    auto file = LittleFS.open("config.json", "r");
    auto size = file.size();

    char *config = new char[size];

    file.readBytes(config, size);
    file.close();

    StaticJsonDocument<200> json;
    deserializeJson(json, config);

    controllerId = json["controllerId"].as<std::string>();
    host = json["host"].as<std::string>();
    ssid = json["ssid"].as<std::string>();
    password = json["password"].as<std::string>();
};

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

    Serial.printf("[network] connecting: ");

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

    Serial.println();
    Serial.print("[network] ip = ");
    Serial.println(WiFi.localIP());
};


core::Mqtt::Mqtt(Client& networkClient) {
    client.setClient(networkClient);
};

void core::Mqtt::resubscribe() {
    for (auto const& item: callbacks) {
        client.subscribe(item.first.c_str());
    }
}

void core::Mqtt::subscribe(std::string topic, mqttCallback callback) {
    Serial.printf("[mqtt] subscribing to %s\n", topic.c_str());

    callbacks.insert_or_assign(topic, callback);
    client.subscribe(topic.c_str());
}

void core::Mqtt::connect(std::string host, std::string id, uint16 port) {
    _host = host;
    _id = id;
    
    Serial.printf("[mqtt] connecting: ");

    client.setServer(host.c_str(), port);

    client.setCallback([&](char* _topic, byte* payload, unsigned int length) {
        const std::string topic(_topic);
        auto result = callbacks.find(topic);

        payload[length] = '\0';

        Serial.printf("[mqtt] message from %s\n", _topic);

        if (result != callbacks.end()) {
            result -> second(payload, length);
        }
    });

    client.connect(id.c_str());

    Serial.printf("%s\n", client.connected() ? "success" : "fail");

    if (!client.connected()) {
        Serial.printf("[mqtt] connection failed. error code = %d\n", client.state());
        return;
    }

    resubscribe();
};

void core::Mqtt::publish(std::string topic, std::string payload) {
    Serial.printf("[mqtt] publishing to %s\n", topic.c_str());
    client.publish(topic.c_str(), payload.c_str());
};

void core::Mqtt::loop() {
    if (!client.connected()) {
        Serial.printf("[mqtt] connection lost. attempting to reconnect");
        connect(_host, _id);
    }

    client.loop();
};


time_t core::Time::sync() {
    Serial.print("[time] sync: ");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);

    while (now < 8 * 3600 * 2) {
        delay(100);
        Serial.print(".");
        now = time(nullptr);
    }

    struct tm timeinfo;

    gmtime_r(&now, &timeinfo);

    Serial.printf("\n[time] current time: %s", asctime(&timeinfo));

    return now;
};
