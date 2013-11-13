# Makefile for GWFL Nagios eSensors EM01 plugin
# $Id: Makefile,v 1.1 2005/10/09 19:37:57 gary Exp $

CC=gcc 
CFLAGS=-Wall -W
LIBS=-lcurl
EXTLIBS=
DESTDIR=

modules = check_http_em01 

all: $(modules)

$(modules): %: %.c
	$(CC) $(CFLAGS) $(LIBS) $(EXTLIBS) -o $@ $<

.PHONY: install
install:
	if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	install -v $(modules) $(DESTDIR)

.PHONY: clean
clean:
	rm -f $(modules)
