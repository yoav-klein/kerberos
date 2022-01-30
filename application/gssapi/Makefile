
KRB5LIBS=$(shell krb5-config --libs gssapi)
CC=gcc
CFLAGS=-Wall -Wextra -pedantic-errors -ansi
LDFLAGS=$(KRB5LIBS)

server: server.c utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: client.c utils.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)
	
utils.o: utils.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm client
