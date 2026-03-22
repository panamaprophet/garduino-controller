#include <core/time/time.h>

namespace {
    constexpr const char* NTP_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SECONDARY = "time.nist.gov";
    constexpr time_t MIN_VALID_TIME = 8 * 3600 * 2;
}

time_t core::Time::sync() {
    Serial.printf("[time] sync ");

    configTime(0, 0, NTP_PRIMARY, NTP_SECONDARY);

    time_t now = time(nullptr);

    while (now < MIN_VALID_TIME) {
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
