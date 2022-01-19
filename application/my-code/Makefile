
SHELL=/bin/bash
GREEN="\e[32;1m"
RESET="\e[0m"


KRB5LIBS=$(shell krb5-config --libs)
CC=gcc
CFLAGS=-g -Wall -Wextra -ansi -pedantic-errors
LDFLAGS=$(KRB5LIBS)

.PHONY: all
all: client server


client: client.c
	@echo -e $(GREEN)"=== Building client ==="$(RESET)
	@$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)
	
server: server.c
	@echo -e $(GREEN)"=== Building server ==="$(RESET)
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	
test: test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 
	
.PHONY: clean
clean:
	@rm test client server 2>/dev/null || echo "hello" > /dev/null
