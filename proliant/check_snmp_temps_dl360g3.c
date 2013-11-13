/* $Id: check_snmp_temps_dl360g3.c,v 1.3 2005/11/03 20:13:57 gary Exp $
 *
 * check_snmp_temps_dl360g3.c -- check system temperatures via SNMP
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

#define PROGNAME "check_snmp_temps_dl360g3"
#define SUMMARY "Check the system temperatures on a DL360 G3 using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-n numcpus] [-p]\n\nIf you want this plugin to return performance data for Nagios, use the `-p` option."

#include "../common/common.h"
#include "../common/common.c"

char *temptable = "1.3.6.1.4.1.232.6.2.6.8.1.4.1"; /* Temperature table */

float thr[] = {56.0, 67.0, 57.0, 67.0};

char *names[] = {
	"Processor zone",
	"CPU 1",
	"I/O zone",
	"CPU 2"
};

int numcpus = 2, perfdata = 0, warning = 10;

int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;
	int ret = OK;

	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}

	if (!(tree = walkoid(host, community, temptable)) || !test_treeint(tree)) {
	  printf("Error: walkoid() returned nothing\n");
    exit(UNKNOWN);
  }
      
  p = tree->next;
	
	printf(" ");

	while (p != NULL) {
		if (!perfdata)
			op = p;

		if ((numcpus > 1) || (strcmp(names[p->index], "CPU 2")) != 0) {
			if (p->index)
				printf(", ");
			if (p->data >= thr[p->index]) {
				printf("%s %.1fC (threshold %.1fC)", names[p->index], p->data,
					thr[p->index]);
				ret = CRITICAL;
			} else if (p->data >= (thr[p->index] - warning)) {
				printf("%s %.1fC (threshold %.1fC)", names[p->index], p->data,
					(thr[p->index] - warning));
				if (ret < CRITICAL)
					ret = WARNING;
			} else
				printf("%s %.1fC", names[p->index], p->data);
		}

		p = p->next;

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
																		
			if (strstr(names[p->index], "Processor zone"))
				printf(" CPUzone=%.1f", p->data);
			else if (strstr(names[p->index], "CPU 1"))
				printf(" CPU1=%.1f", p->data);
			else if (strstr(names[p->index], "I/O zone"))
				printf(" IOzone=%.1f", p->data);
			else if (strstr(names[p->index], "CPU 2") && (numcpus > 1))
				printf(" CPU2=%.1f", p->data);

			p = p->next;
			free(op);
			op = NULL;
		}
	}

	free(tree);
	
	printf("\n");
	
	return ret;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:H:C:n:w:p")) != -1) {
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
			case 'n':
				numcpus = strtod(optarg, NULL);
				break;
			case 'w':
				warning = strtod(optarg, NULL);
				break;
			case 'p':
				perfdata = 1;
				break;
			case '?':
				printf("Usage: " PROGNAME " " OPTIONS "\n");
				exit(UNKNOWN);
		}
	}

	if (host != NULL && community != NULL && numcpus)
		return 1;
	
	return 0;
}
