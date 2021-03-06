/*
 * $Id: unixsock.c,v 1.1.1.1 2006/08/31 22:40:52 hari Exp $
 *
 * UNIX Socket Interface
 *
 * Copyright (C) 2002-2004 FhG FOKUS
 *
 * This file is part of msicp, a free SIP server.
 *
 * msicp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the msicp software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * msicp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hash.h"
#include "../../unixsock_server.h"
#include "../../dprint.h"
#include "../../db/db.h"
#include "../../ut.h"
#include "unixsock.h"
#include "fifo.h" /* Because of reload_domain_table */


/*
 * Fifo function to reload domain table
 */
static int domain_reload(str* msg)
{
	if (reload_domain_table () == 1) {
		unixsock_reply_asciiz("200 OK\n");
		unixsock_reply_send();
		return 0;
	} else {
		unixsock_reply_asciiz("400 Domain table reload failed\n");
		unixsock_reply_send();
		return -1;
	}
}


/* Print domains stored in hash table */
static int hash_table_print_unixsock(struct domain_list **hash_table)
{
	int i;
	struct domain_list *np;

	for (i = 0; i < HASH_SIZE; i++) {
		np = hash_table[i];
		while (np) {
			if (unixsock_reply_printf("%4d %.*s\n", i, np->domain.len, ZSW(np->domain.s)) < 0) {
				return -1;
			}
			np = np->next;
		}
	}
	return 0;
}


/*
 * Fifo function to print domains from current hash table
 */
static int domain_dump(str* msg)
{
	unixsock_reply_asciiz("200 OK\n");
	if (hash_table_print_unixsock(*hash_table) < 0) {
		unixsock_reply_reset();
		unixsock_reply_asciiz("500 Could not print the contents\n");
		unixsock_reply_send();
		return -1;
	}
	unixsock_reply_send();
	return 0;
}


/*
 * Register domain unixsock functions
 */
int init_domain_unixsock(void) 
{
	if (unixsock_register_cmd("domain_reload", domain_reload) < 0) {
		LOG(L_ERR, "init_domain_unixsock: Cannot register domain_reload\n");
		return -1;
	}

	if (unixsock_register_cmd("domain_dump", domain_dump) < 0) {
		LOG(L_ERR, "init_domain_unixsock: Cannot register domain_dump\n");
		return -1;
	}

	return 0;
}
