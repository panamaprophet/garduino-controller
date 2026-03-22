#include <core/firmware/firmware.h>
#include <ESP8266httpUpdate.h>

namespace core {
    Firmware::Firmware(WiFiClient& client) : _client(client) {}

    void Firmware::onStart(StartCallback callback) {
        _onStart = callback;
    }

    void Firmware::onSuccess(SuccessCallback callback) {
        _onSuccess = callback;
    }

    void Firmware::onError(ErrorCallback callback) {
        _onError = callback;
    }

    void Firmware::update(const char* url, const char* md5) {
        Serial.printf("[firmware] starting update using %s\n", url);

        if (_onStart) {
            _onStart();
        }

        httpUpdate.rebootOnUpdate(false);
        httpUpdate.closeConnectionsOnUpdate(false);
        httpUpdate.setMD5sum(md5);

        Serial.printf("[firmware] md5 checksum: %s\n", md5);

        httpUpdate.onProgress([](unsigned int progress, unsigned int total) {
            auto percentage = (progress * 100) / total;

            Serial.printf("\r[firmware] update: %u%%", percentage);
        });

        auto result = httpUpdate.update(_client, url);
        auto lastError = httpUpdate.getLastErrorString();
        auto isOk = result == HTTPUpdateResult::HTTP_UPDATE_OK;

        Serial.printf("[firmware] update result: %s\n", isOk ? "success" : lastError.c_str());

        if (result == HTTP_UPDATE_OK) {
            _onSuccess();
            return;
        }

        if (result == HTTP_UPDATE_NO_UPDATES) {
            _onError("no update");
            return;
        }

        if (result == HTTP_UPDATE_FAILED) {
            _onError(lastError.c_str());
        }
    }
}
