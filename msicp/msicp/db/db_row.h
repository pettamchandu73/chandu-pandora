/* 
 * $Id: db_row.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $ 
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


#ifndef DB_ROW_H
#define DB_ROW_H

#include "db_val.h"


/*
 * Structure holding result of query_table function (ie. table row)
 */
typedef struct db_row {
	db_val_t* values;  /* Columns in the row */
	int n;             /* Number of columns in the row */
} db_row_t;


#define ROW_VALUES(rw) ((rw)->values)
#define ROW_N(rw)      ((rw)->n)


#endif /* DB_ROW_H */
