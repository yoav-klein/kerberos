#!/bin/bash

#### Copy applications into containers

client_container=krb5-client
server_container=krb5-server

docker cp bin/server ${server_container}:/server
docker cp bin/client ${client_container}:/client
