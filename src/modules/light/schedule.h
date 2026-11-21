#pragma once

#include <stdint.h>

namespace modules {
    namespace schedule {
        constexpr uint32_t SECONDS_PER_DAY = 86400;

        inline bool isActive(uint32_t now, uint32_t start, uint32_t duration) {
            if (duration == 0) {
                return false;
            }

            if (duration >= SECONDS_PER_DAY) {
                return true;
            }

            uint32_t time = now % SECONDS_PER_DAY;
            uint32_t end = (start + duration) % SECONDS_PER_DAY;

            if (start < end) {
                return time >= start && time < end;
            }

            return time >= start || time < end;
        }

        inline bool parseTimeOfDay(const char* value, uint32_t& seconds) {
            if (!value) {
                return false;
            }

            auto isDigit = [](char c) { return c >= '0' && c <= '9'; };

            bool isFormatValid = isDigit(value[0]) && isDigit(value[1]) && value[2] == ':'
                && isDigit(value[3]) && isDigit(value[4]) && value[5] == '\0';

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
    }
}
