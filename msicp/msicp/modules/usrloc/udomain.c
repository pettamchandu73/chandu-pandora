/* 
 * $Id: udomain.c,v 1.2 2006/12/15 08:10:16 hari Exp $ 
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
 * 2003-03-11 changed to the new locking scheme: locking.h (andrei)
 * 2003-03-12 added replication mark and zombie state (nils)
 * 2004-06-07 updated to the new DB api (andrei)
 * 2004-08-23  hash function changed to process characters as unsigned
 *             -> no negative results occur (jku)
 *   
 */

#include "udomain.h"
#include <string.h>
#include "../../mem/shm_mem.h"
#include "../../dprint.h"
#include "../../db/db.h"
#include "../../ut.h"
#include "ul_mod.h"            /* usrloc module parameters */
#include "notify.h"


/*
 * Hash function
 */
static inline int hash_func(udomain_t* _d, unsigned char* _s, int _l)
{
	int res = 0, i;
	
	for(i = 0; i < _l; i++) {
		res += _s[i];
	}
	
	return res % _d->size;
}


/*
 * Add a record to list of all records in a domain
 */
static inline void udomain_add(udomain_t* _d, urecord_t* _r)
{
	if (_d->d_ll.n == 0) {
		_d->d_ll.first = _r;
		_d->d_ll.last = _r;
	} else {
		_r->d_ll.prev = _d->d_ll.last;
		_d->d_ll.last->d_ll.next = _r;
		_d->d_ll.last = _r;
	}
	_d->d_ll.n++;
}


/*
 * Remove a record from list of all records in a domain
 */
static inline void udomain_remove(udomain_t* _d, urecord_t* _r)
{
	if (_d->d_ll.n == 0) return;

	if (_r->d_ll.prev) {
		_r->d_ll.prev->d_ll.next = _r->d_ll.next;
	} else {
		_d->d_ll.first = _r->d_ll.next;
	}

	if (_r->d_ll.next) {
		_r->d_ll.next->d_ll.prev = _r->d_ll.prev;
	} else {
		_d->d_ll.last = _r->d_ll.prev;
	}

	_r->d_ll.prev = _r->d_ll.next = 0;
	_d->d_ll.n--;
}


/*
 * Create a new domain structure
 * _n is pointer to str representing
 * name of the domain, the string is
 * not copied, it should point to str
 * structure stored in domain list
 * _s is hash table size
 */
int new_udomain(str* _n, int _s, udomain_t** _d)
{
	int i;

	     /* Must be always in shared memory, since
	      * the cache is accessed from timer which
	      * lives in a separate process
	      */
	*_d = (udomain_t*)shm_malloc(sizeof(udomain_t));
	if (!(*_d)) {
		LOG(L_ERR, "new_udomain(): No memory left\n");
		return -1;
	}
	memset(*_d, 0, sizeof(udomain_t));
	
	(*_d)->table = (hslot_t*)shm_malloc(sizeof(hslot_t) * _s);
	if (!(*_d)->table) {
		LOG(L_ERR, "new_udomain(): No memory left 2\n");
		shm_free(*_d);
		return -2;
	}

	(*_d)->name = _n;
	
	for(i = 0; i < _s; i++) {
		if (init_slot(*_d, &((*_d)->table[i])) < 0) {
			LOG(L_ERR, "new_udomain(): Error while initializing hash table\n");
			shm_free((*_d)->table);
			shm_free(*_d);
			return -3;
		}
	}

	(*_d)->size = _s;
	lock_init(&(*_d)->lock);
	(*_d)->users = 0;
	(*_d)->expired = 0;
	
	return 0;
}


/*
 * Free all memory allocated for
 * the domain
 */
void free_udomain(udomain_t* _d)
{
	int i;
	
	lock_udomain(_d);
	if (_d->table) {
		for(i = 0; i < _d->size; i++) {
			deinit_slot(_d->table + i);
		}
		shm_free(_d->table);
	}
	unlock_udomain(_d);
	lock_destroy(&_d->lock);/* destroy the lock (required for SYSV sems!)*/

        shm_free(_d);
}


/*
 * Just for debugging
 */
void print_udomain(FILE* _f, udomain_t* _d)
{
	struct urecord* r;
	fprintf(_f, "---Domain---\n");
	fprintf(_f, "name : '%.*s'\n", _d->name->len, ZSW(_d->name->s));
	fprintf(_f, "size : %d\n", _d->size);
	fprintf(_f, "table: %p\n", _d->table);
	fprintf(_f, "d_ll {\n");
	fprintf(_f, "    n    : %d\n", _d->d_ll.n);
	fprintf(_f, "    first: %p\n", _d->d_ll.first);
	fprintf(_f, "    last : %p\n", _d->d_ll.last);
	fprintf(_f, "}\n");
	/*fprintf(_f, "lock : %d\n", _d->lock); -- can be a structure --andrei*/
	if (_d->d_ll.n > 0) {
		fprintf(_f, "\n");
		r = _d->d_ll.first;
		while(r) {
			print_urecord(_f, r);
			r = r->d_ll.next;
		}


		fprintf(_f, "\n");
	}
	fprintf(_f, "---/Domain---\n");
}



