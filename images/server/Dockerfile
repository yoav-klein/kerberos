
FROM ubuntu-minimum:0.1

RUN apt-get update
RUN apt-get install -y krb5-user
RUN apt-get install -y openssh-server

RUN mkdir /run/sshd
RUN sed -i 's/#GSSAPIAuthentication.*/GSSAPIAuthentication yes/' /etc/ssh/sshd_config
RUN sed -i 's/#GSSAPIStrictAcceptorCheck.*/GSSAPIStrictAcceptorCheck no/' /etc/ssh/sshd_config

# for SSH authentication
RUN useradd -m yoav 

COPY ./config_files/krb5/clients/krb5.conf /etc/krb5.conf

CMD ["tail", "-f", "/dev/null"]
