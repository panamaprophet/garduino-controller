#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266HTTPUpdate.h>
#include <functional>

namespace core {
    class Firmware {
        private:
            WiFiClient& _client;
            ESP8266HTTPUpdate httpUpdate;

            typedef std::function<void()> StartCallback;
            typedef std::function<void()> SuccessCallback;
            typedef std::function<void(const char*)> ErrorCallback;

            StartCallback _onStart;
            SuccessCallback _onSuccess;
            ErrorCallback _onError;

        public:
            Firmware(WiFiClient& client);

            void onStart(StartCallback callback);
            void onSuccess(SuccessCallback callback);
            void onError(ErrorCallback callback);

            void update(const char* url, const char* md5);
    };
}
