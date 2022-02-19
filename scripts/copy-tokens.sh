#!/bin/bash

if [ -n "$1" ]; then
    DEST=$1
else
    DEST=.
fi

SERVICE_CONTAINER="krb5-server"
CLIENT_CONTAINER="krb5-client"

server_tokens=$(docker exec $SERVICE_CONTAINER ls | grep server_token)
for tok in $server_tokens; do
    docker cp $SERVICE_CONTAINER:$tok .
done

client_tokens=$(docker exec $CLIENT_CONTAINER ls | grep client_token)
for tok in $client_tokens; do
    docker cp $CLIENT_CONTAINER:$tok .
done
