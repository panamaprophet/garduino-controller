#include <core/firmware/firmware.h>
#include <core/logger/logger.h>
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
        Logger::info("firmware", "starting update from %s", url);

        if (_onStart) {
            _onStart();
        }

        httpUpdate.rebootOnUpdate(false);
        httpUpdate.closeConnectionsOnUpdate(false);
        httpUpdate.setMD5sum(md5);

        Logger::info("firmware", "md5: %s", md5);

        httpUpdate.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("\r  updating... %u%%  ", (progress * 100) / total);
        });

        auto result = httpUpdate.update(_client, url);
        auto lastError = httpUpdate.getLastErrorString();

        Serial.println();

        if (result == HTTP_UPDATE_OK) {
            Logger::info("firmware", "update complete");
            _onSuccess();
            return;
        }

        if (result == HTTP_UPDATE_NO_UPDATES) {
            Logger::error("firmware", "no update available");
            _onError("no update");
            return;
        }

        if (result == HTTP_UPDATE_FAILED) {
            Logger::error("firmware", "update failed: %s", lastError.c_str());
            _onError(lastError.c_str());
        }
    }
}
