#include <modules.h>


modules::Fan::Fan(int _pin) {
    pin = _pin;
    pinMode(pin, OUTPUT);
    analogWrite(pin, minSpeed);
};

void modules::Fan::setSpeed(unsigned int speed) {
    currentSpeed = std::min(std::max(minSpeed, speed), maxSpeed);

    Serial.printf("[module:fan] set speed to: %d (requested %d)\n", currentSpeed, speed);

    analogWrite(pin, currentSpeed);
}

void modules::Fan::run() {
    Serial.printf("[module:fan] run\n");

    setSpeed(defaultSpeed);
};

void modules::Fan::stepUp() {
    setSpeed(currentSpeed + step);
};

void modules::Fan::reset() {
    setSpeed(defaultSpeed);
};


modules::Light::Light(int _pin) {
    pin = _pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
};

void modules::Light::run () {
    Serial.printf("[module:light] run. light is %s. will be switched in %lu hours (%lu ms).\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, switchIn);

    ticker.attach_ms(checkInterval, [&]() {
        switchIn -= checkInterval;

        if (switchIn < 0) {
            isOn = !isOn;
            switchIn = isOn ? duration : 86400000 - duration;   

            Serial.printf("[module:light] switch. light is %s. will be switched in %lu hours (%lu ms).\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, switchIn);

            digitalWrite(pin, isOn ? LOW : HIGH);

            // @todo: add api to act on switch
            // onSwitch(isOn, switchIn)
        }
    });

    digitalWrite(pin, isOn ? LOW : HIGH);
};


typedef void readCallback(float humidity, float temperature);
typedef void errorCallback(unsigned int error);


modules::Sensor::Sensor(int _pin) {
    pin = _pin;
    sensor.setup(pin);

    sensor.onData([&](float newHumidity, float newTemperature) {
        if (newTemperature < temperature) {
            stabilityFactor--;
        }

        if (newTemperature > temperature) {
            stabilityFactor++;
        }

        if (newTemperature == temperature) {
            stabilityFactor > 0 ? stabilityFactor-- : stabilityFactor++;
        }

        if (newTemperature >= thresholdTemperature && newTemperature > temperature) {
            // @todo: add api to act on threshold
            // trigger threshold event
            Serial.printf("[module:sensor] threshold reached\n");
        }

        humidity = newHumidity;
        temperature = newTemperature;
    });

    sensor.onError([&](uint8_t error) {
        retryCounter++;

        if (retryCounter <= retryCount) {
            sensor.read();
        } else {
            retryCounter = 0;
            Serial.printf("[module:sensor] error reading sensor data: %s\n", sensor.getError());
        }
    });
};

void modules::Sensor::run() {
    Serial.printf("[module:sensor] run. polling interval set to %d seconds\n", interval / 1000);

    ticker.attach_ms(interval, [&]() {
        sensor.read();
    });
};
