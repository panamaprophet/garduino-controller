#include <Arduino.h>
#include <core/core.h>
#include <modules/modules.h>

namespace {
    constexpr unsigned long FAN_RESET_DELAY_MS = 60 * 1000;
}


core::Config config;
core::Network network;
core::Mqtt mqtt(network.client);
core::Scheduler scheduler;
core::Firmware firmware(network.client);
core::EventBus eventBus;
core::MessageBus messageBus(mqtt, config.controllerId);

modules::Fan fan(eventBus, config.pinFan);
modules::Light light(eventBus, config.pinLight);
modules::Sensor sensor(eventBus, config.pinSensor);

core::ModuleRegistry registry({&fan, &light, &sensor});
core::ProtocolV2 protocol(messageBus, registry, firmware, scheduler);


void onModuleChanged(const char* name) {
    protocol.publishChange(name);
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


void setup() {
    core::Logger::begin();
    core::Logger::info("firmware", "version: %s", core::Firmware::version);

    network.connect(config.ssid, config.password);

    core::Time::sync();

    eventBus.on(modules::Module::CHANGED_EVENT, onModuleChanged);
    eventBus.on(modules::Module::CHANGED_EVENT, onLightChanged);
    eventBus.on(modules::Module::CHANGED_EVENT, onTemperatureChanged);

    protocol.begin();

    mqtt.onConnect([]() { protocol.onConnected(); });
    mqtt.connect(config.host, config.controllerId);
}

void loop() {
    mqtt.loop();
}
