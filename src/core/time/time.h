#pragma once

#include <time.h>
#include <Arduino.h>

namespace core {
    class Time {
        public:
            static constexpr uint32_t SECONDS_PER_DAY = 86400;

            time_t sync();

            static uint32_t secondsToday();
            static bool parseTimeToSeconds(const char* value, uint32_t& seconds);
    };
}
