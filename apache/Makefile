# Makefile for GWFL Nagios Beta plugins
# $Id: Makefile,v 1.1 2005/11/23 10:22:41 gary Exp $

CC=gcc 
CFLAGS=-Wall -W `net-snmp-config --cflags`
LIBS=`net-snmp-config --libs`
EXTLIBS=`net-snmp-config --external-libs`
DESTDIR=

modules = check_snmp_apache_totalaccess check_snmp_apache_workers 

all: $(modules)

$(modules): ../common/common.c ../common/common.h
$(modules): %: %.c
	$(CC) $(CFLAGS) $(LIBS) $(EXTLIBS) -o $@ $<

.PHONY: install
install:
	if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	install -v $(modules) $(DESTDIR)

.PHONY: clean
clean:
	rm -f $(modules)
