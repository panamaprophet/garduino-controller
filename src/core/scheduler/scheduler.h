#include <Ticker.h>

namespace core {
    typedef void (*schedulerCallback)();

    class Scheduler {
        private:
            Ticker ticker;

        public:
            void schedule(unsigned long interval, schedulerCallback callback);
    };
}
