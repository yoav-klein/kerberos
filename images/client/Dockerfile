
FROM ubuntu-minimum:0.1

RUN apt-get update
RUN apt-get install -y krb5-user
RUN apt-get install -y openssh-client

COPY ./config_files/krb5/clients/krb5.conf /etc/krb5.conf

RUN sed -i 's/#.*StrictHostKeyChecking.*/StrictHostKeyChecking no/' /etc/ssh/ssh_config

CMD ["tail", "-f", "/dev/null"]
