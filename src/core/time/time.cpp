#include <core/time/time.h>

time_t core::Time::sync() {
    Serial.printf("[time] sync ");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);

    while (now < 8 * 3600 * 2) {
        delay(10);
        Serial.printf(".");
        now = time(nullptr);
    }

    Serial.printf(" success\n");

    struct tm timeinfo;

    gmtime_r(&now, &timeinfo);

    Serial.printf("[time] current time = %s", asctime(&timeinfo));

    return now;
};
