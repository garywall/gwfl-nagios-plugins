/* $Id: check_snmp_apache_workers.c,v 1.1 2005/11/23 10:22:41 gary Exp $
 *
 * check_snmp_apache_workers.c -- check Apache busy/idle workers via SNMP
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

#define PROGNAME "check_snmp_apache_workers"
#define SUMMARY "Check busy/idle workers on server(s) via SNMP."

#define OPTIONS "-H <ip_address1[,ipaddress2...]> [-C community] [-e <entity_name>] [-w warn_percent_busy] [-c crit_percent_busy]"

#include "../common/common.h"
#include "../common/common.c"

char *busyworkers = "1.3.6.1.4.1.19786.1.1.2.3.0";
char *idleworkers = "1.3.6.1.4.1.19786.1.1.2.4.0";

int warning = 70, critical = 90, perfdata = 0;
char *entity = NULL;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	char *tmpstr = NULL, *tmp1 = NULL;
	float totalworkers = 0, totalbusy = 0, totalidle = 0, pcbusy;
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

		if (!(tree = walkoid(tmpstr, community, busyworkers)) || 
				!test_treeint(tree)) {
			printf("Error: walkoid() returned nothing from %s\n", tmpstr);
			exit(UNKNOWN);
		}

		totalbusy += tree->next->data;
		free(tree);

		if (!(tree = walkoid(tmpstr, community, idleworkers)) ||
				!test_treeint(tree)) {
			printf("Error: walkoid() returned nothing from %s\n", tmpstr);
			exit(UNKNOWN);
		}

		totalidle += tree->next->data;
		free(tree);

		c++;
		
		if (running)
			tmpstr = tmp1 + 1;
	}

	totalworkers = totalbusy + totalidle;
	pcbusy = (totalbusy / (totalworkers / 100));

	printf(" %.0f of %.0f workers on %d host(s) busy, %.0f idle (%.1f%% utilised)", 
		totalbusy, totalworkers, c, totalidle, pcbusy);

	if (perfdata)
		printf(" | percentbusy=%.1f", pcbusy);

	printf("\n");

	if (pcbusy >= critical)
		return CRITICAL;
	else if (pcbusy >= warning)
		return WARNING; 

	return OK;
}

int parse_args(int argc, char **argv)
{
  int c;
  opterr = 0;
  
  while ((c = getopt(argc, argv, "hh:H:C:e:w:c:p")) != -1) {
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
