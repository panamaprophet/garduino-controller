#pragma once

#include <Arduino.h>
#include <stdarg.h>

namespace core {
    namespace Logger {
        void begin(unsigned long baudRate = 115200);
        void info(const char* tag, const char* format, ...);
        void error(const char* tag, const char* format, ...);
    }
}
