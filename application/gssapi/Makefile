

SHELL=/bin/bash
GREEN="\e[32;1m"
RESET="\e[0m"

KRB5LIBS=$(shell krb5-config --libs gssapi)
CC=gcc
CFLAGS=-Wall -Wextra -pedantic-errors -ansi
LDFLAGS=$(KRB5LIBS)

.PHONY: all
all: server client

server: server.c utils.o
	@echo -e $(GREEN)"=== Building server ==="$(RESET)
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: client.c utils.o
	@echo -e $(GREEN)"=== Building client ==="$(RESET)
	@$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)

test: test.c utils.o
	@echo -e $(GREEN)"=== Building test ==="$(RESET)
	@$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)

utils.o: utils.c
	@$(CC) -c $(CFLAGS) -o $@ $^

clean:
	@rm client server *.o 2>/dev/null | echo "hello" > /dev/null
