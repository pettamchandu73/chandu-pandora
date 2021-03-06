/*
 * $Id: domain_mod.h,v 1.1.1.1 2006/08/31 22:40:52 hari Exp $
 *
 * Domain module headers
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
 */


#ifndef DOMAIN_MOD_H
#define DOMAIN_MOD_H


#include "../../db/db.h"
#include "../../str.h"


/*
 * Constants
 */
#define HASH_SIZE 128

/*
 * Type definitions
 */
struct domain_list {
	str domain;
	struct domain_list *next;
};

/*
 * Module parameters variables
 */
extern int db_mode;             /* Database usage mode: 0 = no cache, 1 = cache */
extern str domain_table;	/* Domain table name */
extern str domain_col;   	/* Domain column name */


/*
 * Other module variables
 */
extern struct domain_list **hash_table_1; /* Hash table for domains */
extern struct domain_list **hash_table_2; /* Hash table for domains */
extern struct domain_list ***hash_table;  /* Current hash table */


#endif /* DOMAIN_MOD_H */
