/* $Id: check_snmp_ns_mem_used.c,v 1.2 2005/11/02 21:28:07 gary Exp $
 * 
 * check_snmp_ns_mem_used.c -- check used memory on a Netscreen via SNMP
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

#define PROGNAME "check_snmp_ns_mem_used"
#define SUMMARY "Check the used memory on a Netscreen using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_percent] [-c crit_percent] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *memused = "1.3.6.1.4.1.3224.16.2.1.0"; /* Used */
char *memfree = "1.3.6.1.4.1.3224.16.2.2.0"; /* Free*/

float warning = 80.0, critical = 90.0;;
int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	double used, mfree, total;
	float pc;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* Used */
	if (!(tree = walkoid(host, community, memused)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	used = tree->next->data;
	free(tree);

	/* Free */
	if (!(tree = walkoid(host, community, memfree)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	mfree = tree->next->data;
	free(tree);
	
	total = (used + mfree);
	pc = (used / (total / 100));

	if ((used / 1073741824) >= 1)
		printf(" %.2fGB of ", (used / 1073741824));
	else
		printf(" %.2fMB of ", (used / 1048576));

	if ((total / 1073741824) >= 1)
		printf("%.2fGB ", (total / 1073741824));
	else
		printf("%.2fMB ", (total / 1048576));

	printf("(%.1f%%) used", pc);
	
	if (perfdata)
		printf(" | usedpercent=%.1f", pc);

	printf("\n");
	
	if (pc >= critical)
		return CRITICAL;
	else if (pc >= warning)
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
