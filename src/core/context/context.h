#pragma once

#include <core/scheduler/scheduler.h>
#include <core/event-bus/event-bus.h>

namespace core {
    struct Context {
        Scheduler& scheduler;
        EventBus& eventBus;
    };
}
