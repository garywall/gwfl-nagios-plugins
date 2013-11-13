/* $Id: check_snmp_apache_totalaccess.c,v 1.1 2005/11/23 10:22:41 gary Exp $
 *
 * check_snmp_apache_totalaccess.c -- check Apache total accesses via SNMP
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

#define PROGNAME "check_snmp_apache_totalaccess"
#define SUMMARY "Check total accesses on server(s) via SNMP."

#define OPTIONS "-H <ip_address1[,ipaddress2...]> [-C community] [-e <entity_name>]"

#include "../common/common.h"
#include "../common/common.c"

char *totalaccess = "1.3.6.1.4.1.19786.1.1.2.2.0";

int perfdata = 0;
char *entity = NULL;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	char *tmpstr = NULL, *tmp1 = NULL;
	double total = 0;
	int running = 1, c = 0;
	
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tmpstr = strdup(host))) {
		printf("Could not allocate memory to duplicate host!\n");
		exit(UNKNOWN);
	}

	if (!(tmpstr = strdup(host))) {
		printf("Could not allocate memory to duplicate host!\n");
		exit(UNKNOWN);
	}

	while(running) {
		if (!(tmp1 = strstr(tmpstr, ",")))
			running = 0;
		else
			*tmp1 = '\0';

		if (!(tree = walkoid(tmpstr, community, totalaccess)) || 
				!test_treeint(tree)) {
			printf("Error: walkoid() returned nothing from %s\n", tmpstr);
			exit(UNKNOWN);
		}

		total += tree->next->data;
		free(tree);
		c++;
		
		if (running)
			tmpstr = tmp1 + 1;
	}

	printf("%s|Apache Total Accesses| TotalAccesses %.0f", entity ? entity : host, 		total);

	if (perfdata)
		printf(" | totalaccesses=%.0f", total);

	printf("\n");

	return OK;
}

int parse_args(int argc, char **argv)
{
  int c;
  opterr = 0;
  
  while ((c = getopt(argc, argv, "hh:H:C:e:p")) != -1) {
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
			case 'e':
				entity = optarg;
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
