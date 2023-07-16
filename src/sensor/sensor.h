#include <Arduino.h>
#include <DHT.h>

typedef void readCallback(float humidity, float temperature);
typedef void errorCallback(uint8_t error);

class Sensor {
    private:
        DHT11 sensor;
        Ticker ticker;
        int errorCount = 0;

    public:
        void setPin(uint8_t pin);
        void setReadInterval(unsigned long interval);
        void setHandlers(readCallback& onRead, errorCallback& onError);
        void read();
        const char* getLastError();
};
