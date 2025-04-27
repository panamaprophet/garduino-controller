#include <core/scheduler/scheduler.h>

void core::Scheduler::schedule(unsigned long interval, SchedulerCallback callback, bool repeat) {
    Serial.printf("[scheduler] scheduling %s call every %lu ms\n", repeat ? "recurrent" : "one-shot", interval);

    for (auto ticker = tickers.begin(); ticker != tickers.end();) {
        if (!ticker -> active()) {
            Serial.println("[scheduler] removing expired task");
            ticker = tickers.erase(ticker);
            break;
        }

        ticker++;
    }

    tickers.push_back(Ticker());

    if (repeat) {
        tickers.back().attach_ms(interval, callback);
    }

    if (!repeat) {
        tickers.back().once_ms(interval, callback);
    }
}

void core::Scheduler::scheduleOnce(unsigned long interval, SchedulerCallback callback) {
    schedule(interval, callback, false);
}
