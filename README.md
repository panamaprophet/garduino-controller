# Garduino Controller

ESP8266 firmware for garduino garden controllers. It drives the light, the fan and a DHT11 sensor, and talks to the [garduino-aws](https://github.com/panamaprophet/garduino-aws) backend over AWS IoT Core using device protocol v2.

The backend owns the contract: topics and payloads are described in `garduino-aws/docs/events.md`, provisioning and firmware updates in `garduino-aws/docs/devices.md`.

## Protocol

Every message carries `"v": 2`. `<id>` is the controllerId, which is also the MQTT client id.

| Direction | Topic | Payload |
|---|---|---|
| device → cloud | `controllers/<id>/telemetry` | `{ "type": …, … }`, see below |
| device → cloud | `controllers/<id>/settings/get` | `{}`, sent at boot |
| cloud → device | `controllers/<id>/settings` | `{ "light": { "onTime", "durationMs" }, "fan": { "defaultSpeed" }, "sensor": { "thresholdTemperature" } }` |
| cloud → device | `controllers/<id>/commands` | `{ "type": "reboot" }`, `{ "type": "refresh" }`, `{ "type": "firmware.update", "url", "md5" }` |

Events published to `telemetry`:

| Type | When |
|---|---|
| `boot` | once per start: `firmwareVersion`, `protocolVersion`, `modules`, `resetReason` |
| `telemetry` | every 10 minutes and on `refresh`: the status of every module |
| `<module>:change` | a module's status changed: `light` on every switch, `fan` on every speed change, `sensor` when readings start failing and when they recover |
| `settings:applied` | settings were applied |
| `firmware:update` | `{ "status": "started" \| "success" \| "error", "message"? }` |

Connection status is not published by the firmware: AWS IoT reports connects and disconnects itself.

## Code structure

### Core

- `core/config`: reads `config.json` from the filesystem: Wi-Fi credentials, IoT endpoint, controllerId and pins.
- `core/network`: Wi-Fi and the TLS client, with certificates loaded from the filesystem.
- `core/mqtt`: PubSubClient wrapper that reconnects and resubscribes.
- `core/message-bus`: builds `controllers/<id>/<channel>` topics and serializes payloads with the protocol version.
- `core/event-bus`: in-process events between modules and `main`.
- `core/registry`: the module list; applies settings and collects statuses.
- `core/scheduler`: recurring and one-shot tasks.
- `core/time`: SNTP sync.
- `core/firmware`: over-the-air updates.

### Modules

A module implements `name()`, `apply(settings)` and `getStatus()`, and calls `changed()` when its status changes, which publishes `<module>:change`. Modules only read the settings fields that are present.

- `modules/light`: switches the light on a daily UTC schedule (`onTime`, `durationMs`), evaluated against wall-clock time every 5 seconds.
- `modules/fan`: PWM fan speed. Steps up while the temperature rises at or above the threshold, and returns to the default speed a minute after the light turns off.
- `modules/sensor`: polls the DHT11 every 30 seconds and reports `{ "error" }` instead of readings while it fails.

## Provisioning a controller

1. Create the controller through the API (`POST /v2/controllers`) and save the response body, for example as `growbox.json`. It holds the only copy of the private key.
2. Write the filesystem image files:

    ```sh
    scripts/provision.sh growbox.json data.growbox
    ```

    The script writes the certificates and asks for the Wi-Fi credentials and pins. `data.*/` folders are git-ignored.

    After a certificate rotation (`POST /v2/controllers/{id}/certificate`), run it again with the rotation response and the same folder: it replaces the certificates and keeps Wi-Fi and pins.

3. Flash the filesystem image and the firmware:

    ```sh
    PLATFORMIO_DATA_DIR=data.growbox pio run -t uploadfs
    pio run -t upload
    ```

Requires [jq](https://jqlang.org/).

## Development

```sh
pio run                 # build
pio device monitor      # serial output
```

The firmware version comes from the latest `v*` git tag (`scripts/inject-version.py`).

## Releases

Releases are automated with [release-please](https://github.com/googleapis/release-please) from conventional commits on `main`. Merging a release PR tags the version, builds the firmware and uploads `garduino-firmware-<version>.bin` to the firmware bucket and to the GitHub release. The build then shows up in `GET /v2/firmware` and can be sent to a controller with the `firmware.update` command.

The `firmware` GitHub environment needs the secrets `AWS_ACCESS_KEY_ID` and `AWS_SECRET_ACCESS_KEY` and the variables `AWS_REGION` and `FIRMWARE_BUCKET`.
