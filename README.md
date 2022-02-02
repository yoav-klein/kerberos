# Kerberos
---

This repository contains a demo Kerberos cluster, which includes a KDC and a client application that authenticates to a server application using Kerberos tickets.

## Docker Infrastructure
---
The Kerberos cluster is run in Docker containers.
We have one container which is the KDC of the cluster, and two containers which are the server and client.
 
Theses containers are spawn using a `docker-compose.yaml` file in the root of the project.

### Technical notes regarding this
1. A network is created outside of the docker-compose. The name of the network is significant,
since it is appended to the name of the container when doing DNS lookups. So for example if the 
network name is `example.com` and a service name is `server`, then any container in the network can reach
this container using the domain name `server.example.com`.
2. Containers are named so it is easy to perform operations in them in a uniform manner.

## KDC
---
Once we have our containers built and ready, we set up our KDC:
1. Create a `admin` principal (actually not very needed but whatever)
2. Create an account for user named `yoav` - this will be the client
3. Create an account for the service (Service Principal Name) of the form `service/domainname`
4. Create a keytab file for the service account.

Additionally, we copy the keytab file to the `krb5-server` container to `/etc/krb5.keytab` which is the default location
for a keytab.

This is all done by the `init-kdc.sh` script.

## Applications
---
We have 2 demo applications, one using the Kerveros API directly, and one uses the more generic GSS-API.

The krb5 application is a little bit more elegant form of the reference code from the MIT `krb5` repository, also included here.

The server applications are copied to the `krb5-server` container, and the client applications are copied to the `krb5-client` container.


## Usage
---

### Build the docker images
This command will build the required docker images.

```
$ make build-docker
```


### Build the application
This will build the `server` and `client` applications and put them in the `bin` folder.

```
$ make build-application
```

Also, you can run
```
$ make build-all
```

that will do all the above steps.

### Initialize the infrastructure
This will initialize the KDC with the required principals, create a keytab and copy it to the `krb5-server` container,
and also copy the client and server applications to the containers.

```
$ make init
```

### Run the test
Now that everything is set up, run the `server` and `client` applications to test the Kerberos authentication:

```
$ make test
```
 

