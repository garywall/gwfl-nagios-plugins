/* $Id: check_snmp_ns_cpu.c,v 1.2 2005/11/02 21:28:07 gary Exp $
 *
 * check_snmp_ns_cpu.c -- check Netscreen CPU utilisation via SNMP
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

#define PROGNAME "check_snmp_ns_cpu"
#define SUMMARY "Check the CPU utilisation on a Netscreen using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_range] [-c crit_range] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *cputable = "1.3.6.1.4.1.3224.16.1"; /* Netscreen CPU table:
																						 we skip .1.0 because to me
																						 the metric given from that
																						 oid does'nt make sense */

float warning = 80.0, critical = 95.0;
int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;
	int ret = OK, i = 1;
	
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, cputable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n"); 
		exit(UNKNOWN);
	}

	/* Waste our time malloc'ing for one struct instead of running init_mnt()
	 * twice more because of the metric .1.0 gives back? I think so.. */
	op = tree;
	tree = tree->next;
	free(op);
	op = NULL;
	p = tree->next;

	printf(" ");

	while (p != NULL) {
		if (!perfdata)
			op = p;
	
		printf("%d min: %.0f%%", (int) i, p->data);
		
		if (p->data >= critical) 
			ret = CRITICAL;
		else if (p->data >= warning)
			if (ret < CRITICAL)
				ret = WARNING;
		
		p = p->next;

		if (p != NULL)
			printf(", ");

		if (!perfdata) {
			free(op);
			op = NULL;
		}
		
		if (i == 1)
			i = 5;
		else
			i = 15;
	}
			
	if (perfdata) {
		printf(" |");

		i = 1;
		p = tree->next;

		while (p != NULL) {
			op = p;
					
			printf(" %dmin=%.0f", (int) i, p->data);

			p = p->next;
			free(op);
			op = NULL;

			if (i == 1)
      	i = 5;
    	else
      	i = 15;
		}
	}

	free(tree);
			
	printf("\n");
	
	return ret;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:c:w:H:C:p")) != -1) {		
		switch (c) {
			case 'h':
				print_help();
				exit(UNKNOWN);
			case 'c':
				critical = strtod(optarg, NULL);
				break;
			case 'w':
				warning = strtod(optarg, NULL);
				break;
			case 'H':
				host = optarg;
				break;
			case 'C':
				community = optarg;
				break;
			case 'p':
				perfdata = 1;
				break;
			case '?':
				printf("Usage: " PROGNAME " " OPTIONS "\n");
				exit(UNKNOWN);
		}
	}

	if (host != NULL && community != NULL)
		return 1;
	
	return 0;
}
