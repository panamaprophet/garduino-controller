#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ENV_FILE="$SCRIPT_DIR/../.env.local"

BINARY_PATH=".pio/build/nodemcuv2/firmware.bin"

COLOR_ERROR='\033[0;31m'
COLOR_NC='\033[0m'

if [ -f "$ENV_FILE" ]; then
    source "$ENV_FILE"
fi

if [ -z "$AWS_BUCKET" ]; then
    printf "${COLOR_ERROR}AWS_BUCKET is not set. Define it in .env.local${COLOR_NC}\n" >&2
    exit 1
fi



check_binary() {
    if [ ! -f "$BINARY_PATH" ]; then
        printf "${COLOR_ERROR}build artifact not found: $BINARY_PATH${COLOR_NC}\n" >&2
        printf "run: pio run\n" >&2
        exit 1
    fi
}

get_firmware_name() {
    local date=$(date +%Y%m%d)
    local git_hash=$(git describe --tags --dirty --always)

    echo "garduino-firmware.${git_hash}.${date}.bin" # e.g. garduino-firmware.gabcdef0.20240601.bin
}

publish_firmware() {
    local name=$1

    printf "publishing $BINARY_PATH as $name\n\n"

    aws s3 cp "$BINARY_PATH" "$AWS_BUCKET/$name"

    printf "published: $AWS_BUCKET/$name\n"
}

main() {
    check_binary

    local name=$(get_firmware_name)
    publish_firmware "$name"

    echo "done."
}

main
