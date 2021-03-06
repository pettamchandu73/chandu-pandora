/* 
 * $Id: urecord.c,v 1.2 2006/12/15 08:10:16 hari Exp $ 
 *
 * Usrloc record structure
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
 * History:
 * ---------
 * 2003-03-12 added replication mark and zombie state support (nils)
 * 2004-03-17 generic callbacks added (bogdan)
 * 2004-06-07 updated to the new DB api (andrei)
 */



#include "urecord.h"
#include <string.h>
#include "../../mem/shm_mem.h"
#include "../../dprint.h"
#include "../../ut.h"
#include "ul_mod.h"
#include "utime.h"
/* #include "del_list.h" */
/* #include "ins_list.h" */
#include "notify.h"
#include "ul_callback.h"


/*
 * Create and initialize new record structure
 */
int new_urecord(str* _dom, str* _aor, urecord_t** _r)
{
	*_r = (urecord_t*)shm_malloc(sizeof(urecord_t));
	if (*_r == 0) {
		LOG(L_ERR, "new_urecord(): No memory left\n");
		return -1;
	}
	memset(*_r, 0, sizeof(urecord_t));

	(*_r)->aor.s = (char*)shm_malloc(_aor->len);
	if ((*_r)->aor.s == 0) {
		LOG(L_ERR, "new_urecord(): No memory left\n");
		shm_free(*_r);
		return -2;
	}
	memcpy((*_r)->aor.s, _aor->s, _aor->len);
	(*_r)->aor.len = _aor->len;
	(*_r)->domain = _dom;
	return 0;
}


/*
 * Free all memory used by the given structure
 * The structure must be removed from all linked
 * lists first
 */
void free_urecord(urecord_t* _r)
{
	notify_cb_t* watcher;
	ucontact_t* ptr;

	while(_r->watchers) {
		watcher = _r->watchers;
		_r->watchers = watcher->next;
		shm_free(watcher);
	}


	while(_r->contacts) {
		ptr = _r->contacts;
		_r->contacts = _r->contacts->next;
		free_ucontact(ptr);
	}
	
	if (_r->aor.s) shm_free(_r->aor.s);
	shm_free(_r);
}


/*
 * Print a record
 */
void print_urecord(FILE* _f, urecord_t* _r)
{
	ucontact_t* ptr;

	fprintf(_f, "...Record(%p)...\n", _r);
	fprintf(_f, "domain: '%.*s'\n", _r->domain->len, ZSW(_r->domain->s));
	fprintf(_f, "aor   : '%.*s'\n", _r->aor.len, ZSW(_r->aor.s));
	
	if (_r->contacts) {
		ptr = _r->contacts;
		while(ptr) {
			print_ucontact(_f, ptr);
			ptr = ptr->next;
		}
	}

	fprintf(_f, ".../Record...\n");
}


/*
 * Add a new contact
 * Contacts are ordered by: 1) q 
 *                          2) descending modification time
 */
int mem_insert_ucontact(urecord_t* _r, str* _c, time_t _e, qvalue_t _q, str* _cid, int _cs, 
			unsigned int _flags, struct ucontact** _con, str* _ua, str* _recv)
{
	ucontact_t* ptr, *prev = 0;

	if (new_ucontact(_r->domain, &_r->aor, _c, _e, _q, _cid, _cs, _flags, _con, _ua, _recv) < 0) {
		LOG(L_ERR, "mem_insert_ucontact(): Can't create new contact\n");
		return -1;
	}
	
	ptr = _r->contacts;

	if (!desc_time_order) {
		while(ptr) {
			if (ptr->q < _q) break;
			prev = ptr;
			ptr = ptr->next;
		}
	}

	if (ptr) {
		if (!ptr->prev) {
			ptr->prev = *_con;
			(*_con)->next = ptr;
			_r->contacts = *_con;
		} else {
			(*_con)->next = ptr;
			(*_con)->prev = ptr->prev;
			ptr->prev->next = *_con;
			ptr->prev = *_con;
		}
	} else if (prev) {
		prev->next = *_con;
		(*_con)->prev = prev;
	} else {
		_r->contacts = *_con;
	}

	return 0;
}


/*
 * Remove the contact from lists
 */
void mem_remove_ucontact(urecord_t* _r, ucontact_t* _c)
{
	if (_c->prev) {
		_c->prev->next = _c->next;
		if (_c->next) {
			_c->next->prev = _c->prev;
		}
	} else {
		_r->contacts = _c->next;
		if (_c->next) {
			_c->next->prev = 0;
		}
	}
}	



/*
 * Remove contact from the list and delete
 */
void mem_delete_ucontact(urecord_t* _r, ucontact_t* _c)
{
	mem_remove_ucontact(_r, _c);
	free_ucontact(_c);
}


/*
 * This timer routine is used when
 * db_mode is set to NO_DB
 */
