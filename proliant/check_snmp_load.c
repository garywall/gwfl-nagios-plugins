/* $Id: check_snmp_load.c,v 1.3 2005/11/18 06:11:52 gary Exp $
 *
 * check_snmp_load.c -- check system load via SNMP
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

#define PROGNAME "check_snmp_load"
#define SUMMARY "Check the load on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_range] [-c crit_range] [-p]\n\nWarning and critical thresholds apply to the 1 minute average returned.\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *loadtable = "1.3.6.1.4.1.2021.10.1.3"; /* Load Averages table */

float warning = 15.0, critical = 30.0;
int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;
	float load[2];

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, loadtable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	p = tree->next;

	while (p != NULL) {
		op = p;

		if (p->index < 3)
			load[p->index] = p->data;

		p = p->next;
		free(op);
		op = NULL;
	}

	free(tree);

	printf(" 1 min: %.2f, 5 min: %.2f, 15 min: %.2f", load[0], load[1], 
		load[2]);

	if (perfdata)
		printf(" | 1min=%.2f 5min=%.2f 15min=%.2f", load[0], load[1], load[2]);

	printf("\n");
	
	if (load[0] >= critical)
		return CRITICAL;
	else if (load[0] >= warning)
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
				community = (u_char *) optarg;
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
