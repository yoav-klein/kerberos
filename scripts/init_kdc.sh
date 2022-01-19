#!/bin/bash


####
# 
#	Initialize the KDC with the appropriate users
#
####

server_container="krb5-server"
kdc_container="krb5-kdc"
spn="host/krb5-server.myexample.com"

docker exec $kdc_container /bin/bash -c "
kadmin.local <<- EOF
	add_principal -pw admin \"admin/admin\"
	add_principal -pw yoav \"yoav\"
	add_principal -pw service \"${spn}\"
	ktadd ${spn}
EOF
"

docker cp $kdc_container:/etc/krb5.keytab .
docker cp krb5.keytab $server_container:/etc/krb5.keytab
rm krb5.keytab
