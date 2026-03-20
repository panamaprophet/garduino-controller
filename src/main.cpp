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
core::Firmware firmware(wifi.client);

modules::Fan fan(config.pinFan);
modules::Light light(config.pinLight);
modules::Sensor sensor(config.pinSensor);


void handleFirmwareUpdateMessage(byte* payload, unsigned int length) {
    Serial.printf("[handler:firmware] update received\n");

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);

    if (err) {
        Serial.printf("[handler:firmware] invalid json: %s\n", err.c_str());
        return;
    }

    const char* url = doc["url"].as<const char*>();
    const char* md5 = doc["md5"].as<const char*>();

    firmware.update(url, md5);
}

void handleRebootMessage(byte* _payload, unsigned int _length) {
    Serial.printf("[handler:reboot] reboot was requested\n");

    char topic[100];
    char message[100];

    sprintf(topic, "controllers/%s/reboot/pub", config.controllerId);
    sprintf(message, "{\"event\":\"reboot\"}");

    mqtt.publish(topic, message);

    scheduler.scheduleOnce(5 * 1000, [](){ ESP.restart(); });
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

    if (!isOn) {
        scheduler.scheduleOnce(60 * 1000, [](){ fan.reset(); });
    }
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

void onFirmwareUpdateStart() {
    char topic[100];

    sprintf(topic, "controllers/%s/firmware/update/pub", config.controllerId);

    mqtt.publish(topic, "{\"event\":\"started\"}");

    Serial.printf("[firmware] update started\n");
}

void onFirmwareUpdateSuccess() {
    char topic[100];

    sprintf(topic, "controllers/%s/firmware/update/pub", config.controllerId);

    mqtt.publish(topic, "{\"event\":\"success\"}");

    scheduler.scheduleOnce(5 * 1000, []() { ESP.restart(); });

    Serial.printf("[firmware] update success\n");
}

void onFirmwareUpdateError(const char* errorMessage) {
    char topic[100];
    char message[256];

    sprintf(topic, "controllers/%s/firmware/update/pub", config.controllerId);
    sprintf(message, "{\"event\":\"error\",\"message\":\"%s\"}", errorMessage);

    mqtt.publish(topic, message);

    Serial.printf("[firmware] update error: %s\n", errorMessage);
}


void setup() {
    Serial.begin(115200);

    wifi.connect(config.ssid, config.password);

    timer.sync();

    mqtt.connect(config.host, config.controllerId);

    char configTopic[100];
    char rebootTopic[100];
    char statusTopic[100];
    char firmwareTopic[100];

    sprintf(configTopic, "controllers/%s/config/sub", config.controllerId);
    sprintf(rebootTopic, "controllers/%s/reboot/sub", config.controllerId);
    sprintf(statusTopic, "controllers/%s/status/sub", config.controllerId);
    sprintf(firmwareTopic, "controllers/%s/firmware/update/sub", config.controllerId);

    firmware.onStart(onFirmwareUpdateStart);
    firmware.onSuccess(onFirmwareUpdateSuccess);
    firmware.onError(onFirmwareUpdateError);

    mqtt.subscribe(configTopic, handleConfigurationMessage);
    mqtt.subscribe(firmwareTopic, handleFirmwareUpdateMessage);
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
