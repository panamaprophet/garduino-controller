#include <Arduino.h>
#include <ArduinoJson.h>
#include <core/core.h>
#include <modules/modules.h>

namespace {
    constexpr const char* CHANNEL_TELEMETRY = "telemetry";
    constexpr const char* CHANNEL_SETTINGS = "settings";
    constexpr const char* CHANNEL_SETTINGS_REQUEST = "settings/get";
    constexpr const char* CHANNEL_COMMANDS = "commands";

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


void publishEvent(const char* type, JsonDocument payload = JsonDocument()) {
    payload["type"] = type;
    messageBus.publish(CHANNEL_TELEMETRY, payload);
}

void publishTelemetry() {
    publishEvent("telemetry", registry.getStatus());
}

void publishBoot() {
    JsonDocument payload;

    payload["firmwareVersion"] = core::Firmware::version;
    payload["protocolVersion"] = core::MessageBus::protocolVersion;
    payload["resetReason"] = ESP.getResetReason();

    auto modules = payload["modules"].to<JsonArray>();

    for (auto* name : registry.names()) {
        modules.add(name);
    }

    publishEvent("boot", payload);
}

void publishFirmwareUpdate(const char* status, const char* message = nullptr) {
    JsonDocument payload;

    payload["status"] = status;

    if (message) {
        payload["message"] = message;
    }

    publishEvent("firmware:update", payload);
}

void publishChange(const char* name) {
    char type[32];

    snprintf(type, sizeof(type), "%s:change", name);

    publishEvent(type, registry.getStatus({name}));
}

void onModuleChanged(const char* name) {
    publishChange(name);
}

void onLightChanged(const char* name) {
    const auto isLightModule = strcmp(name, light.name()) == 0;
    const auto isLightOff = !light.isOn();

    if (isLightModule && isLightOff) {
        scheduler.scheduleOnce(FAN_RESET_DELAY_MS, []() { fan.reset(); });
    }
}

void onTemperatureChanged(const char* name) {
    const auto isSensorModule = strcmp(name, sensor.name()) == 0;
    const auto isOverheating = sensor.isOverheating();

    if (isSensorModule && isOverheating) {
        fan.stepUp();
    }
}

void reboot() {
    scheduler.scheduleOnce(REBOOT_DELAY_MS, []() { ESP.restart(); });
}

void onSettingsReceived(byte* payload, unsigned int length) {
    core::Logger::info("settings", "settings received");

    JsonDocument document;
    DeserializationError error = deserializeJson(document, payload, length);

    if (error) {
        core::Logger::error("settings", "invalid json: %s", error.c_str());
        return;
    }

    registry.apply(document.as<JsonObject>());

    publishEvent("settings:applied");
}

void updateFirmware(const JsonDocument& command) {
    const char* url = command["url"];
    const char* md5 = command["md5"];

    if (!url || !md5) {
        core::Logger::error("firmware", "update command without url or md5");
        publishFirmwareUpdate("error", "url and md5 are required");
        return;
    }

    firmware.update(url, md5);
}

void onCommandReceived(byte* payload, unsigned int length) {
    JsonDocument command;
    DeserializationError error = deserializeJson(command, payload, length);

    if (error) {
        core::Logger::error("command", "invalid json: %s", error.c_str());
        return;
    }

    const char* type = command["type"] | "";

    core::Logger::info("command", "%s requested", type);

    if (strcmp(type, "reboot") == 0) {
        reboot();
        return;
    }

    if (strcmp(type, "refresh") == 0) {
        publishTelemetry();
        return;
    }

    if (strcmp(type, "firmware.update") == 0) {
        updateFirmware(command);
        return;
    }

    core::Logger::error("command", "unknown command: %s", type);
}

void onFirmwareUpdateStarted() {
    core::Logger::info("firmware", "update started");
    publishFirmwareUpdate("started");
}

void onFirmwareUpdateSuccess() {
    core::Logger::info("firmware", "update success, rebooting...");
    publishFirmwareUpdate("success");
    reboot();
}

void onFirmwareUpdateError(const char* message) {
    core::Logger::error("firmware", "update failed: %s", message);
    publishFirmwareUpdate("error", message);
}


void setup() {
    core::Logger::begin();
    core::Logger::info("firmware", "version: %s", core::Firmware::version);

    network.connect(config.ssid, config.password);

    timer.sync();

    mqtt.connect(config.host, config.controllerId);

    firmware.onStart(onFirmwareUpdateStarted);
    firmware.onSuccess(onFirmwareUpdateSuccess);
    firmware.onError(onFirmwareUpdateError);

    eventBus.on(modules::Module::CHANGED_EVENT, onModuleChanged);
    eventBus.on(modules::Module::CHANGED_EVENT, onLightChanged);
    eventBus.on(modules::Module::CHANGED_EVENT, onTemperatureChanged);

    messageBus.subscribe(CHANNEL_SETTINGS, onSettingsReceived);
    messageBus.subscribe(CHANNEL_COMMANDS, onCommandReceived);

    scheduler.schedule(TELEMETRY_INTERVAL_MS, publishTelemetry);

    JsonDocument request;
    messageBus.publish(CHANNEL_SETTINGS_REQUEST, request);

    publishBoot();
}

void loop() {
    mqtt.loop();
}
