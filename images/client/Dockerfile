
FROM ubuntu-minimum:0.1

RUN apt-get update
RUN apt-get install -y krb5-user

COPY ./config_files/krb5/clients/krb5.conf /etc/krb5.conf

CMD ["tail", "-f", "/dev/null"]
