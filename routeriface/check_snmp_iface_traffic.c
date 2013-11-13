/* $Id: check_snmp_iface_traffic.c,v 1.2 2005/11/02 21:28:08 gary Exp $
 *
 * check_snmp_iface_traffic.c -- check interface traffic via SNMP
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

#define PROGNAME "check_snmp_iface_traffic"
#define SUMMARY "Check the traffic on an interface using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-i interface] [-s interface string] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *intable = "1.3.6.1.2.1.2.2.1.10";  /* ifInOctets */
char *outtable = "1.3.6.1.2.1.2.2.1.16"; /* ifOutOctets */

int perfdata = 0, ifindex = 0;
char *ifacestr = NULL;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	char theoid[BUFSIZE];
	double in, out;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* ifInOctets */
	snprintf(theoid, BUFSIZE, "%s.%i", intable, ifindex);
	theoid[BUFSIZE-1] = '\0';

	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	in = tree->next->data;
	free(tree);
	
	/* ifOutOctets */
	snprintf(theoid, BUFSIZE, "%s.%i", outtable, ifindex);
	theoid[BUFSIZE-1] = '\0';

	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	out = tree->next->data;
	free(tree);

	printf("%s|Interface %s Traffic| ifInOctets %.0f, ifOutOctets %.0f", host, 
		ifacestr, in, out);

	if (perfdata)
		printf(" | ifinoctets=%.0f ifoutoctets=%.0f", in, out);

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
