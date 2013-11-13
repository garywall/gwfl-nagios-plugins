/* $Id: check_snmp_disk_free.c,v 1.3 2005/11/18 06:11:52 gary Exp $
 *
 * check_snmp_disk_free.c -- check partition disk space free via SNMP
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

#define PROGNAME "check_snmp_disk_free"
#define SUMMARY "Check partition disk space free on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_percent] [-c crit_percent] [-i partition_index] [-r reserved_block_percent]\n\nTo find the partition index for a filesystem, snmpwalk '1.3.6.1.2.1.25.2.3.1.3' on the target host and use the last digit of the OID corresponding to the filesystem as the index.\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *alloctable = "1.3.6.1.2.1.25.2.3.1.4"; /* Allocation Units Table */
char *totaltable = "1.3.6.1.2.1.25.2.3.1.5"; /* Total Size Table */
char *usedtable = "1.3.6.1.2.1.25.2.3.1.6";  /* Used Space Table */

float warning = 20.0, critical = 10.0;
int pindex = 0, perfdata = 0;
int reserved = 5; /* default usually on linux */

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	double units = 0, total = 0, used = 0, realfree;
	char theoid[BUFSIZE];
	float util;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* Allocation units */
	snprintf(theoid, BUFSIZE, "%s.%i", alloctable, pindex);
	theoid[BUFSIZE-1] = '\0';
	
	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	units = tree->next->data;
	free(tree);
	
	/* Total Size */
	snprintf(theoid, BUFSIZE, "%s.%i", totaltable, pindex);
	theoid[BUFSIZE-1] = '\0';
	
	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	total = tree->next->data;
	free(tree);

	/* Used Space */
	snprintf(theoid, BUFSIZE, "%s.%i", usedtable, pindex);
	
	if (!(tree = walkoid(host, community, theoid)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	used = tree->next->data;
	free(tree);
	
	realfree = ((total * units) - (used * units) - (((total * units) / 100) * reserved));
	util = (realfree / ((total * units) / 100));
	
	if ((realfree / TB) >= 1)
		printf(" %.2fTB of ", (double) (realfree / TB));
	else if ((realfree / GB) >= 1)
		printf(" %.2fGB of ", (double) (realfree / GB));
	else
		printf(" %.2fMB of ", (realfree / 1048576));

	if (((total * units) / TB) >= 1)
		printf("%.2fTB ", (double) (total * units / TB));
	else if (((total * units) / GB) >= 1)
		printf("%.2fGB ", (double) (total * units / GB));
	else
		printf("%.2fMB ", (total * units / 1048576));

	printf("(%.1f%%) free, ", util);

	if (((((total * units) / TB) / 100) * reserved) >= 1)
		printf("%.2fTB (%i%%) reserved",
			(double) ((((total * units) / TB) / 100) * reserved), reserved);
	else if (((((total * units) / GB) / 100) * reserved) >= 1)
		printf("%.2fGB (%i%%) reserved", 
			(double) ((((total * units) / GB) / 100) * reserved), reserved);
	else
		printf("%.2fMB (%i%%) reserved",
			((((total * units) / 1048576) / 100) * reserved), reserved);

	if (perfdata)
		printf(" | freepercent=%.1f reserved=%i", util, reserved);

	printf("\n");
	
	if (util <= critical)
		return CRITICAL;
	else if (util <= warning) 
		return WARNING;
	
	return OK;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:c:w:H:C:i:r:p")) != -1) {
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
			case 'i':
				pindex = strtod(optarg, NULL);
				break;
			case 'r':
				reserved = strtod(optarg, NULL);
				break;
			case 'p':
				perfdata = 1;
				break;
			case '?':
				printf("Usage: " PROGNAME " " OPTIONS "\n");
				exit(UNKNOWN);
		}
	}
	
	if (host != NULL && community != NULL && pindex > 0)
		return 1;

	return 0;
}
