#include <core/config/config.h>

core::Config::Config() {
    LittleFS.begin();

    auto file = LittleFS.open("config.json", "r");
    auto size = file.size();

    char *config = new char[size];

    file.readBytes(config, size);
    file.close();

    JsonDocument json;
    deserializeJson(json, config);

    controllerId = json["controllerId"].as<std::string>();
    host = json["host"].as<std::string>();

    ssid = json["wifi"]["ssid"].as<std::string>();
    password = json["wifi"]["password"].as<std::string>();

    pinFan = json["pins"]["fan"].as<int>();
    pinLight = json["pins"]["light"].as<int>();
    pinSensor = json["pins"]["sensor"].as<int>();
};
