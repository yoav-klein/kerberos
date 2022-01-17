
KRB5LIBS=$(shell krb5-config --libs)
CC=gcc
CFLAGS=-g -Wall -Wextra -ansi -pedantic-errors
LDFLAGS=$(KRB5LIBS)


client: client.c
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)
	
	
test: test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 
	
.PHONY: clean
clean:
	@rm test client 2>/dev/null || echo "hello" > /dev/null
