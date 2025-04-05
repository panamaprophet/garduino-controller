#include <modules/light/light.h>

modules::Light::Light(int _pin) {
    pin = _pin;
    pinMode(pin, OUTPUT);
    analogWrite(pin, PIN_OFF);
};

void modules::Light::run() {
    Serial.printf("[module:light] run. light is %s. will be switched in %lu hours (%lu ms).\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, switchIn);

    ticker.attach_ms(checkInterval, [&]() {
        switchIn -= checkInterval;

        if (switchIn < 0) {
            isOn = !isOn;
            switchIn = isOn ? duration : 86400000 - duration;

            Serial.printf("[module:light] switch. light is %s. will be switched in %lu hours (%lu ms).\n", isOn ? "on" : "off", switchIn / 1000 / 60 / 60, switchIn);

            analogWriteFreq(1000);
            analogWrite(pin, isOn ? PIN_ON : PIN_OFF);

            onSwitchCallback(isOn, switchIn);
        }
        });

    analogWriteFreq(1000);
    analogWrite(pin, isOn ? PIN_ON : PIN_OFF);
};

void modules::Light::onSwitch(switchCallback callback) {
    onSwitchCallback = callback;
};
