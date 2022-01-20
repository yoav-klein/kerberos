# Kerberos
---

This repository contains a demo Kerberos cluster, which includes a KDC and a client application
that authenticates to a server application using Kerberos tickets.

## Docker Infrastructure
---
The Kerberos cluster is run in Docker containers.
We have one container which is the KDC of the cluster, and two containers which are the 
server and client.
 
Theses containers are spawn using a `docker-compose.yaml` file in the root of the project.

### Technical notes regarding this
1. A network is created outside of the docker-compose. The name of the network is significant,
since it is appended to the name of the container when doing DNS lookups. So for example if the 
network name is `example.com` and a service name is `server`, then any container in the network can reach
this container using the domain name `server.example.com`.
2. Containers are named so it is easy to perform operations in them in a uniform manner.

## KDC
---
Once we have our containers built and ready, we set up our KDC

## Application
---
Once we have
