
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

void core::Network::setRootCertificate() {
    auto rootCertificateFile = LittleFS.open("root.crt", "r");
    auto rootCertificateSize = rootCertificateFile.size();
    char *rootCertificateBuffer = new char[rootCertificateSize];

    rootCertificateFile.readBytes(rootCertificateBuffer, rootCertificateSize);
    rootCertificateFile.close();

    const BearSSL::X509List* cert = new BearSSL::X509List(rootCertificateBuffer);

    client.setTrustAnchors(cert);
};

void core::Network::setClientCertificate() {
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


core::Network::Network() {
    LittleFS.begin();
    setRootCertificate();
    setClientCertificate();
};

void core::Network::connect(std::string ssid, std::string password) {
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.printf("\nconnecting to wifi");

    unsigned int timeout = 15000;
    unsigned int step = 500;

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');

        delay(step);
        timeout -= step;

        if (timeout <= 0) {
        Serial.print("timeout");
        }
    }

    Serial.println();
    Serial.print("ip = ");
    Serial.println(WiFi.localIP());
};

void core::Mqtt::resubscribe() {
    for (auto topic: topics) {
        client.subscribe(topic.c_str());
    }
}


core::Mqtt::Mqtt(Client& networkClient) {
        client.setClient(networkClient);
    };

void core::Mqtt::subscribe(std::string topic) {
    topics.push_back(topic);
}

void core::Mqtt::connect(std::string host, std::string id, uint16 port) {
    _host = host;
    _id = id;
    
    Serial.printf("mqtt connection: ");

    client.setServer(host.c_str(), port);

    client.setCallback([](char* topic, byte* payload, unsigned int length) {
        // trah lah lah
    });

    client.connect(id.c_str());

    Serial.printf("%s\n", client.connected() ? "success" : "fail");

    if (!client.connected()) {
        // char message[80];

        // int code = wifi.getLastSSLError(message, sizeof(message));

        // Serial.printf("mqtt connection failed. error code = %d\n", client.state());
        // Serial.printf("mqtt connection failed. ssl error = %d: %s\n", code, message);

        return;
    }

    resubscribe();

    // mqtt.subscribe(("controllers/" + controllerId + "/config/sub").c_str());
    // mqtt.subscribe(("controllers/" + controllerId + "/reboot/sub").c_str());
    // mqtt.subscribe(("controllers/" + controllerId + "/status/sub").c_str());
};

void core::Mqtt::publish(std::string topic, std::string payload) {
    client.publish(topic.c_str(), payload.c_str());
};

void core::Mqtt::loop() {
    if (!client.connected()) {
        connect(_host, _id);
    }

    client.loop();
};


time_t core::Time::sync() {
    Serial.print("time sync...");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);

    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }

    struct tm timeinfo;

    gmtime_r(&now, &timeinfo);

    Serial.printf("\ncurrent time = %s", asctime(&timeinfo));

    return now;
};
