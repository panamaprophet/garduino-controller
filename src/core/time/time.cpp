#include <core/time/time.h>
#include <core/logger/logger.h>

namespace {
    constexpr const char* NTP_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SECONDARY = "time.nist.gov";
    constexpr time_t MIN_VALID_TIME = 8 * 3600 * 2;
}

time_t core::Time::sync() {
    Logger::info("time", "syncing...");

    configTime(0, 0, NTP_PRIMARY, NTP_SECONDARY);

    time_t now = time(nullptr);

    while (now < MIN_VALID_TIME) {
        delay(10);
        now = time(nullptr);
    }

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);

    char formatted[32];
    strftime(formatted, sizeof(formatted), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Logger::info("time", "synced (%s)", formatted);

    return now;
};

uint32_t core::Time::secondsToday() {
    return static_cast<uint32_t>(time(nullptr) % SECONDS_PER_DAY);
}

bool core::Time::parseTimeToSeconds(const char* value, uint32_t& seconds) {
    unsigned int hours;
    unsigned int minutes;
    char rest;

    bool isParsed = value && sscanf(value, "%2u:%2u%c", &hours, &minutes, &rest) == 2;

    if (!isParsed || hours > 23 || minutes > 59) {
        return false;
    }

    seconds = hours * 3600 + minutes * 60;

    return true;
}
