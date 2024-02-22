#include <core/config/config.h>

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
