/* $Id: check_snmp_nics.c,v 1.3 2005/11/18 06:11:52 gary Exp $
 *
 * check_snmp_nics.c -- check NIC status via SNMP
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

#define PROGNAME "check_snmp_nics"
#define SUMMARY "Check NIC status and state on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community]"

#include "../common/common.h"
#include "../common/common.c"

char *nicstate = "1.3.6.1.4.1.232.18.2.3.1.1.13";  /* NIC State table */
char *nicstatus = "1.3.6.1.4.1.232.18.2.3.1.1.14"; /* NIC Status table */

char *statestr[] = {
	"Unknown",
	"Active",
	"Standby",
	"Failed"
};

int warning = 4, critical = 3;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;
	int ret = OK, status[24];

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, nicstatus)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	p = tree->next;

	while (p != NULL) {
		op = p;

		if (p->index < 25)
			status[p->index] = p->data;

		p = p->next;
		free(op);
		op = NULL;
	}

	free(tree);

	if (!(tree = walkoid(host, community, nicstate)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	p = tree->next;

	while (p != NULL) {
		op = p;
		
		if (p->index < 25) {
			if (status[(int) p->data] == critical) {
				printf(" NIC %d General Failure (%s)", p->index, 
					statestr[(int) p->data - 1]);
				ret = CRITICAL;
			} else if (status[(int) p->data] == warning) {
				printf(" NIC %d Link Failure (%s)", p->index, 
					statestr[(int) p->data - 1]);
				if (ret < CRITICAL)
					ret = WARNING;
			} else
				printf(" NIC %d OK (%s)", p->index, 
					statestr[(int) p->data - 1]);
		}

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
			case '?':
				printf("Usage: " PROGNAME " " OPTIONS "\n");
				exit(UNKNOWN);
		}
	}

	if (host != NULL && community != NULL)
			return 1;
	
	return 0;
}
