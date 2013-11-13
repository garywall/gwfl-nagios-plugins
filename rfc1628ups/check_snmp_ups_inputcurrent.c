/* $Id: check_snmp_ups_inputcurrent.c,v 1.2 2005/11/10 10:07:01 gary Exp $
 *
 * check_snmp_ups_inputcurrent.c -- check Input Currents via SNMP
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

#define PROGNAME "check_snmp_ups_inputcurrent"
#define SUMMARY "Check the Input Currents on a UPS using SNMP."

#define OPTIONS "-H <ip_address> [-C community]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *inputtable = "1.3.6.1.2.1.33.1.3.3.1.4"; /* Input Current Table */

int perfdata = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, inputtable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}
	
	p = tree->next;

	printf("%s|Input Currents| ", host);
	
	while (p != NULL) {
		if (!perfdata)
			op = p;

		printf("Phase %d: %.0f", (p->index + 1), p->data);

		p = p->next;

		if (p != NULL)
			printf(", ");

		if (!perfdata) {
			free(op);
			op = NULL;
		}
	}	
	
	if (perfdata) {
		printf(" |");

		p = tree->next;

		while (p != NULL) {
			op = p;
		
			printf(" phase%d=%.0f", (p->index + 1), p->data);

			p = p->next;
			free(op);
			op = NULL;
		}
	}

	free(tree);

	printf("\n");
	
	return OK;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:H:C:p")) != -1) {		
		switch (c) {
			case 'h':
				print_help();
				exit(UNKNOWN);
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
