/* $Id: check_snmp_css_cpu.c,v 1.2 2005/11/02 21:28:07 gary Exp $
 *
 * check_snmp_css_cpu.c -- check Cisco CSS CPU utilisation via SNMP
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

#define PROGNAME "check_snmp_css_cpu"
#define SUMMARY "Check the CPU utilisation on a Cisco CSS using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_range] [-c crit_range] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *currentcpu = "1.3.6.1.4.1.2467.1.34.17.1.13.1.1"; /* Current CPU */
char *averagecpu = "1.3.6.1.4.1.2467.1.34.17.1.14.1.1"; /* Average CPU */

float warning = 80.0, critical = 95.0;
int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	float cur, avg;
				
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* Current CPU */
	if (!(tree = walkoid(host, community, currentcpu)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	cur = tree->next->data;
	free(tree);

	/* Average CPU */
	if (!(tree = walkoid(host, community, averagecpu)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	avg = tree->next->data;
	free(tree);

	printf(" Current: %.0f%%, Average: %.0f%%", cur, avg);

	if (perfdata)
		printf(" | current=%.0f average=%.0f", cur, avg);

	printf("\n");
	
	if (cur >= critical)
		return CRITICAL;
	else if (cur >= warning)
		return WARNING;
	
	return OK;
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
