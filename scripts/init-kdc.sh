#!/bin/bash


####
# 
#	Initialize the KDC with the appropriate users
#
####

server_container="krb5-server"
kdc_container="krb5-kdc"
spn="host/krb5-server.myexample.com"

if ! [ "$(docker container ls -q -f name=${kdc_container})" ] || \
! [ "$(docker container ls -q -f name=${server_container})" ]; then
	echo -e "\e[31mKDC container or/and server container is down, exiting"
	exit 1
fi

echo -e "\e[32;1m=== Initializing KDC\e[0m"

docker exec $kdc_container /bin/bash -c "
kadmin.local <<- EOF
	add_principal -pw admin \"admin/admin\"
	add_principal -pw yoav \"yoav\"
	add_principal -pw service \"${spn}\"
	ktadd ${spn}
EOF
"

echo -e "\e[32;1m=== Copying keytab to server\e[0m"
docker cp $kdc_container:/etc/krb5.keytab .
docker cp krb5.keytab $server_container:/etc/krb5.keytab
rm krb5.keytab
