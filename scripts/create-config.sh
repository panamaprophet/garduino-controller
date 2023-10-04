#!/bin/bash

OUTPUT_DIRECTORY='data'
FILE_NAME='config.json'

echo "creating directory..."
echo ""

mkdir -p $OUTPUT_DIRECTORY

read -p "ssid = " WIFI_SSID
read -p "password = " WIFI_PASS
read -p "controller id (press enter to generate automatically) = " CONTROLLER_ID
read -p "host = " HOST

if [ ! "$CONTROLLER_ID" ]; then
    CONTROLLER_ID=$(uuidgen)
fi

echo "{
    \"ssid\": \"${WIFI_SSID}\",
    \"password\": \"${WIFI_PASS}\",
    \"controllerId\": \"${CONTROLLER_ID}\",
    \"host\": \"${HOST}\"
}" > "${OUTPUT_DIRECTORY}/${FILE_NAME}"

echo "done."
