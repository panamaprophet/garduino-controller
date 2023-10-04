#!/bin/bash

OUTPUT_DIRECTORY='data'
POLICY_NAME='garduino-controller-Policy'

if ! command -v jq &> /dev/null
then
    echo "jq was not found. please, install it before continue"
    exit 1
fi

echo "creating directory..."

mkdir -p $OUTPUT_DIRECTORY

echo "creating certificate..."

CREATED_CERTIFICATES=$(
    aws iot create-keys-and-certificate \
        --set-as-active \
        --certificate-pem-outfile $OUTPUT_DIRECTORY/controller.cert.pem \
        --public-key-outfile $OUTPUT_DIRECTORY/controller.key \
        --private-key-outfile $OUTPUT_DIRECTORY/controller.private.key
)

CERTIFICATE_ARN=$(echo $CREATED_CERTIFICATES | jq -r '.certificateArn')

echo "created certificate arn: ${CERTIFICATE_ARN}"

POLICY_EXISTING=$(aws iot get-policy --policy-name $POLICY_NAME)

if ! jq -e . >/dev/null 2>&1 <<< $POLICY_EXISTING
then
    echo "policy doesn't exist. creating..."

    aws iot create-policy \
        --policy-name $POLICY_NAME \
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
fi

echo "attaching policy..."

ATTACHED_POLICY=$(aws iot attach-policy --policy-name $POLICY_NAME --target $CERTIFICATE_ARN);

echo "downloading root certificate..."

curl -s "https://www.amazontrust.com/repository/AmazonRootCA1.pem" > "${OUTPUT_DIRECTORY}/root.crt"

echo "done."