int preload_udomain(db_con_t* _c, udomain_t* _d)
{
	char b[256];
	//db_key_t columns[10];
	db_res_t* res;
	db_row_t* row;
	int i, cseq;
	unsigned int flags;

	str user, contact, callid, ua, received;
	str* rec;
	char* domain;
	time_t expires;
	qvalue_t q;

	urecord_t* r;
	ucontact_t* c;

	char query[1024];
	char* fmt = "SELECT username, contact, expires, q, callid, cseq, flags, user_agent, received FROM location where registrar='%.*s'";
	sprintf(query, fmt, registrar.len, registrar.s);

	LOG(L_WARN, "preload_udomain(): Loading contact details from DB\n");
		
	/*columns[0] = user_col.s;
	columns[1] = contact_col.s;
	columns[2] = expires_col.s;
	columns[3] = q_col.s;
	columns[4] = callid_col.s;
	columns[5] = cseq_col.s;
	columns[6] = flags_col.s;
	columns[7] = user_agent_col.s;
	columns[8] = received_col.s;
	columns[9] = domain_col.s;*/
	
	memcpy(b, _d->name->s, _d->name->len);
	b[_d->name->len] = '\0';

	if (ul_dbf.use_table(_c, b) < 0) {
		LOG(L_ERR, "preload_udomain(): Error in use_table\n");
		return -1;
	}

	/*if (ul_dbf.query(_c, 0, 0, 0, columns, 0, (use_domain) ? 10 : 9, 0,
				&res) < 0) {
		LOG(L_ERR, "preload_udomain(): Error while doing db_query\n");
		return -1;
	}*/
	
	if ((ul_dbf.raw_query(_c, query, &res) < 0) || (res == NULL))
	{
			LOG(L_ERR, "preload_udomain(): Error while getting contacts\n");
			return -1;
	}
										    
	if (RES_ROW_N(res) == 0) {
		DBG("preload_udomain(): Table is empty\n");
		ul_dbf.free_result(_c, res);
		return 0;
	}

	lock_udomain(_d);

	for(i = 0; i < RES_ROW_N(res); i++) {
		row = RES_ROWS(res) + i;
		
		user.s      = (char*)VAL_STRING(ROW_VALUES(row));
		if (user.s == 0) {
			LOG(L_CRIT, "preload_udomain: ERROR: bad username "
							"record in table %s\n", b);
			LOG(L_CRIT, "preload_udomain: ERROR: skipping...\n");
			continue;
		} else {
			user.len = strlen(user.s);
		}

		contact.s = (char*)VAL_STRING(ROW_VALUES(row) + 1);
		if (contact.s == 0) {
			LOG(L_CRIT, "preload_udomain: ERROR: bad contact "
							"record in table %s\n", b);
			LOG(L_CRIT, "preload_udomain: ERROR: for username %.*s\n",
							user.len, user.s);
			LOG(L_CRIT, "preload_udomain: ERROR: skipping...\n");
			continue;
		} else {
			contact.len = strlen(contact.s);
		}
		expires     = VAL_TIME  (ROW_VALUES(row) + 2);
		q           = double2q(VAL_DOUBLE(ROW_VALUES(row) + 3));
		cseq        = VAL_INT   (ROW_VALUES(row) + 5);
		callid.s    = (char*)VAL_STRING(ROW_VALUES(row) + 4);
		if (callid.s == 0) {
			LOG(L_CRIT, "preload_udomain: ERROR: bad callid record in"
							" table %s\n", b);
			LOG(L_CRIT, "preload_udomain: ERROR: for username %.*s,"
							" contact %.*s\n",
							user.len, user.s, contact.len, contact.s);
			LOG(L_CRIT, "preload_udomain: ERROR: skipping...\n");
			continue;
		} else {
			callid.len  = strlen(callid.s);
		}

		flags  = VAL_BITMAP(ROW_VALUES(row) + 6);

		ua.s  = (char*)VAL_STRING(ROW_VALUES(row) + 7);
		if (ua.s) {
			ua.len = strlen(ua.s);
		} else {
			ua.len = 0;
		}

		if (!VAL_NULL(ROW_VALUES(row) + 8)) {
			received.s  = (char*)VAL_STRING(ROW_VALUES(row) + 8);
			if (received.s) {
				received.len = strlen(received.s);
				rec = &received;
			} else {
				received.len = 0;
				rec = 0;
			}
		} else {
			rec = 0;
		}

		if (use_domain) {
			domain  = (char*)VAL_STRING(ROW_VALUES(row) + 9);
			snprintf(b, 256, "%.*s@%s", user.len, ZSW(user.s), domain);
			user.s = b;
			user.len = strlen(b);
		}

		if (get_urecord(_d, &user, &r) > 0) {
			if (mem_insert_urecord(_d, &user, &r) < 0) {
				LOG(L_ERR, "preload_udomain(): Can't create a record\n");
				ul_dbf.free_result(_c, res);
				unlock_udomain(_d);
				return -2;
			}
		}
		
		if (mem_insert_ucontact(r, &contact, expires, q, &callid, cseq, flags, &c, &ua, rec) < 0) {
			LOG(L_ERR, "preload_udomain(): Error while inserting contact\n");
			ul_dbf.free_result(_c, res);
			unlock_udomain(_d);
			return -3;
		}

		     /* We have to do this, because insert_ucontact sets state to CS_NEW
		      * and we have the contact in the database already
			  * we also store zombies in database so we have to restore
			  * the correct state
		      */
		c->state = CS_SYNC;
	}

	ul_dbf.free_result(_c, res);
	unlock_udomain(_d);
	return 0;
}


