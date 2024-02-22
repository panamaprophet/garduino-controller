#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <LittleFS.h>

namespace core {
    class Config {
        public:
            std::string ssid;
            std::string password;
            std::string host;
            std::string controllerId;

            Config();
    };
}
