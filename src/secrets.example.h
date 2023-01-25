#include <Arduino.h>


const char* SSID = "xxxxxxx";
const char* PASS = "xxxxxxx";

const char* CONTROLLER_ID = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";

const char* MQTT_HOST = "domain.example";

static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

static const char privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";
