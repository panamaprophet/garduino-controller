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

uint32_t core::Time::secondsOfDay() {
    return static_cast<uint32_t>(time(nullptr) % SECONDS_PER_DAY);
}

bool core::Time::parseTimeOfDay(const char* value, uint32_t& seconds) {
    if (!value) {
        return false;
    }

    bool isFormatValid = isdigit(value[0]) && isdigit(value[1]) && value[2] == ':'
        && isdigit(value[3]) && isdigit(value[4]) && value[5] == '\0';

    if (!isFormatValid) {
        return false;
    }

    uint32_t hours = (value[0] - '0') * 10 + (value[1] - '0');
    uint32_t minutes = (value[3] - '0') * 10 + (value[4] - '0');

    if (hours > 23 || minutes > 59) {
        return false;
    }

    seconds = hours * 3600 + minutes * 60;

    return true;
}
