# Krb5-API
---

This demo application is using the Kerberos5 API directly.

## Client
NOTE: must run `kinit` before running!
1. Resolves the server's address
2. Translates service name to principal name
3. Gets the default credential from the credential cache (received with kinit)
4. Sends authentication to the server.

## Server
1. Opens a TCP socket and waits for connection.
2. Translates service name to principal name
3. Gets credentials from keytab
4. Receives authentication from client.