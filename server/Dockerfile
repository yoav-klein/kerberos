

####################################################
#
#	Kerberos KDC and admin server
#	
#	This Dockerfile creates an image with a Kerberos KDC and admin server
#	
#	Realm: MYEXAMPLE.COM
#	domain: server.myexample.com
#
#
###############################################

FROM ubuntu:20.04

RUN apt-get update && apt-get install -y krb5-kdc krb5-admin-server

COPY ./krb5.conf /etc/krb5.conf
COPY ./kadm5.acl etc/krb5kdc/kadm5.acl

RUN printf "\
[logging]\n \
	default = FILE:/var/log/kerberos/krb5.log\n \  
	admin_server = FILE:/var/log/kerberos/kadmin.log\n \
	kdc = FILE:/var/log/kerberos/krb5lib.log\n" | tee -a /etc/krb5kdc/kdc.conf

RUN mkdir -p /var/log/kerberos
RUN touch /var/log/kerberos/krb5.log
RUN touch /var/log/kerberos/kadmin.log
RUN touch /var/log/kerberos/krb5lib.log

RUN kdb5_util create -r "MYEXAMPLE.COM" -P yoavklein3 -s
RUN kadmin.local add_principal -pw "admin" admin/admin

COPY ./start.sh /start.sh
ENTRYPOINT ["/start.sh"]

CMD ["tail", "-f", "/dev/null"]
