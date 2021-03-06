/*
 * $Id: domain_mod.c,v 1.1.1.1 2006/08/31 22:40:52 hari Exp $
 *
 * Domain module
 *
 * Copyright (C) 2002-2003 Juha Heinanen
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
 *
 * History:
 * -------
 * 2003-03-11: New module interface (janakj)
 * 2003-03-16: flags export parameter added (janakj)
 * 2003-04-05: default_uri #define used (jiri)
 * 2003-04-06: db connection closed in mod_init (janakj)
 * 2004-06-06  updated to the new DB api, cleanup: static dbf & handler,
 *              calls to domain_db_{bind,init,close,ver} (andrei)
 */


#include "domain_mod.h"
#include <stdio.h>
#include "../../mem/shm_mem.h"
#include "../../sr_module.h"
#include "domain.h"
#include "fifo.h"
#include "unixsock.h"

/*
 * Module management function prototypes
 */
static int mod_init(void);
static void destroy(void);
static int child_init(int rank);

MODULE_VERSION

/*
 * Version of domain table required by the module,
 * increment this value if you change the table in
 * an backwards incompatible way
 */
#define TABLE_VERSION 1

#define DOMAIN_TABLE "domain"
#define DOMAIN_TABLE_LEN (sizeof(DOMAIN_TABLE) - 1)

#define DOMAIN_COL "domain"
#define DOMAIN_COL_LEN (sizeof(DOMAIN_COL) - 1)

/*
 * Module parameter variables
 */
static str db_url = {DEFAULT_RODB_URL, DEFAULT_RODB_URL_LEN};
int db_mode = 0;			/* Database usage mode: 0 = no cache, 1 = cache */
str domain_table = {DOMAIN_TABLE, DOMAIN_TABLE_LEN};     /* Name of domain table */
str domain_col = {DOMAIN_COL, DOMAIN_COL_LEN};           /* Name of domain column */

/*
 * Other module variables
 */
struct domain_list ***hash_table;	/* Pointer to current hash table pointer */
struct domain_list **hash_table_1;	/* Pointer to hash table 1 */
struct domain_list **hash_table_2;	/* Pointer to hash table 2 */


/*
 * Exported functions
 */
static cmd_export_t cmds[] = {
	{"is_from_local",     is_from_local,     0, 0, REQUEST_ROUTE},
	{"is_uri_host_local", is_uri_host_local, 0, 0, REQUEST_ROUTE},
	{0, 0, 0, 0, 0}
};


/*
 * Exported parameters
 */
static param_export_t params[] = {
	{"db_url",		STR_PARAM, &db_url.s	  },
	{"db_mode",             INT_PARAM, &db_mode       },
	{"domain_table",        STR_PARAM, &domain_table.s},
	{"domain_col",          STR_PARAM, &domain_col.s  },
	{0, 0, 0}
};


/*
 * Module interface
 */
struct module_exports exports = {
	"domain", 
	cmds,      /* Exported functions */
	params,    /* Exported parameters */
	mod_init,  /* module initialization function */
	0,         /* response function*/
	destroy,   /* destroy function */
	0,         /* cancel function */
	child_init /* per-child init function */
};


static int mod_init(void)
{
	int i, ver;

	DBG("domain - initializing\n");
	
	db_url.len = strlen(db_url.s);
	domain_table.len = strlen(domain_table.s);
	domain_col.len = strlen(domain_col.s);

	/* Check if database module has been loaded */
	if (domain_db_bind(db_url.s)<0)  return -1;

	/* Check if cache needs to be loaded from domain table */
	if (db_mode != 0) {
		if (domain_db_init(db_url.s)<0) return -1;
		     /* Check table version */
		ver = domain_db_ver(&domain_table);
		if (ver < 0) {
			LOG(L_ERR, "ERROR: domain:mod_init(): "
					"error while querying table version\n");
			domain_db_close();
			return -1;
		} else if (ver < TABLE_VERSION) {
			LOG(L_ERR, "ERROR: domain:mod_init(): invalid table"
					" version (use ser_mysql.sh reinstall)\n");
			domain_db_close();
			return -1;
		}		

		/* Initialize fifo interface */
		(void)init_domain_fifo();

		if (init_domain_unixsock() < 0) {
			LOG(L_ERR, "ERROR: domain:mod_init(): error while initializing"
					" unix socket interface\n");
			domain_db_close();
			return -1;
		}

		/* Initializing hash tables and hash table variable */
		hash_table_1 = (struct domain_list **)shm_malloc(sizeof(struct domain_list *) * HASH_SIZE);
		if (hash_table_1 == 0) {
			LOG(L_ERR, "ERROR: domain: mod_init(): "
					"No memory for hash table\n");
		}

		hash_table_2 = (struct domain_list **)shm_malloc(sizeof(struct domain_list *) * HASH_SIZE);
		if (hash_table_2 == 0) {
			LOG(L_ERR, "ERROR: domain: mod_init():"
					" No memory for hash table\n");
		}
		for (i = 0; i < HASH_SIZE; i++) {
			hash_table_1[i] = hash_table_2[i] = (struct domain_list *)0;
		}

		hash_table = (struct domain_list ***)shm_malloc(sizeof(struct domain_list *));
		*hash_table = hash_table_1;

		if (reload_domain_table() == -1) {
			LOG(L_CRIT, "ERROR: domain:mod_init():"
					" Domain table reload failed\n");
			return -1;
		}
			
		domain_db_close();
	}

	return 0;
}


static int child_init(int rank)
{
	/* Check if database is needed by child */
	if (((db_mode == 0) && (rank > 0)) || ((db_mode != 0) && (rank == PROC_FIFO))) {
		if (domain_db_init(db_url.s)<0) {
			LOG(L_ERR, "ERROR: domain:child_init():"
					" Unable to connect to the database\n");
			return -1;
		}
	}
	return 0;
}


static void destroy(void)
{
	/* Destroy is called from the main process only,
	 * there is no need to close database here because
	 * it is closed in mod_init already
	 */
}
