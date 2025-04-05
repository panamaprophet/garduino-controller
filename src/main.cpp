#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <core/core.h>
#include <modules/modules.h>

core::Config config;
core::Network wifi;
core::Mqtt mqtt(wifi.client);
core::Time timer;
core::Scheduler scheduler;

modules::Fan fan(config.pinFan);
modules::Light light(config.pinLight);
modules::Sensor sensor(config.pinSensor);

void handleRebootMessage(byte* _payload, unsigned int _length) {
    Serial.printf("[handler:reboot] reboot was requested\n");
    ESP.restart();
};

void handleStatusMessage(byte* _payload, unsigned int _length) {
    Serial.printf("[handler:status] status was requested\n");

    char topic[100];
    char message[100];

    sprintf(topic, "controllers/%s/status/pub", config.controllerId);

    sprintf(
        message,
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"isOn\":%s,\"fanSpeed\":%d,\"stabilityFactor\":%d}",
        sensor.temperature, sensor.humidity, light.isOn ? "true" : "false", fan.currentSpeed, sensor.stabilityFactor
    );

    mqtt.publish(topic, message);
};

void handleConfigurationMessage(byte* payload, unsigned int length) {
    Serial.printf("[handler:config] configuration received\n");

    JsonDocument json;
    deserializeJson(json, (char*)payload);

    light.isOn = json["isOn"].as<boolean>();
    light.duration = json["duration"].as<unsigned long>();
    light.switchIn = json["switchIn"].as<unsigned long>();

    fan.defaultSpeed = json["fanSpeed"].as<unsigned int>();

    sensor.thresholdTemperature = json["thresholdTemperature"].as<unsigned int>();

    json.clear();

    Serial.printf("[handler:config] configuration applied\n");

    sensor.run();
    light.run();
    fan.run();

    char topic[100];
    char message[100];

    sprintf(topic, "controllers/%s/events/pub", config.controllerId);
    sprintf(message, "{\"event\": \"run\", \"isOn\": %s}", light.isOn ? "true" : "false");

    mqtt.publish(topic, message);
};


void onLightSwitch(bool isOn, unsigned long switchIn) {
    char topic[100];
    char message[100];

    sprintf(topic, "controllers/%s/events/pub", config.controllerId);
    sprintf(message, "{\"event\":\"switch\", \"isOn\":%s}", light.isOn ? "true" : "false");

    mqtt.publish(topic, message);
};

void onHighTemperature(float temperature) {
    fan.stepUp();
};

void onUpdate() {
    char topic[100];
    char message[100];

    sprintf(topic, "controllers/%s/events/pub", config.controllerId);

    sprintf(
        message,
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"fanSpeed\": %d,\"event\":\"update\"}",
        sensor.temperature,
        sensor.humidity,
        fan.currentSpeed
    );

    mqtt.publish(topic, message);
}


void setup() {
    Serial.begin(115200);

    wifi.connect(config.ssid, config.password);

    timer.sync();

    mqtt.connect(config.host, config.controllerId);

    char configTopic[100];
    char rebootTopic[100];
    char statusTopic[100];

    sprintf(configTopic, "controllers/%s/config/sub", config.controllerId);
    sprintf(rebootTopic, "controllers/%s/reboot/sub", config.controllerId);
    sprintf(statusTopic, "controllers/%s/status/sub", config.controllerId);

    mqtt.subscribe(configTopic, handleConfigurationMessage);
    mqtt.subscribe(rebootTopic, handleRebootMessage);
    mqtt.subscribe(statusTopic, handleStatusMessage);

    light.onSwitch(onLightSwitch);
    sensor.onThreshold(onHighTemperature);
    scheduler.schedule(10 * 60 * 1000, onUpdate);

    char topic[100];

    sprintf(topic, "controllers/%s/config/pub", config.controllerId);

    mqtt.publish(topic);
};

void loop() {
    mqtt.loop();
}
