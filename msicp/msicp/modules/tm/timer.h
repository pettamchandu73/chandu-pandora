/*
 * $Id: timer.h,v 1.1.1.1 2006/08/31 22:40:48 hari Exp $
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
 *  2003-09-12  timer_link.tg exists only if EXTRA_DEBUG (andrei)
 *  2004-02-13  timer_link.payload removed (bogdan)
 */


#ifndef _TIMER_H
#define _TIMER_H

#include "defs.h"

#include "lock.h"

/* timer timestamp value indicating a timer has been 
   deactivated and shall not be executed
*/
#define TIMER_DELETED	1



/* identifiers of timer lists;*/
/* fixed-timer retransmission lists (benefit: fixed timer$
   length allows for appending new items to the list as$
   opposed to inserting them which is costly */
enum lists
{
	FR_TIMER_LIST, FR_INV_TIMER_LIST,
	WT_TIMER_LIST,
	DELETE_LIST,
	RT_T1_TO_1, RT_T1_TO_2, RT_T1_TO_3, RT_T2,
	NR_OF_TIMER_LISTS
};

/* all you need to put a cell in a timer list
   links to neighbors and timer value */
typedef struct timer_link
{
	struct timer_link     *next_tl;
	struct timer_link     *prev_tl;
	volatile unsigned int  time_out;
	struct timer          *timer_list;
#ifdef EXTRA_DEBUG
	enum timer_groups  tg;
#endif
}timer_link_type ;


/* timer list: includes head, tail and protection semaphore */
typedef struct  timer
{
	struct timer_link  first_tl;
	struct timer_link  last_tl;
	ser_lock_t*        mutex;
	enum lists         id;
} timer_type;

/* transaction table */
struct timer_table
{
    /* table of timer lists */
    struct timer   timers[ NR_OF_TIMER_LISTS ];
};





extern int timer_group[NR_OF_TIMER_LISTS];
extern unsigned int timer_id2timeout[NR_OF_TIMER_LISTS];



struct timer_table * tm_init_timers();
void unlink_timer_lists();
void free_timer_table();
void init_timer_list( enum lists list_id);
void reset_timer_list( enum lists list_id);
/*void remove_timer_unsafe(  struct timer_link* tl ) ;
void add_timer_unsafe( struct timer*, struct timer_link*, unsigned int);
struct timer_link  *check_and_split_time_list( struct timer*, int);
*/

void reset_timer( struct timer_link* tl );
/* determine timer length and put on a correct timer list */
void set_timer( struct timer_link *new_tl, enum lists list_id, unsigned int* ext_timeout );
/* similar to set_timer, except it allows only one-time
   timer setting and all later attempts are ignored */
void set_1timer( struct timer_link *new_tl, enum lists list_id, unsigned int* ext_timeout );
/*void unlink_timers( struct cell *t );*/
void timer_routine(unsigned int, void*);


struct timer_table *get_timertable();

#endif
