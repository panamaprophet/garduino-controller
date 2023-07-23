#include <sensor/sensor.h>


void Sensor::setPin(uint8_t pin) {
    sensor.setup(pin);
};

void Sensor::setReadInterval(unsigned long interval) {
    ticker.attach_ms(interval, [&]() { sensor.read(); });
};

void Sensor::setHandlers(readCallback& onRead, errorCallback& onError) {
    sensor.onData(onRead);
    sensor.onError([&](uint8_t error) {
        errorCount++;

        if (errorCount <= 5) {
            sensor.read();
        } else {
            errorCount = 0;
            onError(error);
        }
    });
};

const char* Sensor::getLastError() {
    return sensor.getError();
}

void Sensor::read() {
    sensor.read();
}
