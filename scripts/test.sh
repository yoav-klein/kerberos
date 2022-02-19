#!/bin/bash

#####################
#	
#	Run the server and client applications for test	
#
#######################

PORT=8081
GREEN="\e[32;1m"
RESET="\e[0m"
SERVICE_NAME="host"
SERVICE_HOST="krb5-server.myexample.com"
SERVICE_CONTAINER="krb5-server"
CLIENT_CONTAINER="krb5-client"

set -e

init_client() {
    docker exec ${CLIENT_CONTAINER} /bin/bash -c "echo \"yoav\" | kinit yoav"
}

test_krb5api() {
    echo -e "${GREEN}=== Running Krb5-API application for test ${RESET}"
    echo -e "${GREEN}Running server in server container${RESET}"
    docker exec -d ${SERVICE_CONTAINER} /bin/bash -c "/server/krb5-server -h ${SERVICE_HOST} -p ${PORT} -s ${SERVICE_NAME}"
    docker exec ${CLIENT_CONTAINER} /bin/bash -c "/client/krb5-client -h ${SERVICE_HOST} -p ${PORT}"
}

test_gssapi() {
    if [ -n "$1" ]; then
        mech_opt="-m $1"
    fi
    echo -e "${GREEN}=== Running GSS-API application for test ${RESET}"
    echo -e "${GREEN}Running server in server container${RESET}"
    docker exec -d ${SERVICE_CONTAINER} /bin/bash -c "/server/gss-server -r -p ${PORT} ${SERVICE_NAME}"
    docker exec ${CLIENT_CONTAINER} /bin/bash -c "/client/gss-client -r ${mech_opt} -p ${PORT} ${SERVICE_HOST} ${SERVICE_NAME}@${SERVICE_HOST} \"This is my message !\""
}

test_ssh() {
    echo -e "${GREEN}=== Connecting with SSH from client to server using GSSAPI${RESET}"
    echo -e "${GREEN}Running ssh server in server container${RESET}"
    docker exec ${SERVICE_CONTAINER} /bin/bash -c "service ssh start"
    echo -e "${GREEN}Connecting from client container${RESET}"
    docker exec ${CLIENT_CONTAINER} /bin/bash -c "ssh yoav@${SERVICE_HOST} hostname"
}

all() {
    init_client
    test_krb5api
    test_gssapi spnego
    test_ssh
}

if [ "$1" == "all" ]; then
    all
    echo -e "${GREEN}=== TESTS SUCCEED${RESET}"
fi

