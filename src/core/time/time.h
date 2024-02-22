#include <time.h>
#include <Arduino.h>

namespace core {
    class Time {
        public:
            time_t sync();
    };
}
