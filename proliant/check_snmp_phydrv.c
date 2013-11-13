/* $Id: check_snmp_phydrv.c,v 1.4 2006/05/25 09:38:54 gary Exp $
 *
 * check_snmp_phydrv.c -- check physical drive status via SNMP
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

#define PROGNAME "check_snmp_phydrv"
#define SUMMARY "Check the physical drive health on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-b]\n\nSpecify the `-b' arguement if you want the Bus number for each drive shown in the output. If specified, each drive will be shown as bus#:drive# (ie. Drive 0:1 OK).\n"

#include "../common/common.h"
#include "../common/common.c"

char *loctable = "1.3.6.1.4.1.232.3.2.5.1.1.5"; /* Location Table */
char *drvtable = "1.3.6.1.4.1.232.3.2.5.1.1.6"; /* Drive Table */
char *bustable = "1.3.6.1.4.1.232.3.2.5.1.1.50"; /* Drive Bus */

int warning = 4, critical = 3, showbus = 0;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL, *loc = NULL, *q = NULL, *oq = NULL, 
								 *bus = NULL, *b = NULL, *ob = NULL;
	int ret = OK;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, drvtable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	p = tree->next;

	if (!(loc = walkoid(host, community, loctable)) || !test_treeint(loc)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	q = loc->next;

	if (showbus) {
		if (!(bus = walkoid(host, community, bustable)) || !test_treeint(bus)) {
			printf("Error: walkoid() returned nothing\n");
			exit(UNKNOWN);
		}

		b = bus->next;
	}

	while ((p != NULL) && (q != NULL)) {
		op = p;
		oq = q;

		if (showbus && (b != NULL))
			ob = b;

		if ((int) p->data == critical) {
			if (showbus && (b != NULL))
				printf(" Drive %d:%d FAILED", (int) b->data, (int) q->data);
			else
				printf(" Drive %d FAILED", (int) q->data);
			ret = CRITICAL;
		} else if ((int) p->data == warning) {
			if (showbus && (b != NULL))
				printf(" Drive %d:%d PREDICTIVE", (int) b->data, (int) q->data);
			else
				printf(" Drive %d PREDICTIVE", (int) q->data);
			if (ret < CRITICAL)
				ret = WARNING;			
		} else
			if (showbus && (b != NULL))
				printf(" Drive %d:%d OK", (int) b->data, (int) q->data);
		  else
				printf(" Drive %d OK", (int) q->data);
						
		p = p->next;
		q = q->next;

		if (showbus && (b != NULL))
			b = b->next;
		
		if (p != NULL)
			printf(",");

		free(op);
		free(oq);
		op = NULL;
		oq = NULL;

		if (showbus && (b != NULL)) {
			free(ob);
			ob = NULL;
		}
	}

	printf("\n");
	
	return ret;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:c:w:H:C:b")) != -1) {
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
			case 'b':
				showbus = 1;
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
