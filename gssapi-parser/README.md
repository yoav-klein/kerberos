# GSS-API Parser
---

This is a program to parse GSS-API tokens. This include the GSS-API tokens, and SPNEGO tokens.
It uses the `gssapi.asn1` file as the ASN1 speicification, which is assembled from RFCs 2743 (GSS-API) and 4178 (SPNEGO).

It uses the `asn1c` ASN1 compiler to compile a codec library, which is used by our `decoder.c` program.

## Usage
---
First, you need to have the `asn1c` docker image, you can build and run it using our GitHub project: https://github.com/yoav-klein/asn1.git

Then, inside the docker container, run 
```
$ make decoder
```
and copy out the `decoder` program.

Now you need to have some tokens, which you can take from the `gssapi` client and server application.


### Running
You run the decoder as such
```
$ decoder <file> gssapi/spnego
```

The first token sent by the client is always a GSSAPI token.
If using SPNEGO, the GSSAPI token encapsulates a SPNEGO token. All subsequent tokens are mechanism-specific.
In the case of SPNEGO, that means another SPNEGO token. So if you want to parse the subsequent token, you need to run `decoder <file> spnego`, but see the known issue below...

## Known issue
For some reason, reading the `negTokenResp` doesn't work: it's something with the encoding of the ENUMARTED type.