#pragma once

#include <Ticker.h>
#include <vector>
#include <memory>

namespace core {
    typedef void (*SchedulerCallback)();

    class Scheduler {
        public:
            void schedule(unsigned long interval, SchedulerCallback callback, bool repeat = true);
            void scheduleOnce(unsigned long interval, SchedulerCallback callback);

        private:
            std::vector<std::unique_ptr<Ticker>> tickers;
    };
}
