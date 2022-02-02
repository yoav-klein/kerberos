#!/bin/bash

#########################
#
#   create-deployment.sh
#   
#   create a deployment directory
#   with the files to copy to the containers
#
#########################

GREEN="\e[32;1m"
RESET="\e[0m"

set -e

function create_bin_dir() {
    mkdir bin
    mkdir bin/server
    mkdir bin/client
}

if ! [ -d bin ]; then
    create_bin_dir
else
    read -p "bin directory exists, override? y/n" CONFIRM
    if [ $CONFIRM = "y" ]; then
        rm -rf bin
        create_bin_dir
    else
        exit
    fi
fi

cp application/krb5-api/server bin/server/krb5-server
cp application/krb5-api/client bin/client/krb5-client

cp application/gssapi/server bin/server/gss-server
cp application/gssapi/client bin/client/gss-client

echo -e "${GREEN}=== Created deployment${RESET}"