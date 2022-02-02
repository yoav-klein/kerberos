#!/bin/bash

#### Copy applications into containers

echo -e "\e[32;1m=== Copying applications to containers\e[0m"

client_container=krb5-client
server_container=krb5-server

docker cp bin/server/. ${server_container}:/server
docker cp bin/client/. ${client_container}:/client
