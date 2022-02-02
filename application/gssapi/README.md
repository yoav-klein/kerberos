
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

## Server
1. Opens a TCP socket and waits for connection
2. Establishes a GSS-API context with the client
3. Unwraps the message it received from the client
4. Creates a MIC for the message
5. Sends the MIC to the client
