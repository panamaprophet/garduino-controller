#pragma once

#include <Ticker.h>
#include <vector>
#include <memory>
#include <functional>

namespace core {
    using SchedulerCallback = std::function<void()>;

    class Scheduler {
        public:
            void schedule(unsigned long interval, SchedulerCallback callback, bool repeat = true);
            void scheduleOnce(unsigned long interval, SchedulerCallback callback);

        private:
            std::vector<std::unique_ptr<Ticker>> tickers;
    };
}
