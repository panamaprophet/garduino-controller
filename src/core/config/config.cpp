#include <core/config/config.h>

core::Config::Config() {
    LittleFS.begin();

    auto file = LittleFS.open("config.json", "r");
    auto size = file.size();

    char config[size];

    file.readBytes(config, size);
    file.close();

    JsonDocument json;
    deserializeJson(json, config);

    strcpy(controllerId, json["controllerId"]);
    strcpy(host, json["host"]);

    strcpy(ssid, json["wifi"]["ssid"]);
    strcpy(password, json["wifi"]["password"]);

    pinFan = json["pins"]["fan"].as<int>();
    pinLight = json["pins"]["light"].as<int>();
    pinSensor = json["pins"]["sensor"].as<int>();

    json.clear();
};
