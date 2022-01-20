#!/bin/bash

######
#	
#	Run the server and client application for test	
#
####

GREEN="\e[32;1m"
RESET="\e[0m"

echo -e "${GREEN}Running server in server container${RESET}"
docker exec  krb5-server /bin/bash -c "
/server -h krb5-server.myexample.com -p 8081 -s host
" &

docker exec krb5-client /bin/bash -c "
kinit yoav << EOF
yoav
EOF

/client -h krb5-server.myexample.com -p 8081
"
