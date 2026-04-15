#include <Arduino.h>
#include <ArduinoJson.h>
#include <core/core.h>
#include <modules/modules.h>

namespace {
    constexpr const char* EVENTS_CHANNEL = "events/pub";
    constexpr const char* EVENT_TEMPERATURE_HIGH = "temperature:high";
    constexpr const char* EVENT_LIGHT_SWITCH = "light:switch";
    constexpr unsigned long REBOOT_DELAY_MS = 5 * 1000;
    constexpr unsigned long FAN_RESET_DELAY_MS = 60 * 1000;
    constexpr unsigned long TELEMETRY_INTERVAL_MS = 10 * 60 * 1000;
}


core::Config config;
core::Network network;
core::Mqtt mqtt(network.client);
core::Time timer;
core::Scheduler scheduler;
core::Firmware firmware(network.client);
core::EventBus eventBus;
core::MessageBus messageBus(mqtt, config.controllerId);

modules::Fan fan(eventBus, config.pinFan);
modules::Light light(eventBus, config.pinLight);
modules::Sensor sensor(eventBus, config.pinSensor);

core::ModuleRegistry registry({&fan, &light, &sensor});


void publishStatus() {
    auto status = registry.getStatus();

    status["event"] = "update";

    char buffer[256];
    serializeJson(status, buffer, sizeof(buffer));

    messageBus.publish(EVENTS_CHANNEL, buffer);
}

JsonDocument adaptConfig(const JsonObject& raw) {
    JsonDocument config;

    config["fan"]["defaultSpeed"] = raw["fanSpeed"];
    config["light"]["isOn"] = raw["isOn"];
    config["light"]["switchIn"] = raw["switchIn"];
    config["light"]["duration"] = raw["duration"];
    config["sensor"]["thresholdTemperature"] = raw["thresholdTemperature"];

    return config;
}

void onConfigReceived(byte* payload, unsigned int length) {
    core::Logger::info("config", "configuration received");

    JsonDocument document;
    DeserializationError error = deserializeJson(document, payload, length);

    if (error) {
        core::Logger::error("config", "invalid json: %s", error.c_str());
        return;
    }

    auto config = adaptConfig(document.as<JsonObject>());
    registry.apply(config.as<JsonObject>());

    messageBus.publish(EVENTS_CHANNEL, R"({"event":"run"})");
}

void onStatusRequested(byte* payload, unsigned int length) {
    core::Logger::info("status", "status requested");
    publishStatus();
}

void onRebootRequested(byte* payload, unsigned int length) {
    core::Logger::info("reboot", "reboot requested");

    messageBus.publish(EVENTS_CHANNEL, R"({"event":"reboot"})");

    scheduler.scheduleOnce(REBOOT_DELAY_MS, []() { ESP.restart(); });
}

void onFirmwareUpdateRequested(byte* payload, unsigned int length) {
    core::Logger::info("firmware", "update requested");

    JsonDocument document;
    DeserializationError error = deserializeJson(document, payload, length);

    if (error) {
        core::Logger::error("firmware", "invalid json: %s", error.c_str());
        return;
    }

    const char* url = document["url"].as<const char*>();
    const char* md5 = document["md5"].as<const char*>();

    firmware.update(url, md5);
}


void setup() {
    core::Logger::begin();
    core::Logger::info("firmware", "version: %s", core::Firmware::version);

    network.connect(config.ssid, config.password);
    timer.sync();
    mqtt.connect(config.host, config.controllerId);

    firmware.onStart([]() {
        messageBus.publish(EVENTS_CHANNEL, R"({"event":"firmware:update:started"})");
        core::Logger::info("firmware", "update started");
    });

    firmware.onSuccess([]() {
        messageBus.publish(EVENTS_CHANNEL, R"({"event":"firmware:update:success"})");
        scheduler.scheduleOnce(REBOOT_DELAY_MS, []() { ESP.restart(); });
        core::Logger::info("firmware", "update success, rebooting...");
    });

    firmware.onError([](const char* errorMessage) {
        char message[256];
        snprintf(message, sizeof(message), R"({"event":"firmware:update:error","message":"%s"})", errorMessage);
        messageBus.publish(EVENTS_CHANNEL, message);
        core::Logger::error("firmware", "update failed: %s", errorMessage);
    });

    eventBus.on(EVENT_TEMPERATURE_HIGH, [](const char*) {
        fan.stepUp();
    });

    eventBus.on(EVENT_LIGHT_SWITCH, [](const char* data) {
        char message[128];
        snprintf(message, sizeof(message), R"({"event":"light:switch","isOn":%s})", data);
        messageBus.publish(EVENTS_CHANNEL, message);

        if (strcmp(data, "false") == 0) {
            scheduler.scheduleOnce(FAN_RESET_DELAY_MS, []() { fan.reset(); });
        }
    });

    messageBus.subscribe("config/sub", onConfigReceived);
    messageBus.subscribe("status/sub", onStatusRequested);
    messageBus.subscribe("reboot/sub", onRebootRequested);
    messageBus.subscribe("firmware/update/sub", onFirmwareUpdateRequested);

    scheduler.schedule(TELEMETRY_INTERVAL_MS, publishStatus);

    messageBus.publish("config/pub");
}

void loop() {
    mqtt.loop();
}
