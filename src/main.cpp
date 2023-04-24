#include <Arduino.h>
#include <Ticker.h>
#include <LittleFS.h>
#include <vector>
#include <string>

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

std::string controllerId = "";

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

  mqtt.publish(("controllers/" + controllerId + "/events/pub").c_str(), payload);
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

const char* readFile(const char* name) {
  auto file = LittleFS.open(name, "r");
  auto result = file.readString();

  file.close();

  return result.c_str();
}


void setup() {
  LittleFS.begin();
  Serial.begin(115200);

  // config.json: { controllerId, host, ssid, password }
  auto config = readFile("config.json");
  auto jsonConfig = parseJson(config);

  controllerId = jsonConfig["controllerId"].as<std::string>();

  auto cacert = readFile("root.crt");
  auto clientCertificate = readFile("controller.cert.pem");
  auto privateKey = readFile("controller.private.key");

  wifi.setTrustAnchors(cacert);
  wifi.setClientCertificate(clientCertificate, privateKey);
  wifi.connect(jsonConfig["ssid"], jsonConfig["password"]);

  syncTime();

  mqtt.connect(wifi.getClient(), jsonConfig["host"], controllerId.c_str());
  mqtt.subscribe(("controllers/" + controllerId + "/config/sub").c_str(), handleConfigurationMessage);
  mqtt.subscribe(("controllers/" + controllerId + "/reboot/sub").c_str(), handleRebootMessage);
  mqtt.publish(("controllers/" + controllerId + "/config/pub").c_str());

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
