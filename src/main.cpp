/*

main:
    - core/config
    - core/network      (uses config.ssid, config.password)         connect(),                                                                     getIp(), getLastError()
    - core/mqtt         (uses config.host, config.controllerId)     connect(), subscribe(), loop()
    - core/time         (uses wifi)                                 sync(),                                                                        now()

    - modules/fan       (uses state.temperature)                    currentSpeed, maxSpeed, minSpeed        stepUp(), reset()
    - modules/light                                                 isOn, switchIn, duration                                                       onSwitch()
    - modules/sensor                                                humidity, temperature, stabilityFactor                                         onData(), onError()

    handleConfigurationMessage:

        - setup the modules:

            modules/fan         =   currentSpeed
            modules/light       =   isOn, switchIn, duration
            modules/sensor      =   set up the check interval and start

    handleRebootMessage:

        - reboots the controller

    handleStatusMessage:

        - send the status collected from modules

            modules/fan             currentSpeed
            modules/light           isOn
            modules/sensor          [temperature, humidity]

*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <core.h>
#include <modules.h>

const int LIGHT_PIN = 14;
const int FAN_PIN = 12;
const int SENSOR_PIN = 6;

core::Config config;
core::Network wifi;
core::Mqtt mqtt(wifi.client);
core::Time timer;

modules::Fan fan(FAN_PIN);
modules::Light light(LIGHT_PIN);
modules::Sensor sensor(SENSOR_PIN);


void handleRebootMessage() {
//   char payload[50];
//   const uint32_t interval = 5 * 1000;

//   sprintf(payload, "{\"event\":\"reboot\", \"scheduledIn\":%lu}", interval);

//   sendEvent(payload);

//   rebootTicker.once_ms(interval, [](){
    ESP.restart();
//   });
};

void handleStatusMessage() {
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

void handleConfigurationMessage(char* topic, byte* payload, unsigned int length) {
    Serial.println("mqtt: configuration received");

    payload[length] = '\0';
    
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

    Serial.printf(
        "light is %s. will be switched in %lu hours (%lu ms). fan speed is %u. threshold temperature is %u\n", 
        light.isOn ? "on" : "off", 
        light.switchIn / 1000 / 60 / 60, 
        light.switchIn, 
        fan.currentSpeed, 
        sensor.thresholdTemperature
    );

    // sendRunEvent();
};

void setup () {
    wifi.connect(config.ssid, config.password);

    timer.sync();

    mqtt.connect(config.host, config.controllerId);

    mqtt.subscribe("controllers/" + config.controllerId + "/config/sub");
    mqtt.subscribe("controllers/" + config.controllerId + "/reboot/sub");
    mqtt.subscribe("controllers/" + config.controllerId + "/status/sub");

    mqtt.publish("controllers/" + config.controllerId + "/config/pub");
};

void loop () {
    mqtt.loop();
}
