#!/bin/bash

COLOR_ERROR='\033[0;31m'
COLOR_NC='\033[0m'

check_jq() {
    if ! command -v jq &> /dev/null; then
        printf "${COLOR_ERROR}jq was not found. please, install it before continue${COLOR_NC}\n" >&2
        exit 1
    fi
}

check_directory() {
    local directory=$1

    if [ ! -d $directory ]; then
        printf "directory ${directory} doesn't exists. creating...\n\n"

        mkdir -p $directory
    fi
}

create_certificates() {
    printf "creating certificates...\n\n" >&2

    local directory=$1

    result=$(
        aws iot create-keys-and-certificate \
        --set-as-active \
        --certificate-pem-outfile $directory/controller.cert.pem \
        --public-key-outfile $directory/controller.key \
        --private-key-outfile $directory/controller.private.key
    )

    local created_certificate_arn=$(echo $result | jq -r '.certificateArn')

    echo $created_certificate_arn
}

is_policy_exists() {
    local policy_name=$1
    local policy=$(aws iot get-policy --policy-name $policy_name)

    echo $policy
}

create_policy() {
    printf "creating policy...\n\n"

    local policy_name=$1

    aws iot create-policy \
        --policy-name $policy_name \
        --policy-document \
            '{
                "Version": "2012-10-17",
                "Statement": [{
                    "Effect": "Allow",
                    "Action": "iot:Connect",
                    "Resource": "*"
                }, {
                    "Effect": "Allow",
                    "Action": "iot:Publish",
                    "Resource": "*"
                }, {
                    "Effect": "Allow",
                    "Action": "iot:Subscribe",
                    "Resource": "*"
                }, {
                    "Effect": "Allow",
                    "Action": "iot:Receive",
                    "Resource": "*"
                }]
            }'
}

attach_policy() {
    printf "attaching policy...\n\n"

    local policy_name=$1
    local certificate_arn=$2

    aws iot attach-policy --policy-name $policy_name --target $certificate_arn
}

download_root_certificate() {
    printf "downloading root certificate...\n\n"

    local url="https://www.amazontrust.com/repository/AmazonRootCA1.pem"
    local directory=$1

    curl -s $url > "$directory/root.crt"
}

main() {
    local output_directory="data"
    local policy_name="garduino-controller-Policy"

    check_jq
    check_directory

    if [[ ! $(is_policy_exists $policy_name) ]]; then
        create_policy $policy_name
    fi

    local created_certificate_arn=$(create_certificates $output_directory)

    attach_policy $policy_name $created_certificate_arn
    download_root_certificate $output_directory

    echo "done."
}

main
