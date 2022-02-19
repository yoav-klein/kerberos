
# GSS-API Application
---

This is a demonstration of using the GSS-API with combination of Kerberos.
We have a server and client application, in which the client authenticates to the server.

GSS-API provides, additionaly to authentication (which is the most basic service of GSS-API)
also _integrity_ and _confidentiality_.

Whether or not these services are available depends on the underlying mechanism.

## Client
1. Opens a TCP socket with the server
2. Establishes a GSS-API context with the server
3. Wraps a message received from the user
4. Sends the message to the server
5. Receives the integrity tag of the message from the server
6. Verifies the tag.

Steps 5 + 6 are optional, and are used to make sure that the server got the message exactly as we sent it.

### Usage
```
client [-p <port>] [-r] [-d] [-m <mech>] host service [-f] message
```

Arguments:
- `-d` - delegate. not in use currently
- `-m` - security mechanism to use.
- `-r` - record context initialization tokens in files for later inspection.
- `host` - the host domain name e.g. `krb5-server.myexample.com`
- `service` - A Service Principal Name, e.g. `host@krb5-server.myexample.com`
- `message` - to send to the server. 

## Server
1. Opens a TCP socket and waits for connection
2. Establishes a GSS-API context with the client
3. Unwraps the message it received from the client
4. Creates a MIC for the message
5. Sends the MIC to the client

### Usage
```
$ server [-r] [-p <port>] [-m <mech>] service
```

Arguments:
- `-r` - record tokens
- `-m` - currently not in use
- `service` - the name of the service, such as `host`.

## Tokens recording
We added the the ability to record the context initiation tokens so that we can then decode them using the `gssapi-decoder` application. The tokens are saved into files, one file per token.

## Some technical details

### Credentials
For the server, the `gss_acquire_creds` function is the one that accesses the keytab and takes the key of the service name.
For the client, you need to run `kinit` before running.

### Context establishment
The `gss_init_sec_context` and `gss_accept_sec_context` functions are responsible for establishing
a security context between the parties, in which _authentication_ takes place. In this example, 
if you break after `gss_init_sec_context` and look at the packets passed in Wireshark, you'll be able to see the 
TGS-REQ and TGS-REP messages in which the client contacts the KDC to ask for a ticket for the service.

### Service Princiapl Name
When running the `server`, you need to pass in a `service` argument. We pass in just `service` (not `<service>@<host>`), and this string is passed in to `gss_import_name` with the _name type_ `GSS_C_NT_HOSTBASED_SERVICE,` which makes it `<service>@<host>`.
When running the client, we pass in a `host` and `service`. Here, for the `service` we need to pass `<service>@<host>` 
such as `host@krb5-service.myexample.com` - as it is in the KDC.
