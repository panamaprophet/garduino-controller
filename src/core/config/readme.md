## Config Example

Controller configuration with unique `controllerId` can be generated using `POST` request to `/configurations` endpoint of garduino backend (preferable way) or using `scripts/create-thing.sh` script.

Section `wifi` contains network credentials, section `pins` contains mappings between pins and modules.

```json
{
    "controllerId": "xxxxxx-xxxx-xxxx-xxxxxxx",

    "host": "https://some.endpoint.com",

    "wifi": {
        "ssid": "",
        "password": "",
    },

    "pins": {
        "fan": 12,
        "light": 14,
        "sensor": 2,
    }
}
```
