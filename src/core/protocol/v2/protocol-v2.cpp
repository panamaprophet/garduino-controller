#include <core/protocol/v2/protocol-v2.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* CHANNEL_TELEMETRY = "telemetry";
    constexpr const char* CHANNEL_SETTINGS = "settings";
    constexpr const char* CHANNEL_SETTINGS_REQUEST = "settings/get";
    constexpr const char* CHANNEL_COMMANDS = "commands";

    constexpr unsigned long REBOOT_DELAY_MS = 5 * 1000;
    constexpr unsigned long TELEMETRY_INTERVAL_MS = 10 * 60 * 1000;
}

core::ProtocolV2::ProtocolV2(MessageBus& messageBus, ModuleRegistry& registry, Firmware& firmware, Scheduler& scheduler)
    : messageBus(messageBus), registry(registry), firmware(firmware), scheduler(scheduler) {}

void core::ProtocolV2::begin() {
    messageBus.subscribe(CHANNEL_SETTINGS, [this](byte* payload, unsigned int length) { onSettings(payload, length); });
    messageBus.subscribe(CHANNEL_COMMANDS, [this](byte* payload, unsigned int length) { onCommand(payload, length); });

    firmware.onStart([this]() {
        Logger::info("firmware", "update started");
        publishFirmwareUpdate("started");
    });

    firmware.onSuccess([this]() {
        Logger::info("firmware", "update success, rebooting...");
        publishFirmwareUpdate("success");
        reboot();
    });

    firmware.onError([this](const char* message) {
        Logger::error("firmware", "update failed: %s", message);
        publishFirmwareUpdate("error", message);
    });

    scheduler.schedule(TELEMETRY_INTERVAL_MS, [this]() { publishTelemetry(); });

    requestSettings();
    publishBoot();
}

void core::ProtocolV2::publishTelemetry() {
    publishEvent("telemetry", registry.getStatus());
}

void core::ProtocolV2::publishChange(const char* name) {
    char type[32];

    snprintf(type, sizeof(type), "%s:change", name);

    publishEvent(type, registry.getStatus({name}));
}

void core::ProtocolV2::publish(const char* channel, JsonDocument& payload) {
    payload["v"] = version;
    messageBus.publish(channel, payload);
}

void core::ProtocolV2::publishEvent(const char* type, JsonDocument payload) {
    payload["type"] = type;
    publish(CHANNEL_TELEMETRY, payload);
}

void core::ProtocolV2::publishBoot() {
    JsonDocument payload;

    payload["firmwareVersion"] = Firmware::version;
    payload["protocolVersion"] = version;
    payload["resetReason"] = ESP.getResetReason();

    auto modules = payload["modules"].to<JsonArray>();

    for (auto* name : registry.names()) {
        modules.add(name);
    }

    publishEvent("boot", payload);
}

void core::ProtocolV2::publishFirmwareUpdate(const char* status, const char* message) {
    JsonDocument payload;

    payload["status"] = status;

    if (message) {
        payload["message"] = message;
    }

    publishEvent("firmware:update", payload);
}

void core::ProtocolV2::requestSettings() {
    JsonDocument request;
    publish(CHANNEL_SETTINGS_REQUEST, request);
}

void core::ProtocolV2::onSettings(byte* payload, unsigned int length) {
    Logger::info("settings", "settings received");

    JsonDocument document;
    DeserializationError error = deserializeJson(document, payload, length);

    if (error) {
        Logger::error("settings", "invalid json: %s", error.c_str());
        return;
    }

    registry.apply(document.as<JsonObject>());

    publishEvent("settings:applied");
}

void core::ProtocolV2::onCommand(byte* payload, unsigned int length) {
    JsonDocument command;
    DeserializationError error = deserializeJson(command, payload, length);

    if (error) {
        Logger::error("command", "invalid json: %s", error.c_str());
        return;
    }

    const char* type = command["type"] | "";

    Logger::info("command", "%s requested", type);

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

    Logger::error("command", "unknown command: %s", type);
}

void core::ProtocolV2::updateFirmware(const JsonDocument& command) {
    const char* url = command["url"];
    const char* md5 = command["md5"];

    if (!url || !md5) {
        Logger::error("firmware", "update command without url or md5");
        publishFirmwareUpdate("error", "url and md5 are required");
        return;
    }

    firmware.update(url, md5);
}

void core::ProtocolV2::reboot() {
    scheduler.scheduleOnce(REBOOT_DELAY_MS, []() { ESP.restart(); });
}
