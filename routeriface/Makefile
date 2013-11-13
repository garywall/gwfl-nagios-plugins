# Makefile for GWFL Nagios Router Interface plugins
# $Id: Makefile,v 1.1 2005/10/09 19:37:57 gary Exp $

CC=gcc 
CFLAGS=-Wall -W `net-snmp-config --cflags`
LIBS=`net-snmp-config --libs`
EXTLIBS=`net-snmp-config --external-libs`
DESTDIR=

modules = check_snmp_iface_traffic check_snmp_iface_errors 

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
