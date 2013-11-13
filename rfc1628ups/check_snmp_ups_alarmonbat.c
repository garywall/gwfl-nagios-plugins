/* $Id: check_snmp_ups_alarmonbat.c,v 1.1 2005/11/04 11:10:03 gary Exp $
 *
 * check_snmp_ups_alarmonbat.c -- check Alarm on Battery via SNMP
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

#define PROGNAME "check_snmp_ups_alarmonbat"
#define SUMMARY "Check Alarm on Battery status on a UPS using SNMP."
#define OPTIONS "-H <ip_address> [-C community]"

#include "../common/common.h"
#include "../common/common.c"

char *alarmonbat = "1.3.6.1.2.1.33.1.6.3.2.0"; /* Alarm on Battery OID */

int warning = 1;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL;
	int stat, ret = OK;

  if (!parse_args(argc, argv)) {
    printf("Usage: " PROGNAME " " OPTIONS "\n");
    exit(UNKNOWN);
  }
  
	if (!(tree = walkoid(host, community, alarmonbat)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	stat = tree->next->data;
	free(tree);
	
	if (stat) {
		printf(" UPS is currently drawing power from battery.\n");
		ret = WARNING;
	} else
		printf(" UPS is not currently drawing power from battery.\n");

	return ret;
}

int parse_args(int argc, char **argv)
{
  int c;
  opterr = 0;

  while ((c = getopt(argc, argv, "hh:H:C:")) != -1) {
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
      case '?':
        printf("Usage: " PROGNAME " " OPTIONS "\n");
        exit(UNKNOWN);
    }
  }

  if (host != NULL && community != NULL)
    return 1;

  return 0;
}


