# Makefile for GWFL Nagios plugins
# $Id: Makefile,v 1.13 2005/11/23 10:44:13 gary Exp $

DESTDIR=

all: proliant ciscocat ciscocss netscreen routeriface em01 beta \
	rfc1628ups apache

.PHONY: proliant proliant-install proliant-clean
proliant:
	@cd proliant && $(MAKE)

proliant-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd proliant && $(MAKE) install DESTDIR=$(DESTDIR)

proliant-clean:
	@cd proliant && $(MAKE) clean

.PHONY: ciscocat ciscocat-install ciscocat-clean
ciscocat:
	@cd ciscocat && $(MAKE)

ciscocat-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd ciscocat && $(MAKE) install DESTDIR=$(DESTDIR)

ciscocat-clean:
	@cd ciscocat && $(MAKE) clean

.PHONY: ciscocss ciscocss-install ciscocss-clean
ciscocss:
	@cd ciscocss && $(MAKE)

ciscocss-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd ciscocss && $(MAKE) install DESTDIR=$(DESTDIR)
	
ciscocss-clean:
	@cd ciscocss && $(MAKE) clean

.PHONY: em01 em01-install em01-clean
em01:
	@cd em01 && $(MAKE)

em01-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd em01 && $(MAKE) install DESTDIR=$(DESTDIR)

em01-clean:
	@cd em01 && $(MAKE) clean

.PHONY: netscreen netscreen-install netscreen-clean
netscreen:
	@cd netscreen && $(MAKE)

netscreen-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd netscreen && $(MAKE) install DESTDIR=$(DESTDIR)

netscreen-clean:
	@cd netscreen && $(MAKE) clean

.PHONY: routeriface routeriface-install routeriface-clean
routeriface:
	@cd routeriface && $(MAKE)

routeriface-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd routeriface && $(MAKE) install DESTDIR=$(DESTDIR)

routeriface-clean:
	@cd routeriface && $(MAKE) clean

.PHONY: beta beta-install beta-clean
beta:
	@cd beta && $(MAKE)

beta-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd beta && $(MAKE) install DESTDIR=$(DESTDIR)

beta-clean:
	@cd beta && $(MAKE) clean

.PHONY: rfc1628ups rfc1628ups-install rfc1628ups-clean
rfc1628ups:
	@cd rfc1628ups && $(MAKE)

rfc1628ups-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd rfc1628ups && $(MAKE) install DESTDIR=$(DESTDIR)

rfc1628ups-clean:
	@cd rfc1628ups && $(MAKE) clean

.PHONY: apache apache-install apache-clean
apache:
	@cd apache && $(MAKE)

apache-install:
	@if ! test -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	@cd rfc1628ups && $(MAKE) install DESTDIR=$(DESTDIR)

apache-clean:
	@cd apache && $(MAKE) clean

.PHONY: install
install: proliant-install ciscocat-install ciscocss-install \
	netscreen-install routeriface-install em01-install beta-install \
	rfc1628ups-install apache-install

.PHONY: clean
clean: proliant-clean ciscocat-clean ciscocss-clean \
	netscreen-clean routeriface-clean em01-clean beta-clean \
	rfc1628ups-clean apache-clean
