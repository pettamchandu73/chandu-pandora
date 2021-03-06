/*
 * $Id: ul_mod.c,v 1.2 2006/12/15 08:10:16 hari Exp $
 *
 * Usrloc module interface
 *
 * Copyright (C) 2001-2003 FhG Fokus
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
 *
 * History:
 * ---------
 * 2003-01-27 timer activity printing #ifdef-ed to EXTRA_DEBUG (jiri)
 * 2003-03-11 New module interface (janakj)
 * 2003-03-12 added replication and state columns (nils)
 * 2003-03-16 flags export parameter added (janakj)
 * 2003-04-05: default_uri #define used (jiri)
 * 2003-04-21 failed fifo init stops init process (jiri)
 * 2004-03-17 generic callbacks added (bogdan)
 * 2004-06-07 updated to the new DB api (andrei)
 */

#include <stdio.h>
#include "ul_mod.h"
#include "../../sr_module.h"
#include "../../dprint.h"
#include "../../timer.h"     /* register_timer */
#include "../../globals.h"   /* is_main */
#include "dlist.h"           /* register_udomain */
#include "udomain.h"         /* {insert,delete,get,release}_urecord */
#include "urecord.h"         /* {insert,delete,get}_ucontact */
#include "ucontact.h"        /* update_ucontact */
#include "ul_fifo.h"
#include "ul_unixsock.h"
#include "ul_callback.h"
#include "notify.h"
#include "usrloc.h"

MODULE_VERSION

#define USER_COL       "username"
#define DOMAIN_COL     "domain"
#define CONTACT_COL    "contact"
#define EXPIRES_COL    "expires"
#define Q_COL          "q"
#define CALLID_COL     "callid"
#define CSEQ_COL       "cseq"
#define METHOD_COL     "method"
#define STATE_COL      "state"
#define FLAGS_COL      "flags"
#define USER_AGENT_COL "user_agent"
#define RECEIVED_COL   "received"
#define REGISTRAR  "111.111.111.111:65535"

static int mod_init(void);                          /* Module initialization function */
static void destroy(void);                          /* Module destroy function */
static void timer(unsigned int ticks, void* param); /* Timer handler */
static int child_init(int rank);                    /* Per-child init function */

extern int bind_usrloc(usrloc_api_t* api);

/*
 * Module parameters and their default values
 */
str user_col        = {USER_COL, sizeof(USER_COL) - 1};             /* Name of column containing usernames */
str domain_col      = {DOMAIN_COL, sizeof(DOMAIN_COL) - 1};         /* Name of column containing domains */
str contact_col     = {CONTACT_COL, sizeof(CONTACT_COL) - 1};       /* Name of column containing contact addresses */
str expires_col     = {EXPIRES_COL, sizeof(EXPIRES_COL) - 1};       /* Name of column containing expires values */
str q_col           = {Q_COL, sizeof(Q_COL) - 1};                   /* Name of column containing q values */
str callid_col      = {CALLID_COL, sizeof(CALLID_COL) - 1};         /* Name of column containing callid string */
str cseq_col        = {CSEQ_COL, sizeof(CSEQ_COL) - 1};             /* Name of column containing cseq values */
str method_col      = {METHOD_COL, sizeof(METHOD_COL) - 1};         /* Name of column containing supported method */
str state_col       = {STATE_COL, sizeof(STATE_COL) - 1};           /* Name of column containing contact state */
str flags_col       = {FLAGS_COL, sizeof(FLAGS_COL) - 1};           /* Name of column containing flags */
str user_agent_col  = {USER_AGENT_COL, sizeof(USER_AGENT_COL) - 1}; /* Name of column containing user agent string */
str received_col    = {RECEIVED_COL, sizeof(RECEIVED_COL) - 1};     /* Name of column containing transport info of REGISTER */
str db_url          = {DEFAULT_DB_URL, sizeof(DEFAULT_DB_URL) - 1}; /* Database URL */
str registrar		= {REGISTRAR, sizeof(REGISTRAR) - 1};			/* the IP Address and Port on which this "Registrar" is running */
int timer_interval  = 60;             /* Timer interval in seconds */
int db_mode         = 0;              /* Database sync scheme: 0-no db, 1-write through, 2-write back */
int use_domain      = 0;              /* Whether usrloc should use domain part of aor */
int desc_time_order = 0;              /* By default do not enable timestamp ordering */                  


db_con_t* ul_dbh; /* Database connection handle */
db_func_t ul_dbf;


/*
 * Exported functions
 */
static cmd_export_t cmds[] = {
	{"ul_register_udomain",   (cmd_function)register_udomain,   1, 0, 0},
	{"ul_insert_urecord",     (cmd_function)insert_urecord,     1, 0, 0},
	{"ul_delete_urecord",     (cmd_function)delete_urecord,     1, 0, 0},
	{"ul_get_urecord",        (cmd_function)get_urecord,        1, 0, 0},
	{"ul_lock_udomain",       (cmd_function)lock_udomain,       1, 0, 0},
	{"ul_unlock_udomain",     (cmd_function)unlock_udomain,     1, 0, 0},
	{"ul_release_urecord",    (cmd_function)release_urecord,    1, 0, 0},
	{"ul_insert_ucontact",    (cmd_function)insert_ucontact,    1, 0, 0},
	{"ul_delete_ucontact",    (cmd_function)delete_ucontact,    1, 0, 0},
	{"ul_get_ucontact",       (cmd_function)get_ucontact,       1, 0, 0},
	{"ul_get_all_ucontacts",  (cmd_function)get_all_ucontacts,  1, 0, 0},
	{"ul_update_ucontact",    (cmd_function)update_ucontact,    1, 0, 0},
	{"ul_register_watcher",   (cmd_function)register_watcher,   1, 0, 0},
	{"ul_unregister_watcher", (cmd_function)unregister_watcher, 1, 0, 0},
	{"ul_bind_usrloc",        (cmd_function)bind_usrloc,        1, 0, 0},
	{"ul_register_ulcb",      (cmd_function)register_ulcb,      1, 0, 0},
	// introduced by svrk -- starts
	{"ul_get_ucontact_bycallid",    (cmd_function)get_ucontact_bycallid,    1, 0, 0},
	{"ul_update_ucontact_bycallid",    (cmd_function)update_ucontact_bycallid,    1, 0, 0},
	// introduced by svrk -- ends
	{0, 0, 0, 0, 0}
};


