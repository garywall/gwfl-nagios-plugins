/* $Id: check_snmp_cs_cpu.c,v 1.2 2005/11/02 21:28:07 gary Exp $
 *
 * check_snmp_cs_cpu.c -- check Cisco CPU utilisation via SNMP
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

#define PROGNAME "check_snmp_cs_cpu"
#define SUMMARY "Check the CPU utilisation on a Cisco using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_range] [-c crit_range] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *cpu1min = "1.3.6.1.4.1.9.2.1.57.0"; /* 1 min */
char *cpu5min = "1.3.6.1.4.1.9.2.1.58.0"; /* 5 min */

float warning = 80.0, critical = 95.0;
int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	float util1, util5;
	
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* 1 min */
	if (!(tree = walkoid(host, community, cpu1min)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	util1 = tree->next->data;
	free(tree);

	/* 5 min */
	if (!(tree = walkoid(host, community, cpu5min)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	util5 = tree->next->data;
	free(tree);
	
	printf(" 1 min: %.0f%%, 5 min: %.0f%%", util1, util5);

	if (perfdata)
		printf(" | 1min=%.0f 5min=%.0f", util1, util5);

	printf("\n");
	
	if (util1 >= critical)
		return CRITICAL;
	else if (util1 >= warning)
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
