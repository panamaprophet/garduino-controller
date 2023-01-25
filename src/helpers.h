#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>


DynamicJsonDocument parseJson(const char* string) {
    Serial.printf("deserializing json = %s\n", string);

    DynamicJsonDocument json(JSON_OBJECT_SIZE(14));
    DeserializationError deserializationError = deserializeJson(json, string);

    Serial.printf("deserializing result = %s\n", deserializationError.c_str());

    return json;
};

auto syncTyme() {
    Serial.print("time sync");

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
};
