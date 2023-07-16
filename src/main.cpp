#include <Arduino.h>
#include <Ticker.h>
#include <LittleFS.h>
#include <vector>
#include <string>
#include <algorithm>

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

const int LIGHT_PIN = 14;
const int FAN_PIN = 12;
const int SENSOR_PIN = 2;

const int FAN_SPEED_MAX = 255;
const int FAN_SPEED_STEP = 50;

const unsigned long UPDATE_INTERVAL = 10 * 60 * 1000;
const unsigned long SENSOR_READ_INTERVAL = 30 * 1000;

unsigned long previousFanCheckInterval = millis();
unsigned long fanCheckInterval = 5000;

std::vector<int> temperatureHistory;


int getTemperatureStabilityFactor(std::vector<int> temperatures) {
  int items_count = static_cast<int>(temperatures.size());
  int slice_size = 5;

  int n = std::min(slice_size, items_count);
  int start_index = items_count - n + 1;

  int result = 0;

  for (int i = start_index; i < items_count; i++) {
    if (temperatures[i - 1] < temperatures[i]) {
      result += 1;
    }

    if (temperatures[i -1] > temperatures[i]) {
      result -= 1;
    }
  }

  return result;
}


void handleSwitch() {
  isOn = !isOn;

  auto switchIn = isOn ? (86400000 - duration) : duration;

  Serial.printf("light is %s. will be switched in %lu hours.\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60);

  analogWrite(LIGHT_PIN, isOn ? 255 : 0);

  ticker.once_ms(switchIn, handleSwitch);
}

void handleConfigurationMessage(const char* topic, const char* message) {
  const auto json = parseJson(message);

  isOn = json["isOn"];
  duration = json["duration"];
  fanSpeed = json["fanSpeed"];
  thresholdTemperature = json["thresholdTemperature"];

  unsigned long switchIn = json["switchIn"];

  analogWrite(FAN_PIN, fanSpeed);
  analogWrite(LIGHT_PIN, isOn ? 255: 0);

  ticker.once_ms(switchIn, handleSwitch);

  Serial.printf("light is %s. will be switched in %lu hours. Fan speed is %u\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, fanSpeed);
}

void handleRebootMessage(const char* topic, const char* message) {
  ESP.restart();
}

void handleStatusMessage(const char* topic, const char* message) {
  auto payload = (
    "{\"temperature\":" + String(temperature) +
    ",\"humidity\":" + String(humidity) +
    ",\"fanSpeed\": " + String(fanSpeed) +
    ",\"stabilityFactor\": " + String(getTemperatureStabilityFactor(temperatureHistory)) +
    ",\"isOn\": " + (isOn ? "true" : "false") +
    "}");

  mqtt.publish(("controllers/" + controllerId + "/status/pub").c_str(), payload.c_str());
}

void sendEvent(const char* payload) {
  if (DEBUG_NO_MQTT_EVENTS) {
    Serial.printf("dummy event send with payload %s\r\n", payload);

    return;
  }

  mqtt.publish(("controllers/" + controllerId + "/events/pub").c_str(), payload);
}

void onSensorData(float h, float t) {
  humidity = h;
  temperature = t;

  temperatureHistory.push_back(t);

  int temperatureFactor = getTemperatureStabilityFactor(temperatureHistory);
  int temperatureFactorThreshold = 2;

  bool hitFactorThreshold = temperatureFactorThreshold <= temperatureFactor;
  bool hitTemperatureThreshold = temperature >= thresholdTemperature;

  if (hitFactorThreshold || hitTemperatureThreshold) {
    fanSpeed = hitFactorThreshold ? fanSpeed + FAN_SPEED_STEP : FAN_SPEED_MAX;

    Serial.printf("temperature factor is raising, setting fan speed to %d\n", fanSpeed);

    analogWrite(FAN_PIN, fanSpeed);
  }

  Serial.printf("temperature = %.0f, humidity = %.0f, stability factor = %d\n", temperature, humidity, temperatureFactor);
}

void onSensorError(uint8_t error) {
  auto lastError = sensor.getLastError();
  auto payload = "{\"errorMessage\":\"" + std::string(lastError) + "\"}";

  sendEvent(payload.c_str());
}

const auto readFile(const char* name) {
  auto file = LittleFS.open(name, "r");
  auto result = file.readString();

  file.close();

  return result;
}


void setup() {
  LittleFS.begin();
  Serial.begin(115200);

  // config.json: { controllerId, host, ssid, password }
  auto config = readFile("config.json").c_str();
  auto jsonConfig = parseJson(config);

  controllerId = jsonConfig["controllerId"].as<std::string>();

  auto cacert = readFile("root.crt");
  auto clientCertificate = readFile("controller.cert.pem");
  auto privateKey = readFile("controller.private.key");

  wifi.setTrustAnchors(cacert.c_str());
  wifi.setClientCertificate(clientCertificate.c_str(), privateKey.c_str());

  auto isWiFiConnected = wifi.connect(jsonConfig["ssid"], jsonConfig["password"]);

  if (!isWiFiConnected) {
    Serial.println("wifi connection error");
    return;
  }

  syncTime();

  mqtt.connect(wifi.getClient(), jsonConfig["host"], controllerId.c_str());

  mqtt.subscribe(("controllers/" + controllerId + "/config/sub").c_str(), handleConfigurationMessage);
  mqtt.subscribe(("controllers/" + controllerId + "/reboot/sub").c_str(), handleRebootMessage);
  mqtt.subscribe(("controllers/" + controllerId + "/status/sub").c_str(), handleStatusMessage);

  mqtt.publish(("controllers/" + controllerId + "/config/pub").c_str());

  sensor.setPin(SENSOR_PIN);
  sensor.setReadInterval(SENSOR_READ_INTERVAL);
  sensor.setHandlers(onSensorData, onSensorError);

  sensor.read();

  updateTicker.attach_ms(UPDATE_INTERVAL, [&]() {
    const auto payload = "{\"temperature\": " + std::to_string(temperature) + ",\"humidity\": " + std::to_string(humidity) + "}";

    sendEvent(payload.c_str());
  });
}

void loop() {
  mqtt.loop();
}
