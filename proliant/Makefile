# Makefile for GWFL Nagios ProLiant plugins
# $Id: Makefile,v 1.3 2006/05/25 10:02:50 gary Exp $

CC=gcc 
CFLAGS=-Wall -W `net-snmp-config --cflags`
LIBS=`net-snmp-config --libs`
EXTLIBS=`net-snmp-config --external-libs`
DESTDIR=

modules = check_snmp_cpu check_snmp_disk_free check_snmp_disk_used \
	check_snmp_fans check_snmp_load check_snmp_mem_free \
	check_snmp_mem_used check_snmp_nics check_snmp_phydrv \
	check_snmp_psus check_snmp_swap_free check_snmp_swap_used \
	check_snmp_temps_dl360g3 check_snmp_temps_dl360g4 \
	check_snmp_temps_dl380g4 check_snmp_temps_dl385g1 \
	check_snmp_totalprocs check_snmp_temps_dl320g3 \
	check_snmp_temps_dl380g3 check_snmp_temps_ml370g4

all: $(modules)

$(modules): ../common/common.c ../common/common.h
$(modules): %: %.c
	$(CC) $(CFLAGS) $(LIBS) $(EXTLIBS) -o $@ $<

.PHONY: install
install:
	if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	install -v $(modules) "$(TOPDIR)/$(DESTDIR)"

.PHONY: clean
clean:
	rm -f $(modules)
