#!/bin/bash

COLOR_SUCCESS='\033[0;32m'
COLOR_ERROR='\033[0;31m'
COLOR_NC='\033[0m'

check_jq() {
    if ! command -v jq &> /dev/null; then
        printf "${COLOR_ERROR}jq was not found. please, install it before continue${COLOR_NC}\n" >&2
        exit 1
    fi
}

get_certificates() {
    aws iot list-certificates | jq -r '.certificates'
}

list_certificates() {
    echo "listing certificates..."
    echo ""

    local arns=($(echo $1 | jq -r '.[] | .certificateArn'))
    local dates=($(echo $1 | jq -r '.[] | .creationDate'))

    echo "available certificates:"

    for i in ${!arns[@]}
    do
        printf "\n$(expr $i + 1)) arn:  ${COLOR_SUCCESS}${arns[$i]}${COLOR_NC}\n   date: ${dates[$i]}\n"
    done

    echo ""
}

select_certificate() {
    local certificates=$(get_certificates)

    list_certificates "$certificates" >&2 # using stderr to separate return value from listing

    read -p "select certificate = " selected_certificate_number

    local arns=($(echo $certificates | jq -r '.[] | .certificateArn'))

    echo "${arns[selected_certificate_number-1]}"
}

create_thing() {
    local thing_name=$1

    result=$(aws iot create-thing --thing-name $thing_name)

    local thing_arn=$(echo $result | jq -r '.thingArn')
    local thing_id=$(echo $result | jq -r '.thingId')

    printf "created thing arn = ${COLOR_SUCCESS}$thing_arn${COLOR_NC}\n"
    printf "created thing id  = ${COLOR_SUCCESS}$thing_id${COLOR_NC}\n"
}

attach_thing_policy() {
    local thing_name=$1
    local policy=$2

    echo "attaching the policy..."

    aws iot attach-thing-principal --thing-name $thing_name --principal $policy
}

main() {
    check_jq

    read -p "thing name = " thing_name

    echo ""

    local selected_policy=$(select_certificate)

    create_thing $thing_name
    attach_thing_policy $thing_name $selected_policy

    echo "done."
}

main
