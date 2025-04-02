#!/bin/bash

write_configuration() {
    printf "writing configuration to $5\n\n"

    echo "{
        \"wifi\": {
            \"ssid\": \"$1\",
            \"password\": \"$2\"
        },
        \"pins\": {
            \"fan\": $5,
            \"light\": $6,
            \"sensor\": $7
        },
        \"controllerId\": \"$3\",
        \"host\": \"$4\"
    }" > "$8"
}

check_directory() {
    local directory=$1

    if [ ! -d $directory ]; then
        printf "directory ${directory} doesn't exists. creating...\n\n"

        mkdir -p $directory
    fi
}

main() {
    local output_directory="data"
    local file_name="config.json"
    local path="$output_directory/$file_name"

    check_directory $output_directory

    read -p "ssid = " wifi_ssid
    read -p "password = " wifi_pass
    read -p "controller id (press enter to generate automatically) = " controller_id
    read -p "host = " host

    read -p "pins.fan = " pinFan
    read -p "pins.light = " pinLight
    read -p "pins.sensor = " pinSensor

    echo ""

    if [ ! $controller_id ]; then
        controller_id=$(uuidgen)
    fi

    write_configuration $wifi_ssid $wifi_pass $controller_id $host $pinFan $pinLight $pinSensor $path

    echo "done."
}

main
