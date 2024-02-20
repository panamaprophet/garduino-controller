#include <Arduino.h>
#include <Ticker.h>
#include <LittleFS.h>
#include <string>
#include <time.h>
#include <sensor/sensor.h>
#include <fanSpeedController/fanSpeedController.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool isOn;

unsigned long duration = 12 * 60 * 60 * 1000;
unsigned int thresholdTemperature = 30;

long switchIn = 12 * 60 * 60 * 1000;

float humidity;
float temperature;
float stabilityFactor = 0;

std::string controllerId = "";

Sensor sensor;
Ticker ticker;
Ticker updateTicker;
Ticker cooldownTicker;
Ticker rebootTicker;

BearSSL::WiFiClientSecure wifi;
PubSubClient mqtt(wifi);

FanSpeedController fanSpeedController;

const int LIGHT_PIN = 14;
const int FAN_PIN = 12;
const int SENSOR_PIN = 2;

const unsigned long UPDATE_INTERVAL = 10 * 60 * 1000;
const unsigned long SENSOR_READ_INTERVAL = 30 * 1000;
const unsigned long CYCLE_TICKER_INTERVAL = 1 * 5000;
const unsigned long COOL_DOWN_INTERVAL = 5 * 60 * 1000;

time_t syncTime() {
    Serial.print("time sync...");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);

    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }

    struct tm timeinfo;

    gmtime_r(&now, &timeinfo);

    Serial.printf("\ncurrent time = %s", asctime(&timeinfo));

    return now;
}

void sendEvent(const char* payload) {
  Serial.printf("sending event, payload = %s\n", payload);

  mqtt.publish(("controllers/" + controllerId + "/events/pub").c_str(), payload);
}

void sendRunEvent() {
  char payload[40];

  sprintf(payload, "{\"isOn\": %s, \"event\": \"run\"}", isOn ? "true" : "false");

  sendEvent(payload);
}

void sendSwitchEvent() {
  char payload[40];

  sprintf(payload, "{\"isOn\": %s, \"event\": \"switch\"}", isOn ? "true" : "false");

  sendEvent(payload);
}

void sendUpdateEvent() {
  char payload[100];

  sprintf(payload, "{\"temperature\":%.2f,\"humidity\":%.2f,\"fanSpeed\": %.2d,\"event\":\"update\"}", temperature, humidity, fanSpeedController.currentSpeed);

  sendEvent(payload); 
}

