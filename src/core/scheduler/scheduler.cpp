#include <core/scheduler/scheduler.h>

void core::Scheduler::schedule(unsigned long interval, schedulerCallback callback) {
    Serial.printf("[scheduler] scheduling recurrent call every %ld seconds\n", interval / 1000);
    ticker.attach_ms(interval, callback);
};
