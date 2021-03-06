/*
 * $Id: lock.h,v 1.1.1.1 2006/08/31 22:40:48 hari Exp $
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
 */
/*
 * History:
 * --------
 *  2003-03-17  converted to locking.h (andrei)
 *  2004-07-28  s/lock_set_t/gen_lock_set_t/ because of a type conflict
 *              on darwin (andrei)
 */

#include "defs.h"


#ifndef __lock_h
#define __lock_h

#include "../../dprint.h"
#include "../../locking.h"



#ifdef GEN_LOCK_T_PREFERED
#define ser_lock_t gen_lock_t
#else
/* typedef to structure we use for mutexing;
   currently, index to a semaphore set identifier now */
typedef struct {
	gen_lock_set_t* semaphore_set;
	int semaphore_index;
} ser_lock_t;
#endif


enum timer_groups {
	TG_FR,
	TG_WT,
	TG_DEL,
	TG_RT,
	TG_NR
};


/* extern ser_lock_t timer_group_lock[TG_NR]; */


#include "h_table.h"
#include "timer.h" 

/* Uni*x permissions for IPC */
#define IPC_PERMISSIONS 0666


int lock_initialize();
void lock_cleanup();

#ifdef DBG_LOCK
#define lock(_s) _lock( (_s), __FILE__, __FUNCTION__, __LINE__ )
#define unlock(_s) _unlock( (_s), __FILE__, __FUNCTION__, __LINE__ )
#else
#define lock(_s) _lock( (_s) )
#define unlock(_s) _unlock( (_s) )
#endif


int init_cell_lock( struct cell *cell );
int init_entry_lock( struct s_table* ht, struct entry *entry );


int release_cell_lock( struct cell *cell );
int release_entry_lock( struct entry *entry );
int release_timerlist_lock( struct timer *timerlist );



/* lock semaphore s */
#ifdef DBG_LOCK
static inline void _lock( ser_lock_t* s , char *file, char *function,
							unsigned int line )
#else
static inline void _lock( ser_lock_t* s )
#endif
{
#ifdef DBG_LOCK
	DBG("DEBUG: lock : entered from %s , %s(%d)\n", function, file, line );
#endif
#ifdef GEN_LOCK_T_PREFERED 
	lock_get(s);
#else
	lock_set_get(s->semaphore_set, s->semaphore_index);
#endif
}



#ifdef DBG_LOCK
static inline void _unlock( ser_lock_t* s, char *file, char *function,
		unsigned int line )
#else
static inline void _unlock( ser_lock_t* s )
#endif
{
#ifdef DBG_LOCK
	DBG("DEBUG: unlock : entered from %s, %s:%d\n", file, function, line );
#endif
#ifdef GEN_LOCK_T_PREFERED
	lock_release(s);
#else
	lock_set_release( s->semaphore_set, s->semaphore_index );
#endif
}

int init_timerlist_lock(  enum lists timerlist_id);


#endif

