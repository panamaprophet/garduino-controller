#pragma once

namespace core {
    class Protocol {
        public:
            virtual ~Protocol() = default;

            virtual void begin() = 0;
            virtual void onConnected() = 0;
            virtual void publishTelemetry() = 0;
            virtual void publishChange(const char* name) = 0;
    };
}
