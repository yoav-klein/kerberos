GREEN=\e[32;1m
RESET=\e[0m

CODEC_DIR=codec
CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic-errors -ansi
CFLAGS+=-I$(CODEC_DIR)

LDFLAGS=-L$(CODEC_DIR)
LIBS=-lasncodec
ASN_FILES=gssapi.asn1

$(CODEC_DIR)/libasncodec.a: $(ASN_FILES)
	@echo -e "$(GREEN)=== Compiling codec library $(RESET)"
	@./build-codec.sh

.PHONY: all
all: decoder

decoder: decoder.c $(CODEC_DIR)/libasncodec.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)


spnego-decoder: spnego-decoder.c $(CODEC_DIR)/libasncodec.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	@rm -rf decoder $(CODEC_DIR) *.ber *.der