#include <modules.h>


modules::Fan::Fan(int _pin) {
    pin = _pin;
    pinMode(pin, OUTPUT);
    analogWrite(pin, minSpeed);
};

void modules::Fan::run () {
    currentSpeed = defaultSpeed;
    analogWrite(pin, currentSpeed);
};

void modules::Fan::stepUp() {
    currentSpeed += step;
    analogWrite(pin, currentSpeed);
};

void modules::Fan::reset () {
    currentSpeed = defaultSpeed;
    analogWrite(pin, currentSpeed);
};


modules::Light::Light(int _pin) {
    pin = _pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
};

void modules::Light::run () {
    ticker.attach_ms(checkInterval, [&]() {
        switchIn -= checkInterval;

        if (switchIn < 0) {
            isOn = !isOn;
            switchIn = isOn ? duration : 86400000 - duration;   

            // Serial.printf("switching. light is %s. will be switched in %lu hours (%lu ms).\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, switchIn);

            digitalWrite(pin, isOn ? LOW : HIGH);

            // onSwitch(isOn, switchIn)
        }

        // sendSwitchEvent();

        // if (!isOn) {
        //     Serial.printf("timer will be reset in %lu minutes", COOL_DOWN_INTERVAL / 1000 / 60);

        //     cooldownTicker.once_ms(COOL_DOWN_INTERVAL, [](){ 
        //     fanSpeedController.reset();
        //     });
        // }
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
            // trigger threshold event
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
            //   Serial.printf("error reading sensor data = %s\n", sensor.getLastError());
        }
    });
};

void modules::Sensor::run() {
    ticker.attach_ms(interval, [&]() {
        sensor.read();
    });
};
