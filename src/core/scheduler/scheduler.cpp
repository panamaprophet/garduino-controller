#include <core/scheduler/scheduler.h>

void core::Scheduler::schedule(unsigned long interval, schedulerCallback callback) {
    ticker.attach_ms(interval, callback);
};
