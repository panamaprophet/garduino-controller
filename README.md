# Garduino Controller

## Code structure

### Core:

- `core/config`

    Reads the config from the config.json, provides basic configuration for controller, such as *wi-fi credentials*, *mqtt endpoint* and *controllerId*.

- `core/network`

    Provides secure internet access over wi-fi using certificates loaded from the filesystem.

- `core/mqtt`

    Provides an interface to PubSub client, automatically reconnect.

- `core/time`

    Time sync and utilites.

- `core/scheduler`

    Scheduler for recurrent tasks. Currently a wrapper for ticker's *attach_ms* method.

### Modules:

- `modules/fan`

    Fan speed controller. Exposes current fan speed, reset() and stepup() functions

- `modules/light`

    Light controller. Controls the light cycle.

- `modules/sensor`

    Polling the temperature and humidity sensor, 

### The following mqtt topics supported:

- `controllers/<controllerId>/configuration/sub`.

    once the configuration received it configures (or re-configures) the modules.

- `controllers/<controllerId>/reboot/sub`:

    Reboots the controller.

- `controllers/<controllerId>/status/sub`:

    Send the status collected from modules.


## Scripts

- `scripts/create-certificates.sh`

- `scripts/create-config.sh`

- `scripts/create-thing.sh`