/* 
 * Exported parameters 
 */
static param_export_t params[] = {
	{"user_column",       STR_PARAM, &user_col.s     },
	{"domain_column",     STR_PARAM, &domain_col.s   },
	{"contact_column",    STR_PARAM, &contact_col.s  },
	{"expires_column",    STR_PARAM, &expires_col.s  },
	{"q_column",          STR_PARAM, &q_col.s        },
	{"callid_column",     STR_PARAM, &callid_col.s   },
	{"cseq_column",       STR_PARAM, &cseq_col.s     },
	{"method_column",     STR_PARAM, &method_col.s   },
	{"flags_column",      STR_PARAM, &flags_col.s    },
	{"db_url",            STR_PARAM, &db_url.s       },
	{"timer_interval",    INT_PARAM, &timer_interval },
	{"db_mode",           INT_PARAM, &db_mode        },
	{"use_domain",        INT_PARAM, &use_domain     },
	{"desc_time_order",   INT_PARAM, &desc_time_order},
	{"user_agent_column", STR_PARAM, &user_agent_col },
	{"received_column",   STR_PARAM, &received_col   },
	{"registrar",         STR_PARAM, &registrar      },
	{0, 0, 0}
};


struct module_exports exports = {
	"usrloc",
	cmds,       /* Exported functions */
	params,     /* Export parameters */
	mod_init,   /* Module initialization function */
	0,          /* Response function */
	destroy,    /* Destroy function */
	0,          /* OnCancel function */
	child_init  /* Child initialization function */
};


/*
 * Module initialization function
 */
static int mod_init(void)
{
	DBG("usrloc - initializing\n");

	     /* Compute the lengths of string parameters */
	user_col.len = strlen(user_col.s);
	domain_col.len = strlen(domain_col.s);
	contact_col.len = strlen(contact_col.s);
	expires_col.len = strlen(expires_col.s);
	q_col.len = strlen(q_col.s);
	callid_col.len = strlen(callid_col.s);
	cseq_col.len = strlen(cseq_col.s);
	method_col.len = strlen(method_col.s);
	flags_col.len = strlen(flags_col.s);
	user_agent_col.len = strlen(user_agent_col.s);
	received_col.len = strlen(received_col.s);
	db_url.len = strlen(db_url.s);

	     /* Register cache timer */
	register_timer(timer, 0, timer_interval);

	     /* Initialize fifo interface */
	if (init_ul_fifo() < 0) {
		LOG(L_ERR, "ERROR: usrloc/fifo initialization failed\n");
		return -1;
	}

	if (init_ul_unixsock() < 0) {
		LOG(L_ERR, "ERROR: usrloc/unixsock initialization failed\n");
		return -1;
	}

	/* init the callbacks list */
	if ( init_ulcb_list() < 0) {
		LOG(L_ERR, "ERROR: usrloc/callbacks initialization failed\n");
		return -1;
	}

	/* Shall we use database ? */
	if (db_mode != NO_DB) { /* Yes */
		if (bind_dbmod(db_url.s, &ul_dbf) < 0) { /* Find database module */
			LOG(L_ERR, "ERROR: mod_init(): Can't bind database module\n");
			return -1;
		}
		if (!DB_CAPABILITY(ul_dbf, DB_CAP_ALL)) {
			LOG(L_ERR, "usrloc:mod_init: Database module does not implement"
						" all functions needed by the module\n");
			return -1;
		}
	}


	return 0;
}


static int child_init(int _rank)
{
 	     /* Shall we use database ? */
	if (db_mode != NO_DB) { /* Yes */
		ul_dbh = ul_dbf.init(db_url.s); /* Get a new database connection */
		if (!ul_dbh) {
			LOG(L_ERR, "ERROR: child_init(%d): "
					"Error while connecting database\n", _rank);
			return -1;
		}
	}

	return 0;
}


/*
 * Module destroy function
 */
static void destroy(void)
{
	/* Parent only, synchronize the world
	* and then nuke it */
	if (is_main) {
		if (synchronize_all_udomains() != 0) {
			LOG(L_ERR, "timer(): Error while flushing cache\n");
		}
		free_all_udomains();
	}
	
	/* All processes close database connection */
	if (ul_dbh) ul_dbf.close(ul_dbh);

	/* free callbacks list */
	destroy_ulcb_list();
}


/*
 * Timer handler
 */
static void timer(unsigned int ticks, void* param)
{
	if (synchronize_all_udomains() != 0) {
		LOG(L_ERR, "timer(): Error while synchronizing cache\n");
	}
}
