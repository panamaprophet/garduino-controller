# Changelog

## 2.0.0 (2026-11-23)


### ⚠ BREAKING CHANGES

* the v1 topics (events/pub, config/*, status/sub, reboot/sub, firmware/update/sub, firmware/version/*) are removed; the firmware speaks device protocol v2 only.

### Features

* device protocol v2 ([#7](https://github.com/panamaprophet/garduino-controller/issues/7)) ([68e952e](https://github.com/panamaprophet/garduino-controller/commit/68e952e5c1d4250563f5b8f6aa6b02fabf2023ad))
* firmware update ([9d67662](https://github.com/panamaprophet/garduino-controller/commit/9d676620453dfccdcc873824af98bb3cc43d97a1))
* move pins to config ([#2](https://github.com/panamaprophet/garduino-controller/issues/2)) ([85d5a22](https://github.com/panamaprophet/garduino-controller/commit/85d5a22161620baa8f658a519c84c312b1793ce2))
* scheduler with multi task support ([#3](https://github.com/panamaprophet/garduino-controller/issues/3)) ([d0e4e18](https://github.com/panamaprophet/garduino-controller/commit/d0e4e188b80508c7cbad787a1cd6301b873c78f6))
* support firmware/version mqtt topic ([80ecf37](https://github.com/panamaprophet/garduino-controller/commit/80ecf3794473363f82762b2b1098d6ac9efa3129))


### Bug Fixes

* add connection check into mqtt::publish ([25a47d8](https://github.com/panamaprophet/garduino-controller/commit/25a47d8fde3a85767fc30cf2b1e43ed00b4a4e32))
* staticjsondocument -&gt; jsondocument ([1fecf92](https://github.com/panamaprophet/garduino-controller/commit/1fecf92be63cd9c0ea994610aa6be180e9c184e3))
* use std::unique_ptr for ticker management ([#4](https://github.com/panamaprophet/garduino-controller/issues/4)) ([3b5cc18](https://github.com/panamaprophet/garduino-controller/commit/3b5cc184047b4bdf0eeeef9e496e1a2436658792))
