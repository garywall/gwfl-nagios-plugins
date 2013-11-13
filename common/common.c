/* $Id: common.c,v 1.4 2005/11/10 10:17:14 gary Exp $
 *
 * common.c -- some common stuff used throughout other .c's 
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

#define NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC  3

/* Almost a carbon copy of snmpwalk() */
struct treeint *walkoid(char *host, char *community, char *oidtable)
{
	struct snmp_session session;
	struct snmp_session *sess_handle;
  struct snmp_pdu *response;
	struct snmp_pdu *pdu;
  struct variable_list *vars;
	
	oid root_oid[MAX_OID_LEN];	
  oid next_oid[MAX_OID_LEN];
	size_t root_len = MAX_OID_LEN;
	size_t next_len = MAX_OID_LEN;
	
	int status, count, check, i, running = 0;
	struct treeint *tree = NULL;
	char buf[BUFSIZE];
	
	init_snmp("SNMP");
	snmp_sess_init(&session);
	session.version = SNMP_VERSION_1;
	session.community = community;
	session.community_len = strlen((char *) session.community);
	session.peername = host;

	if (!(sess_handle = snmp_open(&session))) {
		snmp_sess_perror(PROGNAME, &session);
		exit(UNKNOWN);
	}

	if (!read_objid(oidtable, root_oid, &root_len)) {
		snmp_perror(oidtable);
		exit(UNKNOWN);
	}
	
	init_mib();

	memmove(next_oid, root_oid, root_len  * sizeof(oid));
	next_len = root_len;

	running = 1;

	check = !netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
		NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC);
	
	for (i = 0; running;) {
		pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
  	snmp_add_null_var(pdu, next_oid, next_len);

  	status = snmp_synch_response(sess_handle, pdu, &response);

		if (status == STAT_SUCCESS) {
			if (response->errstat == SNMP_ERR_NOERROR) {
				for (vars = response->variables; vars;
					vars = vars->next_variable) {
					if ((vars->name_length < root_len)
						|| (memcmp(root_oid, vars->name, root_len * sizeof(oid)) != 0)) {
						running = 0;
						continue;
					}

					if ((vars->type != SNMP_ENDOFMIBVIEW) &&
							(vars->type != SNMP_NOSUCHOBJECT) &&
							(vars->type != SNMP_NOSUCHINSTANCE)) {
						if (check && snmp_oid_compare(next_oid, next_len,
							vars->name, vars->name_length) >= 0) {
							printf("Error: OID not increasing: ");
							print_objid(next_oid, next_len);
							printf(" >= ");
							print_objid(vars->name, vars->name_length);
							printf("\n");
							running = 0;
						}

						if (tree == NULL)
							tree = init_treeint();

						snprint_value(buf, BUFSIZE, vars->name, vars->name_length, vars);
						push_treeint(tree, i, strtod(scrub(buf), NULL));

						memmove((char *) next_oid, response->variables->name,
							response->variables->name_length * sizeof(oid));
						next_len = response->variables->name_length;
						i++;
					} else
						running = 0;
				}
			} else {
				running = 0;
				if (response->errstat == SNMP_ERR_NOSUCHNAME) {
					printf("End of MIB\n");
				} else {
					printf("Error in packet.\nReason: %s\n", 
						snmp_errstring(response->errstat));
					if (response->errindex != 0) {
						printf("Failed object: ");
						for (count = 1, vars = response->variables;
							vars && count != response->errindex;
							vars = vars->next_variable, count++)
							;
						if (vars)
							print_objid(vars->name, vars->name_length);
						printf("\n");
					}
				}
			}
		} else if (status == STAT_TIMEOUT) {
			printf("Timeout: No Response from %s\n", session.peername);
			running = 0;
		} else {
			snmp_sess_perror("snmpwalk", sess_handle);
			running = 0;
		}

		if (response)
			snmp_free_pdu(response);
	}

	if (tree == NULL && status == STAT_SUCCESS) {
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		snmp_add_null_var(pdu, root_oid, root_len);
		status = snmp_synch_response(sess_handle, pdu, &response);

		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
			for (vars = response->variables; vars; vars = vars->next_variable) {
				tree = init_treeint();
				snprint_value(buf, BUFSIZE, vars->name, vars->name_length, vars);
				push_treeint(tree, i, strtod(scrub(buf), NULL));
			}
		}

		if (response)
			snmp_free_pdu(response);
	}

	snmp_close(sess_handle);

	return tree;		
}
	
char *scrub(char *show)
{
	if (!show || show == NULL)
		return NULL;

	if (strstr(show, "Gauge: "))
		return strstr(show, "Gauge: ") + 7;
	else if (strstr(show, "Gauge32: "))
		return strstr(show, "Gauge32: ") + 9;
	else if (strstr(show, "Counter32: "))
		return strstr(show, "Counter32: ") + 11;
	else if (strstr(show, "INTEGER: "))
		return strstr(show, "INTEGER: ") + 9;
	else if (strstr(show, "STRING: "))
		return strstr(show, "STRING: ") + 8;

	return show;
					
}
	
void print_help(void)
{
	printf("Program: " PROGNAME " - Revision " REVISION "\nAuthor: " 
		AUTHOR " <" EMAIL "> \nCopyright: " COPYRIGHT "\nSummary: " SUMMARY 
		"\n\nOptions: " OPTIONS "\n\n");
}

struct treeint *push_treeint(struct treeint *root, int index, double data)
{
	struct treeint	*p = root;

/*
 *	Create dummy root if we did'nt supply a valid root:
 *	Also fill it with bogus data
 */
	if (root == NULL)
	{
		p = (struct treeint *) malloc(sizeof(struct treeint));
		p->index = 0;
		p->data = 0;
		p->next = NULL;
		return p;
	}

	while (p->next != NULL)
		p = p->next;

	p->next = (struct treeint *) malloc(sizeof(struct treeint));
	p->next->index = index;
	p->next->data = data;
	p->next->next = NULL;
	return p->next;
}

struct treeint *init_treeint()
{
	return push_treeint(NULL, 0, 0);
}

void print_treeint(struct treeint *root)
{
	struct treeint	*p = root->next;
	struct treeint  *op = NULL;
	
	while (p != NULL) {
		printf("index: %d, data: %.0f\n", p->index, p->data);
		op = p;
		p = p->next;
		free(op);
		op = NULL;
	}
}

int pop_treeint_index(struct treeint *root)
{
	struct treeint  *p = root;
	int   index;

	while (p->next->next != NULL)
		p = p->next;

	index = p->next->index;
	free(p->next);
	p->next = NULL;
	return index;
}

int test_treeint(struct treeint *root)
{
	if (root == NULL || root->next == NULL)
		return FALSE;
	return TRUE;
}
