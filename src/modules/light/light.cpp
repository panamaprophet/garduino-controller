#include <modules/light/light.h>

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

            onSwitchCallback(isOn, switchIn);
        }
    });

    digitalWrite(pin, isOn ? LOW : HIGH);
};

void modules::Light::onSwitch(switchCallback callback) {
    onSwitchCallback = callback;
};