static inline int nodb_timer(urecord_t* _r)
{
	ucontact_t* ptr, *t;
	int not = 0;

	ptr = _r->contacts;

	while(ptr) {
		if (!VALID_CONTACT(ptr, act_time)) {
			/* run callbacks for EXPIRE event */
			if (exists_ulcb_type(UL_CONTACT_EXPIRE))
				run_ul_callbacks( UL_CONTACT_EXPIRE, ptr);

			notify_watchers(_r, ptr, PRES_OFFLINE);

			LOG(L_NOTICE, "Binding '%.*s','%.*s' has expired\n",
			    ptr->aor->len, ZSW(ptr->aor->s),
			    ptr->c.len, ZSW(ptr->c.s));
			
			t = ptr;
			ptr = ptr->next;
			
			     /* it was the last contact and it was in normal
			      * state, so notify */
			if (!ptr && t->state == CS_NEW) not=1;
			
			mem_delete_ucontact(_r, t);
			_r->slot->d->expired++;
		} else {
			ptr = ptr->next;
		}
	}

	return 0;
}



/*
 * This routine is used when db_mode is
 * set to WRITE_THROUGH
 */
static inline int wt_timer(urecord_t* _r)
{
	ucontact_t* ptr, *t;
	int not = 0;
	
	ptr = _r->contacts;
	
	while(ptr) {
		if (!VALID_CONTACT(ptr, act_time)) {
			/* run callbacks for EXPIRE event */
			if (exists_ulcb_type(UL_CONTACT_EXPIRE)) {
				run_ul_callbacks( UL_CONTACT_EXPIRE, ptr);
			}

			notify_watchers(_r, ptr, PRES_OFFLINE);

			LOG(L_NOTICE, "Binding '%.*s','%.*s' has expired\n",
			    ptr->aor->len, ZSW(ptr->aor->s),
			    ptr->c.len, ZSW(ptr->c.s));
			
			t = ptr;
			ptr = ptr->next;
			
			     /* it was the last contact and it was in normal
			      * state, so notify */
			if (!ptr && t->state == CS_SYNC) not=1;
			
			if (db_delete_ucontact(t) < 0) {
				LOG(L_ERR, "wt_timer(): Error while deleting contact from "
				    "database\n");
			}
			mem_delete_ucontact(_r, t);
			_r->slot->d->expired++;
		} else {
			     /* the contact was unregistered and is not marked 
			      * for replication so remove it, but the notify was
			      * already done during unregister */
			ptr = ptr->next;
		}
	}
	
	return 0;
}



/*
 * Write-back timer
 */
static inline int wb_timer(urecord_t* _r)
{
	ucontact_t* ptr, *t;
	int op;
	int not = 0;

	ptr = _r->contacts;

	while(ptr) {
		if (!VALID_CONTACT(ptr, act_time)) {
			/* run callbacks for EXPIRE event */
			if (exists_ulcb_type(UL_CONTACT_EXPIRE)) {
				run_ul_callbacks( UL_CONTACT_EXPIRE, ptr);
			}

			notify_watchers(_r, ptr, PRES_OFFLINE);

			LOG(L_NOTICE, "Binding '%.*s','%.*s' has expired\n",
			    ptr->aor->len, ZSW(ptr->aor->s),
			    ptr->c.len, ZSW(ptr->c.s));
			if (ptr->next == 0) not=1;
			_r->slot->d->expired++;

			t = ptr;
			ptr = ptr->next;
			
			     /* Should we remove the contact from the database ? */
			if (st_expired_ucontact(t) == 1) {
				if (db_delete_ucontact(t) < 0) {
					LOG(L_ERR, "wb_timer(): Can't delete contact from the database\n");
				}
			}
			
			mem_delete_ucontact(_r, t);
		} else {
			     /* Determine the operation we have to do */
			op = st_flush_ucontact(ptr);
			
			switch(op) {
			case 0: /* do nothing, contact is synchronized */
				break;

			case 1: /* insert */
				if (db_insert_ucontact(ptr) < 0) {
					LOG(L_ERR, "wb_timer(): Error while inserting contact into database\n");
				}
				break;

			case 2: /* update */
				if (db_update_ucontact(ptr,1) < 0) {
					LOG(L_ERR, "wb_timer(): Error while updating contact in db\n");
				}
				break;

			case 4: /* delete */
				if (db_delete_ucontact(ptr) < 0) {
					LOG(L_ERR, "wb_timer(): Can't delete contact from database\n");
				}
				     /* fall through to the next case statement */

			case 3: /* delete from memory */
				mem_delete_ucontact(_r, ptr);
				break;
			}

			ptr = ptr->next;
		}
	}

	return 0;
}



int timer_urecord(urecord_t* _r)
{
	switch(db_mode) {
	case NO_DB:         return nodb_timer(_r);
	case WRITE_THROUGH: return wt_timer(_r);
	case WRITE_BACK:    return wb_timer(_r);
	}

	return 0; /* Makes gcc happy */
}



