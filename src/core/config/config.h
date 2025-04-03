#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <LittleFS.h>

namespace core {
    class Config {
        public:
            char ssid[64];
            char password[64];
            char host[128];
            char controllerId[64];

            int pinFan;
            int pinLight;
            int pinSensor;

            Config();
    };
}
