#!/bin/bash

set -euo pipefail

COLOR_ERROR='\033[0;31m'
COLOR_NC='\033[0m'

DEFAULT_PIN_FAN=12
DEFAULT_PIN_LIGHT=14
DEFAULT_PIN_SENSOR=2

fail() {
    printf "${COLOR_ERROR}%s${COLOR_NC}\n" "$1" >&2
    exit 1
}

usage() {
    echo "usage: $0 <response.json> <output-directory>" >&2
    echo "" >&2
    echo "  response.json     body of POST /v2/controllers or POST /v2/controllers/{id}/certificate" >&2
    echo "  output-directory  where to write the filesystem image files, e.g. data.growbox" >&2
    exit 1
}

check_jq() {
    command -v jq &> /dev/null || fail "jq was not found. please, install it before continue"
}

read_field() {
    jq -er ".data // . | $1" "$response" || fail "$1 is missing in $response"
}

write_certificates() {
    printf "writing certificates...\n"

    read_field '.certificates.root' > "$output/root.crt"
    read_field '.certificates.pem' > "$output/controller.cert.pem"
    read_field '.certificates.privateKey' > "$output/controller.private.key"
    read_field '.certificates.publicKey' > "$output/controller.key"

    chmod 600 "$output/controller.private.key"
}

update_config() {
    printf "updating config.json, keeping wifi and pins...\n"

    local updated
    updated=$(jq --arg id "$controller_id" --arg host "$endpoint" '.controllerId = $id | .host = $host' "$config")

    echo "$updated" > "$config"
}

create_config() {
    printf "creating config.json...\n\n"

    local ssid password pin_fan pin_light pin_sensor

    read -r -p "wifi ssid = " ssid
    read -r -s -p "wifi password = " password
    echo ""
    read -r -p "pins.fan [$DEFAULT_PIN_FAN] = " pin_fan
    read -r -p "pins.light [$DEFAULT_PIN_LIGHT] = " pin_light
    read -r -p "pins.sensor [$DEFAULT_PIN_SENSOR] = " pin_sensor
    echo ""

    jq -n \
        --arg id "$controller_id" \
        --arg host "$endpoint" \
        --arg ssid "$ssid" \
        --arg password "$password" \
        --argjson fan "${pin_fan:-$DEFAULT_PIN_FAN}" \
        --argjson light "${pin_light:-$DEFAULT_PIN_LIGHT}" \
        --argjson sensor "${pin_sensor:-$DEFAULT_PIN_SENSOR}" \
        '{
            controllerId: $id,
            host: $host,
            wifi: { ssid: $ssid, password: $password },
            pins: { fan: $fan, light: $light, sensor: $sensor }
        }' > "$config"

    chmod 600 "$config"
}

main() {
    [ $# -eq 2 ] || usage

    response=$1
    output=$2
    config="$output/config.json"

    [ -f "$response" ] || fail "file not found: $response"

    check_jq

    controller_id=$(read_field '.controllerId')
    endpoint=$(read_field '.endpoint')

    mkdir -p "$output"

    write_certificates

    if [ -f "$config" ]; then
        update_config
    else
        create_config
    fi

    printf "\ncontroller %s written to %s\n" "$controller_id" "$output"
    printf "flash it with: PLATFORMIO_DATA_DIR=%s pio run -t uploadfs\n" "$output"
}

main "$@"
