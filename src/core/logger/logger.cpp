#include <core/logger/logger.h>

namespace {
    constexpr size_t BUFFER_SIZE = 256;
    constexpr int TAG_WIDTH = 10;

    void write(const char* indicator, const char* tag, const char* format, va_list args) {
        char message[BUFFER_SIZE];
        vsnprintf(message, sizeof(message), format, args);

        unsigned long ms = millis();
        unsigned long seconds = ms / 1000;
        unsigned long tenths = (ms % 1000) / 100;

        Serial.printf("%5lu.%lus %s %-*s %s\n", seconds, tenths, indicator, TAG_WIDTH, tag, message);
    }
}

void core::Logger::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
    Serial.println();
    Serial.println("\xe2\x80\x94 garduino \xe2\x80\x94");
    Serial.println();
}

void core::Logger::info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    write("\xE2\x96\xB8", tag, format, args);
    va_end(args);
}

void core::Logger::error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    write("\xE2\x9C\x97", tag, format, args);
    va_end(args);
}
