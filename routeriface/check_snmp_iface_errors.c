/* $Id: check_snmp_iface_errors.c,v 1.2 2005/11/02 21:28:08 gary Exp $
 *
 * check_snmp_iface_errors.c -- check interface errors via SNMP
 *
 * Copyright (C) Gary Wall & Fredrik Löhr 2005
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define PROGNAME "check_snmp_iface_errors"
#define SUMMARY "Check errors/discards on an interface using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-i interface] [-s interface string] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *inerrors    = "1.3.6.1.2.1.2.2.1.14"; /* ifInErrors */
char *outerrors   = "1.3.6.1.2.1.2.2.1.20"; /* ifOutErrors */
char *indiscards  = "1.3.6.1.2.1.2.2.1.13"; /* ifInDiscards */
char *outdiscards = "1.3.6.1.2.1.2.2.1.19"; /* ifOutDiscards */

int perfdata = 0, ifindex = 0;
char *ifacestr = NULL;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	double inerr, outerr, indis, outdis;
	char theoid[BUFSIZE];
	
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* ifInErrors */
	snprintf(theoid, BUFSIZE, "%s.%i", inerrors, ifindex);
	theoid[BUFSIZE-1] = '\0';
	
	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	inerr = tree->next->data;
	free(tree);
	
	/* ifOutErrors */
	snprintf(theoid, BUFSIZE, "%s.%i", outerrors, ifindex);
	theoid[BUFSIZE-1] = '\0';

	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	outerr = tree->next->data;
	free(tree);

	/* ifInDiscards */
	snprintf(theoid, BUFSIZE, "%s.%i", indiscards, ifindex);
	theoid[BUFSIZE-1] = '\0';

	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	indis = tree->next->data;
	free(tree);
	
	/* ifOutDiscards */
	snprintf(theoid, BUFSIZE, "%s.%i", outdiscards, ifindex);
	theoid[BUFSIZE-1] = '\0';

	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	outdis = tree->next->data;
	free(tree);

	printf("%s|Interface %s Errors/Discards| ifInErrors %.0f, ifOutErrors %.0f, ifInDiscards %.0f, ifOutDiscards %.0f", host, ifacestr, inerr, outerr, indis, outdis);

	if (perfdata)
		printf(" | ifinerrors=%.0f ifouterrors=%.0f ifindiscards=%.0f ifoutdiscards=%.0f", inerr, outerr, indis, outdis);

	printf("\n");

	return 0;	
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:i:s:H:C:p")) != -1) {
		switch (c) {
			case 'h':
				print_help();
				exit(UNKNOWN);
			case 'H':
				host = optarg;
				break;
			case 'C':
				community = optarg;
				break;
			case 'p':
				perfdata = 1;
				break;
			case 'i':
				ifindex = strtod(optarg, NULL);
				break;	
			case 's':
				ifacestr = optarg;
				break;
			case '?':
				printf("Usage: " PROGNAME " " OPTIONS "\n");
				exit(UNKNOWN);
		}
	}

	if (host != NULL && community != NULL && ifacestr != NULL && ifindex)
		return 1;

	return 0;
}
