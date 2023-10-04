#!/bin/bash

if ! command -v jq &> /dev/null
then
    echo "jq was not found. please, install it before continue"
    exit 1
fi

read -p "thing name = " THING_NAME

echo ""

CERTIFICATES=$(aws iot list-certificates | jq -r '.certificates')
CERTIFICATE_ARNS=($(echo $CERTIFICATES | jq -r '.[] | .certificateArn'))
CERTIFICATE_DATES=($(echo $CERTIFICATES | jq -r '.[] | .creationDate'))

echo "available certificates:"

for i in ${!CERTIFICATE_ARNS[@]}
do
    echo ""
    echo "$(expr $i + 1)) arn: " ${CERTIFICATE_ARNS[$i]}
    echo "   date: " ${CERTIFICATE_DATES[$i]}
done

echo ""

read -p "select certificate = " SELECTED_CERTIFICATE_NUMBER

THING=$(aws iot create-thing --thing-name $THING_NAME)

echo ""

aws iot attach-thing-principal --thing-name $THING_NAME --principal ${CERTIFICATE_ARNS[$SELECTED_CERTIFICATE_NUMBER - 1]}

echo "created thing arn = `(echo $THING | jq -r '.thingArn')`" 
echo "created thing id  = `(echo $THING | jq -r '.thingId')`"

echo ""

echo "done."