int db_delete_urecord(urecord_t* _r)
{
	char b[256];
	db_key_t keys[2];
	db_val_t vals[2];
	char* dom;

	keys[0] = user_col.s;
	keys[1] = domain_col.s;
	vals[0].type = DB_STR;
	vals[0].nul = 0;
	vals[0].val.str_val.s = _r->aor.s;
	vals[0].val.str_val.len = _r->aor.len;

	if (use_domain) {
		dom = q_memchr(_r->aor.s, '@', _r->aor.len);
		vals[0].val.str_val.len = dom - _r->aor.s;

		vals[1].type = DB_STR;
		vals[1].nul = 0;
		vals[1].val.str_val.s = dom + 1;
		vals[1].val.str_val.len = _r->aor.s + _r->aor.len - dom - 1;
	}

	     /* FIXME */
	memcpy(b, _r->domain->s, _r->domain->len);
	b[_r->domain->len] = '\0';
	if (ul_dbf.use_table(ul_dbh, b) < 0) {
		LOG(L_ERR, "ERROR: db_delete_urecord():"
		                " Error in use_table\n");
		return -1;
	}

	if (ul_dbf.delete(ul_dbh, keys, 0, vals, (use_domain) ? (2) : (1)) < 0) {
		LOG(L_ERR, "ERROR: db_delete_urecord():"
				" Error while deleting from database\n");
		return -1;
	}
	return 0;
}


/*
 * Release urecord previously obtained
 * through get_urecord
 */
void release_urecord(urecord_t* _r)
{
	if (_r->contacts == 0) {
		mem_delete_urecord(_r->slot->d, _r);
	}
}


/*
 * Create and insert new contact
 * into urecord
 */
int insert_ucontact(urecord_t* _r, str* _c, time_t _e, qvalue_t _q, str* _cid, 
		    int _cs, unsigned int _flags, struct ucontact** _con, str* _ua, str* _recv)
{
	if (mem_insert_ucontact(_r, _c, _e, _q, _cid, _cs, _flags, _con, _ua, _recv) < 0) {
		LOG(L_ERR, "insert_ucontact(): Error while inserting contact\n");
		return -1;
	}

	notify_watchers(_r, *_con, (_e > 0) ? PRES_ONLINE : PRES_OFFLINE);

	if (exists_ulcb_type(UL_CONTACT_INSERT)) {
		run_ul_callbacks( UL_CONTACT_INSERT, *_con);
	}

	if (db_mode == WRITE_THROUGH) {
		if (db_insert_ucontact(*_con) < 0) {
			LOG(L_ERR, "insert_ucontact(): Error while inserting in database\n");
		}
		(*_con)->state = CS_SYNC;
	}

	return 0;
}


/*
 * Delete ucontact from urecord
 */
int delete_ucontact(urecord_t* _r, struct ucontact* _c)
{
	if (exists_ulcb_type(UL_CONTACT_DELETE)) {
		run_ul_callbacks( UL_CONTACT_DELETE, _c);
	}

	notify_watchers(_r, _c, PRES_OFFLINE);

	if (st_delete_ucontact(_c) > 0) {
		if (db_mode == WRITE_THROUGH) {
			if (db_delete_ucontact(_c) < 0) {
				LOG(L_ERR, "delete_ucontact(): Can't remove contact from "
							"database\n");
			}
		}

		mem_delete_ucontact(_r, _c);
	}

	return 0;
}


/*
 * Get pointer to ucontact with given contact
 */
int get_ucontact(urecord_t* _r, str* _c, struct ucontact** _co)
{
	ucontact_t* ptr;
	char  *pTemp=NULL;	
	int  len1 =  0, len2 = 0;
	ptr = _r->contacts;
	len1 = _c->len; 
	pTemp = memchr(_c->s,';',_c->len);
	if( pTemp ){
		len1 = pTemp - _c->s;	
	}

	while(ptr) {
//		LOG(L_ERR ,  "[get_ucontact] '%.*s'  '%.*s'\n", _c->len, _c->s , ptr->c.len , ptr->c.s);		
		len2 = ptr->c.len;
		pTemp = memchr(ptr->c.s,';',ptr->c.len);
		if (pTemp) {
			len2 = pTemp - ptr->c.s;
		}		
		if ((len1 == len2) && (!memcmp(_c->s, ptr->c.s, len1))) {
				*_co = ptr;
				return 0;		
		}
/*				
		if ((_c->len == ptr->c.len) &&
		    !memcmp(_c->s, ptr->c.s, _c->len)) {
			*_co = ptr;
			return 0;
		}
*/			
		ptr = ptr->next;
	}
	return 1;
}

/*
 * ---- function introduced by svrk
 *  this function takes callid as input unlike the 
 *  original function which takes contact
 *  
 * Get pointer to ucontact with given callid
 */
int get_ucontact_bycallid(urecord_t* _r, str* callid, struct ucontact** _co)
{
	ucontact_t* ptr;
	
	ptr = _r->contacts;
	while(ptr) {
//		LOG(L_ERR ,  "[get_ucontact_bycallid] '%.*s'  '%.*s'\n", callid->len, callid->s , ptr->callid.len,  ptr->callid.s);		
		if ((callid->len == ptr->callid.len) &&
		    !memcmp(callid->s, ptr->callid.s, callid->len)) {
			*_co = ptr;
			return 0;
		}
		
		ptr = ptr->next;
	}
	return 1;
}
