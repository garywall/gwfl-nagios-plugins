# Makefile for GWFL Nagios Beta plugins
# $Id: Makefile,v 1.1 2005/11/04 11:10:03 gary Exp $

CC=gcc 
CFLAGS=-Wall -W `net-snmp-config --cflags`
LIBS=`net-snmp-config --libs`
EXTLIBS=`net-snmp-config --external-libs`
DESTDIR=

modules = check_snmp_ups_alarmonbat check_snmp_ups_inputcurrent \
	check_snmp_ups_outputcurrent check_snmp_ups_inputvoltage \
	check_snmp_ups_outputvoltage

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
