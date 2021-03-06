/* 
 * $Id: db_con.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $ 
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#ifndef DB_CON_H
#define DB_CON_H


/*
 * This structure represents a database connection
 * and pointer to this structure is used as a connection
 * handle
 */
typedef struct {
	const char* table;     /* Default table to use */
	unsigned long tail;    /* Variable length tail*/
							/* database module specific */
	char url[255];
	char *current_url;
	char *primary_url;
	char *alternate_url;
} db_con_t;


#define CON_TABLE(cn)      ((cn)->table)
#define CON_TAIL(cn)       ((cn)->tail)


#endif /* DB_CON_H */
