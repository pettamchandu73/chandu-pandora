/*
 * $Id: timer.h,v 1.1.1.1 2006/08/31 22:40:45 hari Exp $
 *
 *
 * timer related functions
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



#ifndef timer_h
#define timer_h

typedef void (timer_function)(unsigned int ticks, void* param);


struct sr_timer{
	int id;
	timer_function* timer_f;
	void* t_param;
	unsigned int interval;
	
	unsigned int expires;
	
	struct sr_timer* next;
};



extern struct sr_timer* timer_list;



int init_timer();
void destroy_timer();
/*register a periodic timer;
 * ret: <0 on error*/
int register_timer(timer_function f, void* param, unsigned int interval);
unsigned int get_ticks();
void timer_ticker();

#endif
