/* $Id: check_snmp_psus.c,v 1.3 2005/11/18 06:11:52 gary Exp $
 *
 * check_snmp_psus.c -- check PSU status via SNMP
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

#define PROGNAME "check_snmp_psus"
#define SUMMARY "Check the PSU health on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community]"

#include "../common/common.h"
#include "../common/common.c"

char *psutable = "1.3.6.1.4.1.232.6.2.9.3.1.4.0"; /* PSU Table */

int warning = 3, critical = 4;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;
	int ret = OK;	

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, psutable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	p = tree->next;

	while (p != NULL) {
		op = p;		

		if (p->data == critical) {
			printf(" PSU %d FAILED", p->index);
			ret = CRITICAL;
		} else if (p->data == warning) {
			printf(" PSU %d DEGRADED", p->index);
			if (ret < CRITICAL)
				ret = WARNING;
		} else 
			printf(" PSU %d OK", p->index);

		p = p->next;
		
		if (p != NULL)
			printf(",");

		free(op);
		op = NULL;
	}

	free(tree);

	printf("\n");
	
	return ret;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:c:w:H:C:")) != -1) {
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
			case '?':
				printf("Usage: " PROGNAME " " OPTIONS "\n");
				exit(UNKNOWN);
		}
	}

	if (host != NULL && community != NULL)
			return 1;
	
	return 0;
}