/*
 * Insert a new record into domain
 */
int mem_insert_urecord(udomain_t* _d, str* _aor, struct urecord** _r)
{
	int sl;
	
	if (new_urecord(_d->name, _aor, _r) < 0) {
		LOG(L_ERR, "insert_urecord(): Error while creating urecord\n");
		return -1;
	}

	sl = hash_func(_d, (unsigned char*)_aor->s, _aor->len);
	slot_add(&_d->table[sl], *_r);
	udomain_add(_d, *_r);
	_d->users++;
	return 0;
}


/*
 * Remove a record from domain
 */
void mem_delete_urecord(udomain_t* _d, struct urecord* _r)
{
	if (_r->watchers == 0) {
		udomain_remove(_d, _r);
		slot_rem(_r->slot, _r);
		free_urecord(_r);
		_d->users--; /* FIXME */
	}
		
}


int timer_udomain(udomain_t* _d)
{
	struct urecord* ptr, *t;

	lock_udomain(_d);

	ptr = _d->d_ll.first;

	while(ptr) {
		if (timer_urecord(ptr) < 0) {
			LOG(L_ERR, "timer_udomain(): Error in timer_urecord\n");
			unlock_udomain(_d);
			return -1;
		}
		
		     /* Remove the entire record
		      * if it is empty
		      */
		if (ptr->contacts == 0) {
			t = ptr;
			ptr = ptr->d_ll.next;
			mem_delete_urecord(_d, t);
		} else {
			ptr = ptr->d_ll.next;
		}
	}
	
	unlock_udomain(_d);
/*	process_del_list(_d->name); */
/*	process_ins_list(_d->name); */
	return 0;
}


/*
 * Get lock
 */
void lock_udomain(udomain_t* _d)
{
	lock_get(&_d->lock);
}


/*
 * Release lock
 */
void unlock_udomain(udomain_t* _d)
{
	lock_release(&_d->lock);
}


/*
 * Create and insert a new record
 */
int insert_urecord(udomain_t* _d, str* _aor, struct urecord** _r)
{
	if (mem_insert_urecord(_d, _aor, _r) < 0) {
		LOG(L_ERR, "insert_urecord(): Error while inserting record\n");
		return -1;
	}
	return 0;
}


/*
 * Obtain a urecord pointer if the urecord exists in domain
 */
int get_urecord(udomain_t* _d, str* _aor, struct urecord** _r)
{
	int sl, i;
	urecord_t* r;

	sl = hash_func(_d, (unsigned char*)_aor->s, _aor->len);

	r = _d->table[sl].first;

	for(i = 0; i < _d->table[sl].n; i++) {
		if ((r->aor.len == _aor->len) && !memcmp(r->aor.s, _aor->s, _aor->len)) {
			*_r = r;
			return 0;
		}

		r = r->s_ll.next;
	}

	return 1;   /* Nothing found */
}


/*
 * Delete a urecord from domain
 */
int delete_urecord(udomain_t* _d, str* _aor)
{
	struct ucontact* c, *t;
	struct urecord* r;

	if (get_urecord(_d, _aor, &r) > 0) {
		return 0;
	}
		
	c = r->contacts;
	while(c) {
		t = c;
		c = c->next;
		if (delete_ucontact(r, t) < 0) {
			LOG(L_ERR, "delete_urecord(): Error while deleting contact\n");
			return -1;
		}
	}
	release_urecord(r);
	return 0;
}
