#include <core/protocol/v2/protocol-v2.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* CHANNEL_TELEMETRY = "telemetry";
    constexpr const char* CHANNEL_SETTINGS = "settings";
    constexpr const char* CHANNEL_SETTINGS_REQUEST = "settings/get";
    constexpr const char* CHANNEL_COMMANDS = "commands";

    constexpr int VERSION = 2;

    constexpr unsigned long REBOOT_DELAY_MS = 5 * 1000;
    constexpr unsigned long TELEMETRY_INTERVAL_MS = 10 * 60 * 1000;
}

core::ProtocolV2::ProtocolV2(
    MessageBus& messageBus,
    ModuleRegistry& registry,
    Firmware& firmware,
    Scheduler& scheduler
): Protocol(VERSION, messageBus, registry, firmware, scheduler) {}

void core::ProtocolV2::begin() {
    subscribe(CHANNEL_SETTINGS, [this](JsonDocument& settings) { onSettings(settings); });
    subscribe(CHANNEL_COMMANDS, [this](JsonDocument& command) { onCommand(command); });

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

    registry.onChange([this](modules::Module& module) { publishChange(module); });

    scheduler.schedule(TELEMETRY_INTERVAL_MS, [this]() { publishTelemetry(); });
}

void core::ProtocolV2::onConnected() {
    requestSettings();

    if (!isBootPublished) {
        publishBoot();
        isBootPublished = true;
    }
}

void core::ProtocolV2::publishTelemetry() {
    publishEvent("telemetry", registry.getStatus());
}

void core::ProtocolV2::publishChange(modules::Module& module) {
    char type[32];

    snprintf(type, sizeof(type), "%s:change", module.name());

    JsonDocument payload;

    payload[module.name()].set(module.getStatus());

    publishEvent(type, payload);
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

void core::ProtocolV2::onSettings(JsonDocument& settings) {
    Logger::info("settings", "settings received");

    registry.apply(settings.as<JsonObject>());

    publishEvent("settings:applied");
}

void core::ProtocolV2::onCommand(JsonDocument& command) {
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
