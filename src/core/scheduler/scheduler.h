#include <Ticker.h>
#include <vector>

namespace core {
    typedef void (*SchedulerCallback)();

    class Scheduler {
        private:
            std::vector<std::unique_ptr<Ticker>> tickers;

        public:
            void schedule(unsigned long interval, SchedulerCallback callback, bool repeat = true);
            void scheduleOnce(unsigned long interval, SchedulerCallback callback);
    };
}
