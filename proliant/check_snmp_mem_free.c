/* $Id: check_snmp_mem_free.c,v 1.4 2006/05/25 08:48:51 gary Exp $
 * 
 * check_snmp_mem_free.c -- check available memory via SNMP
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

#define PROGNAME "check_snmp_mem_free"
#define SUMMARY "Check the memory free on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-w warn_percent] [-c crit_percent] [-p] [-n]\n\nBy default, the available memory is calculated as (free + cached + buffers). This can be disabled using the `-n' option.\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *sizetable = "1.3.6.1.4.1.2021.4.5.0"; /* Size table */
char *freetable = "1.3.6.1.4.1.2021.4.6.0"; /* Free table */
char *cachetable = "1.3.6.1.4.1.2021.4.15.0"; /* Cached table */
char *buffertable = "1.3.6.1.4.1.2021.4.14.0"; /* Buffers table */

float warning = 20.0, critical = 10.0;
int perfdata = 0, cachecalc = 1;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	double size = 0, mfree = 0, cached = 0, buffers = 0, avail;
	float pc;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	/* Size */
	if (!(tree = walkoid(host, community, sizetable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	size = tree->next->data;
	free(tree);

	/* Free */
	if (!(tree = walkoid(host, community, freetable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	mfree = tree->next->data;
	free(tree);

	/* Cached */
	if (!(tree = walkoid(host, community, cachetable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	cached = tree->next->data;
	free(tree);

	/* Buffers */
	if (!(tree = walkoid(host, community, buffertable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	buffers = tree->next->data;
	free(tree);
	
	avail = mfree;

	if (cachecalc)
		avail += (cached + buffers);
	
	pc = (avail / (size / 100));

	if ((avail / 1073741824) >= 1)
		printf(" %.2fTB of ", (avail / 1073741824));
	else if ((avail / 1048576) >= 1)
		printf(" %.2fGB of ", (avail / 1048576));
	else
		printf(" %.2fMB of ", (avail / 1024));

	if ((size / 1073741824) >= 1)
		printf("%.2fTB ", (size / 1073741824));
	else if ((size / 1048576) >= 1)
		printf("%.2fGB ", (size / 1048576));
	else
		printf("%.2fMB ", (size / 1024));

	printf("(%.1f%%) free", pc);

	if (perfdata)
		printf(" | freepercent=%.1f", pc);

	printf("\n");
	
	if (pc <= critical)
		return CRITICAL;
	else if (pc <= warning)
		return WARNING;
	
	return OK;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:c:w:H:C:pn")) != -1) {
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
			case 'n':
				cachecalc = 0;
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
