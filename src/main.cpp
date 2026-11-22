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
core::MessageBus messageBus(mqtt, config.controllerId);

modules::Fan fan(config.pinFan);
modules::Light light(config.pinLight);
modules::Sensor sensor(config.pinSensor);

core::ModuleRegistry registry({&fan, &light, &sensor});
core::ProtocolV2 protocol(messageBus, registry, firmware, scheduler);


void onLightChanged(modules::Module& module) {
    const auto isLightModule = &module == &light;
    const auto isLightOff = !light.isOn();

    if (isLightModule && isLightOff) {
        scheduler.scheduleOnce(FAN_RESET_DELAY_MS, []() { fan.reset(); });
    }
}

void onTemperatureChanged(modules::Module& module) {
    const auto isSensorModule = &module == &sensor;
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

    protocol.begin();

    registry.onChange(onLightChanged);
    registry.onChange(onTemperatureChanged);

    mqtt.onConnect([]() { protocol.onConnected(); });
    mqtt.connect(config.host, config.controllerId);
}

void loop() {
    mqtt.loop();
}
