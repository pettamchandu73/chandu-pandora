/* 
 * $Id: db_val.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $ 
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


#ifndef DB_VAL_H
#define DB_VAL_H

#include <time.h>
#include "../str.h"


/*
 * Accepted value types
 */
typedef enum {
	DB_INT,        /* 32-bit integer */
        DB_DOUBLE,     /* double data type */
	DB_STRING,     /* Zero-terminated string */
	DB_STR,        /* str structure */
	DB_DATETIME,   /* Date and time */
	DB_BLOB,       /* Large binary object */
	DB_BITMAP      /* Bitmap of flags */
} db_type_t;


/*
 * Column value structure
 */
typedef struct {
	db_type_t type;                /* Type of the value */
	int nul;                       /* Means that the column in database
					* has no value 
					*/
	union {
		int           int_val;    /* integer value */
		double        double_val; /* double value */
		time_t        time_val;   /* unix time value */
		const char*   string_val; /* NULL terminated string */
		str           str_val;    /* str string value */
		str           blob_val;   /* Blob data */
		unsigned int  bitmap_val; /* Bitmap data type, 32 flags, should be enough */ 
	} val;                            /* union of all possible types */
} db_val_t;


/*
 * Useful macros for accessing attributes of db_val structure
 */
#define VAL_TYPE(dv)   ((dv)->type)
#define VAL_NULL(dv)   ((dv)->nul)
#define VAL_INT(dv)    ((dv)->val.int_val)
#define VAL_DOUBLE(dv) ((dv)->val.double_val)
#define VAL_TIME(dv)   ((dv)->val.time_val)
#define VAL_STRING(dv) ((dv)->val.string_val)
#define VAL_STR(dv)    ((dv)->val.str_val)
#define VAL_BLOB(dv)   ((dv)->val.blob_val)
#define VAL_BITMAP(dv) ((dv)->val.bitmap_val)


#endif /* DB_VAL_H */
