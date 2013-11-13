/* $Id: check_snmp_totalprocs.c,v 1.2 2005/11/02 21:28:08 gary Exp $
 *
 * check_snmp_totalprocs.c -- check total processes via SNMP
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

#define PROGNAME "check_snmp_totalprocs"
#define SUMMARY "Check the total number of processes on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_range] [-c crit_range] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *totalprocs = "1.3.6.1.2.1.25.1.6.0"; /* Total Processes */

unsigned int warning = 400, critical = 600;
int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	unsigned long numprocs;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, totalprocs)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	numprocs = tree->next->data;
	free(tree);

	printf(" %lu processes running", numprocs);

	if (perfdata)
		printf(" | numprocesses=%lu", numprocs);

	printf("\n");
	
	if (numprocs >= critical)
		return CRITICAL;
	else if (numprocs >= warning)
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
				critical = strtoul(optarg, NULL, 10);
				break;
			case 'w':
				warning = strtoul(optarg, NULL, 10);
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
