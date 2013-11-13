/* $Id: check_snmp_phydrv_ffs.c,v 1.12 2005/11/10 10:41:18 gary Exp $
 *
 * check_snmp_phydrv_ffs.c -- check physical drive status via SNMP
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

#define PROGNAME "check_snmp_phydrv_ffs"
#define SUMMARY "Check the physical drive health on a host using SNMP."

#define OPTIONS "-H <ip_address> [-C community] [-n numdrives]"

#include "../common/common.h"
#include "../common/common.c"

char *drvtable = "1.3.6.1.4.1.232.3.2.5.1.1.6"; /* Drive Table */

int warning = 4, critical = 3, ok = 2;

#define DRV_OK 2
#define DRV_WARN 4
#define DRV_CRIT 3


int main(int argc, char **argv)
{
	struct treeint *tree = NULL, *p = NULL, *op = NULL;
	int i = 0, ret = OK;
	int last = -1, top = -1;

	struct treeint *drvok;
	struct treeint *drvwarn;
	struct treeint *drvcrit;
	
	if (!parse_args(argc, argv)) {
		printf("Usage: " PROGNAME " " OPTIONS "\n");
		exit(UNKNOWN);
	}
	
	if (!(tree = walkoid(host, community, drvtable)) || !test_treeint(tree)) {
		printf("Error: walkoid() returned nothing\n");
		exit(UNKNOWN);
	}

	drvok = init_treeint();
	drvwarn = init_treeint();
	drvcrit = init_treeint();

	p = tree->next;

	while (p != NULL) {
		op = p;

		switch ((int) p->data) {
			case DRV_CRIT:
				push_treeint(drvcrit, p->index, p->data);
				break;
			case DRV_WARN:
				push_treeint(drvwarn, p->index, p->data);
				break;
			default:
				push_treeint(drvok, p->index, p->data);
		}

		p = p->next;
		free(op);
		op = NULL;
	}

	free(tree);
	
/*
 *	Let's start grouping drives listed as OK...
 */

	if (test_treeint(drvok)) {
		printf ("Drive(s) ");
		last = top = pop_treeint_index(drvok);
		while (test_treeint(drvok)) {
			i = pop_treeint_index(drvok);
			if (i + 1 == last)
				last = i;
			else {
				if (top == last)
					printf("%d,", top);
				else
					printf("%d-%d,", top, last);
				top = last = i;
			}
		}
		if (top == last)
			printf("%d", i);
		else
			printf("%d-%d", top, last);
		printf(" OK");
		
		if (test_treeint(drvwarn) || test_treeint(drvcrit))
			printf(", ");
	}
	
  if (test_treeint(drvwarn)) {
    printf ("Drive(s) ");
    last = top = pop_treeint_index(drvwarn);
		ret = WARNING;
      
		while (test_treeint(drvwarn)) {
      i = pop_treeint_index(drvwarn);
                        
			if (i + 1 == last)
        last = i;
      else {
				if (top == last)
          printf("%d,", top);
        else
          printf("%d-%d,", top, last);
        top = last = i;
      }
    }
       
		if (top == last)
      printf("%d", i);
		else
			printf("%d-%d", top, last);
    printf(" DEGRADED");
			
		if (test_treeint(drvcrit))
			printf(", ");
  }
  
	if (test_treeint(drvcrit)) {
    printf ("Drive(s) ");
    last = top = pop_treeint_index(drvcrit);
		ret = CRITICAL;
    
		while (test_treeint(drvcrit)) {
      i = pop_treeint_index(drvcrit);
      
			if (i + 1 == last)
        last = i;
      else {
        if (top == last)
          printf("%d,", top);
        else
          printf("%d-%d,", top, last);
        top = last = i;
      }
    }
    
		if (top == last)
      printf("%d", i);
		else
			printf("%d-%d", top, last);
    printf(" FAILED");
  }

	free(drvok);
	free(drvwarn);
	free(drvcrit);

	printf("\n");
	
	return ret;
}

int parse_args(int argc, char **argv)
{
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "hh:c:w:H:C:")) != -1)
	{
		switch (c)
		{
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
