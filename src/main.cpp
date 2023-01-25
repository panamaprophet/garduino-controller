#include <Arduino.h>
#include <Ticker.h>
#include <vector>
#include <string>
#include <secrets.h>

#include <wifi/wifi.h>
#include <mqtt/mqtt.h>
#include <sensor/sensor.h>
#include <helpers.h>

bool isOn;
unsigned long duration;
unsigned int thresholdTemperature;
unsigned int fanSpeed;

float humidity;
float temperature;

Ticker ticker;
Ticker updateTicker;

WirelessNetwork wifi;
Sensor sensor;
Mqtt mqtt;

const int LIGHT_PIN = 10;
const int FAN_PIN = 11;
const int SENSOR_PIN = 12;

const unsigned long UPDATE_INTERVAL = 10 * 60 * 1000;
const unsigned long SENSOR_READ_INTERVAL = 30 * 1000;

void handleSwitch() {
  unsigned long switchIn = isOn ? (86400000 - duration) : duration;

  isOn = !isOn;

  Serial.printf("switching light %s\n", isOn ? "on" : "off");

  digitalWrite(LIGHT_PIN, isOn ? HIGH : LOW);

  ticker.once_ms(switchIn, handleSwitch);
}

void handleConfigurationMessage(const char* topic, const char* message) {
  const auto json = parseJson(message);

  isOn = json["isOn"];
  duration = json["duration"];
  fanSpeed = json["fanSpeed"];
  thresholdTemperature = json["thresholdTemperature"];

  Serial.printf(
    "light is %s. will be switched in %lu hours. fan speed is %d\n",
    isOn ? "on" : "off",
    json["switchIn"].as<unsigned long>() / 1000 / 60 / 60,
    fanSpeed
  );

  ticker.once_ms(json["switchIn"], handleSwitch);

  digitalWrite(LIGHT_PIN, isOn ? HIGH : LOW);
}

void handleRebootMessage(const char* topic, const char* message) {
  ESP.restart();
}

void sendEvent(const char* payload) {
  Serial.println("sending event...");

  mqtt.publish(("controllers/" + std::string(CONTROLLER_ID) + "/events/pub").c_str(), payload);
}

void onSensorData(float h, float t) {
  humidity = h;
  temperature = t;
}

void onSensorError(uint8_t error) {
  auto lastError = sensor.getLastError();
  auto payload = "{\"errorMessage\":\"" + std::string(lastError) + "\"}";

  Serial.printf("sensor error = %s\n", lastError);

  sendEvent(payload.c_str());
}


void setup() {
  Serial.begin(115200);

  wifi.setTrustAnchors(cacert);
  wifi.setClientCertificate(client_cert, privkey);
  wifi.connect(SSID, PASS);

  syncTyme();

  mqtt.connect(wifi.getClient(), MQTT_HOST, CONTROLLER_ID);

  mqtt.subscribe(("controllers/" + std::string(CONTROLLER_ID) + "/config/sub").c_str(), handleConfigurationMessage);
  mqtt.subscribe(("controllers/" + std::string(CONTROLLER_ID) + "/reboot/sub").c_str(), handleRebootMessage);

  mqtt.publish(("controllers/" + std::string(CONTROLLER_ID) + "/config/pub").c_str());

  sensor.setPin(SENSOR_PIN);
  sensor.setReadInterval(SENSOR_READ_INTERVAL);
  sensor.setHandlers(onSensorData, onSensorError);

  updateTicker.attach_ms(UPDATE_INTERVAL, [&]() {
    const auto payload = "{\"temperature\": " + std::to_string(temperature) + ",\"humidity\": " + std::to_string(humidity) + "}";

    sendEvent(payload.c_str());
  });
}

void loop() {
  mqtt.loop();
}
