/* $Id: common.h,v 1.3 2005/11/10 10:15:30 gary Exp $
 *
 * common.h -- common variables
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

#define REVISION "2.0"
#define COPYRIGHT "2005"
#define AUTHOR "Gary Wall & Fredrik Löhr"
#define EMAIL "gwfl-authors@daimonic.org"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <getopt.h>

#define BUFSIZE 512 

/* Nagios standard return values */
#define OK        0
#define WARNING   1
#define CRITICAL  2
#define UNKNOWN   3
/* #define DEPENDENT 4 - unused */

/* Measurements */
#define TB 1099511627776ULL
#define GB 1073741824

/* function prototypes */
int parse_args(int, char **);

/* common.c prototypes */
struct treeint *walkoid(char *, char *, char *);
char *scrub(char *);
void print_help(void);

/* common variables */
char *host = NULL;
unsigned char *community = NULL;

struct treeint {
	int index;
	double data;
	struct treeint 	*next;
};

struct treeint *push_treeint(struct treeint *, int, double);
struct treeint *init_treeint();
void print_treeint(struct treeint *);
int test_treeint(struct treeint *);
int pop_treeint_index(struct treeint *);
