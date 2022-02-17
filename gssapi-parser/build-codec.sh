#!/bin/bash
set -e

GREEN="\e[32;1m"
RESET="\e[0m"

echo -e "${GREEN}=== Building codec library ${RESET}"

if [ -d codec ]; then
    rm -rf codec
fi
mkdir codec
cd codec
asn1c  ../gssapi.asn1
cp ../libasncodec.mk .
make -f libasncodec.mk 