void handleLightSwitch() {
  switchIn -= CYCLE_TICKER_INTERVAL;

  if (switchIn > 0) {
    return;
  }

  isOn = !isOn;

  switchIn = isOn ? duration : 86400000 - duration;

  Serial.printf("switching. light is %s. will be switched in %lu hours (%lu ms).\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, switchIn);

  digitalWrite(LIGHT_PIN, isOn ? LOW : HIGH);

  sendSwitchEvent();

  if (!isOn) {
    Serial.printf("timer will be reset in %lu minutes", COOL_DOWN_INTERVAL / 1000 / 60);

    cooldownTicker.once_ms(COOL_DOWN_INTERVAL, [](){ 
      fanSpeedController.reset();
    });
  }
}

void handleRebootMessage() {
  char payload[50];
  const uint32_t interval = 5 * 1000;

  sprintf(payload, "{\"event\":\"reboot\", \"scheduledIn\":%lu}", interval);

  sendEvent(payload);

  rebootTicker.once_ms(interval, [](){
    ESP.restart();
  });
};

void handleStatusMessage() {
  char payload[100];

  sprintf(
    payload, 
    "{\"temperature\":%.2f,\"humidity\":%.2f,\"isOn\":%s,\"fanSpeed\":%d,\"stabilityFactor\":%.2f}",
    temperature, 
    humidity, 
    isOn ? "true" : "false", 
    fanSpeedController.currentSpeed,
    stabilityFactor
  );

  mqtt.publish(("controllers/" + controllerId + "/status/pub").c_str(), payload);
}

void handleConfigurationMessage(const byte* message) {
    Serial.println("mqtt: configuration received");
    
    StaticJsonDocument<200> json;
    deserializeJson(json, (char *)message);

    isOn = json["isOn"].as<boolean>();
    duration = json["duration"].as<unsigned long>();
    thresholdTemperature = json["thresholdTemperature"].as<unsigned int>();
    switchIn = json["switchIn"].as<unsigned long>();

    unsigned int fanSpeed = json["fanSpeed"].as<unsigned int>();

    json.clear();

    fanSpeedController.setup(FAN_PIN, fanSpeed);

    digitalWrite(LIGHT_PIN, isOn ? LOW : HIGH);

    sensor.read();

    Serial.printf(
      "light is %s. will be switched in %lu hours (%lu ms). fan speed is %u. threshold temperature is %u\n", 
      isOn ? "on" : "off", 
      switchIn / 1000 / 60 / 60, 
      switchIn, 
      fanSpeedController.currentSpeed, 
      thresholdTemperature
    );

    sendRunEvent();
}

void onSensorData(float newHumidity, float newTemperature) {
  if (newTemperature < temperature) {
    stabilityFactor--;
  }

  if (newTemperature > temperature) {
    stabilityFactor++;
  }

  if (newTemperature == temperature) {
    stabilityFactor > 0 ? stabilityFactor-- : stabilityFactor++;
  }

  if (
    newTemperature >= thresholdTemperature && 
    newTemperature > temperature && 
    fanSpeedController.currentSpeed < fanSpeedController.max
  ) {
    fanSpeedController.stepUp();
    sendUpdateEvent();
  }

  humidity = newHumidity;
  temperature = newTemperature;
}

void onSensorError(uint8_t error) {
  Serial.printf("error reading sensor data = %s\n", sensor.getLastError());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  const std::string mqtt_topic_configuration = "controllers/" + controllerId + "/config/sub";
  const std::string mqtt_topic_reboot = "controllers/" + controllerId + "/reboot/sub";
  const std::string mqtt_topic_status = "controllers/" + controllerId + "/status/sub";

  if (mqtt_topic_configuration.compare(topic) == 0) {
    return handleConfigurationMessage(payload);
  }
  
  if (mqtt_topic_reboot.compare(topic) == 0) {
    return handleRebootMessage();
  }

  if (mqtt_topic_status.compare(topic) == 0) {
    return handleStatusMessage();
  }

  Serial.println("no handler for message was found");
}

std::string mqtt_host = "";
uint16 mqtt_port = 8883;

void mqttConnect() {
  Serial.printf("mqtt connection: ");

  mqtt.setServer(mqtt_host.c_str(), mqtt_port);
  mqtt.setCallback(mqttCallback);
  mqtt.connect(controllerId.c_str());

  Serial.printf("%s\n", mqtt.connected() ? "success" : "fail");

  if (!mqtt.connected()) {
    char message[80];

    int code = wifi.getLastSSLError(message, sizeof(message));

    Serial.printf("mqtt connection failed. error code = %d\n", mqtt.state());
    Serial.printf("mqtt connection failed. ssl error = %d: %s\n", code, message);

    return;
  }

  mqtt.subscribe(("controllers/" + controllerId + "/config/sub").c_str());
  mqtt.subscribe(("controllers/" + controllerId + "/reboot/sub").c_str());
  mqtt.subscribe(("controllers/" + controllerId + "/status/sub").c_str());
}

void setup() {
  LittleFS.begin();
  Serial.begin(115200);

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  analogWrite(FAN_PIN, 0);
  digitalWrite(LIGHT_PIN, HIGH);

  auto file = LittleFS.open("config.json", "r");
  auto size = file.size();

  char *config = new char[size];

  file.readBytes(config, size);
  file.close();

  StaticJsonDocument<200> json;
  deserializeJson(json, config);

  controllerId = json["controllerId"].as<std::string>();

  // === root cert ===

  auto rootCertificateFile = LittleFS.open("root.crt", "r");
  auto rootCertificateSize = rootCertificateFile.size();
  char *rootCertificateBuffer = new char[rootCertificateSize];

  rootCertificateFile.readBytes(rootCertificateBuffer, rootCertificateSize);
  rootCertificateFile.close();

  const BearSSL::X509List* cert = new BearSSL::X509List(rootCertificateBuffer);

  wifi.setTrustAnchors(cert);

  // == client cert ===

  auto clientCertificateFile = LittleFS.open("controller.cert.pem", "r");
  auto clientCertificateSize = clientCertificateFile.size();

  char *clientCertificateBuffer = new char[clientCertificateSize];

  clientCertificateFile.readBytes(clientCertificateBuffer, clientCertificateSize);
  clientCertificateFile.close();

  //  === private key ===

  auto privateKeyFile = LittleFS.open("controller.private.key", "r");
  auto privateKeySize = privateKeyFile.size();

  char *privateKeyBuffer = new char[privateKeySize];

  privateKeyFile.readBytes(privateKeyBuffer, privateKeySize);
  privateKeyFile.close();

  const BearSSL::X509List* clientCertificate = new BearSSL::X509List(clientCertificateBuffer);
  const BearSSL::PrivateKey* key = new BearSSL::PrivateKey(privateKeyBuffer);

  std::string ssid = json["ssid"].as<std::string>();
  std::string password = json["password"].as<std::string>();

  wifi.setClientRSACert(clientCertificate, key);
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.printf("\nconnecting to wifi");

  unsigned int timeout = 15000;
  unsigned int step = 500;

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');

    delay(step);
    timeout -= step;

    if (timeout <= 0) {
      Serial.print("timeout");
    }
  }

  Serial.println();
  Serial.print("ip = ");
  Serial.println(WiFi.localIP());

  syncTime();

  mqtt_host = json["host"].as<std::string>();
  mqtt_port = 8883;

  json.clear();

  Serial.printf("controller id = %s\nmqtt host = %s\nmqtt port = %d\n", controllerId.c_str(), mqtt_host.c_str(), mqtt_port);

  sensor.setPin(SENSOR_PIN);
  sensor.setReadInterval(SENSOR_READ_INTERVAL);
  sensor.setHandlers(onSensorData, onSensorError);

  updateTicker.attach_ms(UPDATE_INTERVAL, sendUpdateEvent);

  ticker.attach_ms(CYCLE_TICKER_INTERVAL, handleLightSwitch);

  LittleFS.end();

  Serial.printf("free memory = %d bytes\n", ESP.getFreeHeap());

  mqttConnect();

  Serial.printf("request config\n");

  mqtt.publish(("controllers/" + controllerId + "/config/pub").c_str(), "{}");

  Serial.printf("setup finished\n");
}

void loop() {
  if (!mqtt.connected()) {
    Serial.println("mqtt disconnected. reconnecting...");
    mqttConnect();
  }

  mqtt.loop();
}
