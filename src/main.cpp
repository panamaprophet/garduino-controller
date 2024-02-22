#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <core/core.h>
#include <modules/modules.h>


const int LIGHT_PIN = 14;
const int FAN_PIN = 12;
const int SENSOR_PIN = 2;

core::Config config;
core::Network wifi;
core::Mqtt mqtt(wifi.client);
core::Time timer;
core::Scheduler scheduler;

modules::Fan fan(FAN_PIN);
modules::Light light(LIGHT_PIN);
modules::Sensor sensor(SENSOR_PIN);


void handleRebootMessage(byte* _payload, unsigned int _length) {
    Serial.printf("[handler:reboot] reboot was requested\n");
    ESP.restart();
};

void handleStatusMessage(byte* _payload, unsigned int _length) {
    Serial.printf("[handler:status] status was requested\n");

    char payload[100];

    sprintf(
        payload, 
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"isOn\":%s,\"fanSpeed\":%d,\"stabilityFactor\":%.2d}",
        sensor.temperature, 
        sensor.humidity, 
        light.isOn ? "true" : "false", 
        fan.currentSpeed,
        sensor.stabilityFactor
    );

    mqtt.publish("controllers/" + config.controllerId + "/status/pub", payload);
};

void handleConfigurationMessage(byte* payload, unsigned int length) {
    Serial.printf("[handler:config] configuration received\n");

    StaticJsonDocument<200> json;
    deserializeJson(json, (char *)payload);

    light.isOn = json["isOn"].as<boolean>();
    light.duration = json["duration"].as<unsigned long>();
    light.switchIn = json["switchIn"].as<unsigned long>();

    fan.currentSpeed = json["fanSpeed"].as<unsigned int>();

    sensor.thresholdTemperature = json["thresholdTemperature"].as<unsigned int>();

    json.clear();

    sensor.run();
    light.run();
    fan.run();

    Serial.printf("[handler:config] configuration applied\n");

    mqtt.publish(
        "controllers/" + config.controllerId + "/events/pub",
        "{\"event\": \"run\", \"isOn\": \"" + std::string(light.isOn ? "true" : "false") + "\"}"
    );
};


void onLightSwitch(bool isOn, unsigned long switchIn) {
    mqtt.publish(
        "controllers/" + config.controllerId + "/events/pub", 
        "{\"event\":\"switch\", \"isOn\":\"" + std::string(light.isOn ? "true" : "false") + "\"}"
    );
};

void onHighTemperature(float temperature) {
    fan.stepUp();
};

void onUpdate() {
    char payload[100];

    sprintf(
        payload,
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"fanSpeed\": %.2d,\"event\":\"update\"}", 
        sensor.temperature, 
        sensor.humidity, 
        fan.currentSpeed
    );

    mqtt.publish("controllers/" + config.controllerId + "/events/pub", payload);
}


void setup () {
    Serial.begin(115200);

    wifi.connect(config.ssid, config.password);

    timer.sync();

    mqtt.connect(config.host, config.controllerId);

    mqtt.subscribe("controllers/" + config.controllerId + "/config/sub", handleConfigurationMessage);
    mqtt.subscribe("controllers/" + config.controllerId + "/reboot/sub", handleRebootMessage);
    mqtt.subscribe("controllers/" + config.controllerId + "/status/sub", handleStatusMessage);

    mqtt.publish("controllers/" + config.controllerId + "/config/pub");

    light.onSwitch(onLightSwitch);
    sensor.onThreshold(onHighTemperature);
    scheduler.schedule(10 * 60 * 1000, onUpdate);
};

void loop () {
    mqtt.loop();
}
