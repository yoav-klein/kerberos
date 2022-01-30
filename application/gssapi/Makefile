
KRB5LIBS=$(shell krb5-config --libs gssapi)
CC=gcc
CFLAGS=-Wall -Wextra -pedantic-errors -ansi
LDFLAGS=$(KRB5LIBS)

client: client.c
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)
	

clean:
	rm client
