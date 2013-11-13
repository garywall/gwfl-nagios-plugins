/* $Id: check_snmp_fans.c,v 1.2 2005/11/02 21:28:08 gary Exp $
 *
 * check_snmp_fans.c -- check fan status via SNMP
 *
 * Copyright (C) Gary Wall & Fredrik Löhr & Fredrik Löhr 2005
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

#define PROGNAME "check_snmp_fans"
#define SUMMARY "Check the fan health on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community]" 

#include "../common/common.h"
#include "../common/common.c"

char *fanstatus =	"1.3.6.1.4.1.232.6.2.6.4.0"; /* Fan status */

int warning = 3, critical = 4;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	int stat = 0;
	
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, fanstatus)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}	 
	
	stat = tree->next->data;
	free(tree);
	
	if (stat == critical) {
		printf("A required fan is not operating properly.\n");
		return CRITICAL;
	} else if (stat == warning) {
		printf("A non-required fan is not operating properly.\n");
		return WARNING;
	}
	
	printf("All fans are operating properly.\n");
	return OK;
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
