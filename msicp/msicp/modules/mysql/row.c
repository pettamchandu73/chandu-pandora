/* 
 * $Id: row.c,v 1.1.1.1 2006/08/31 22:40:52 hari Exp $ 
 *
 * MySQL module row related functions
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


#include "../../dprint.h"
#include "../../mem/mem.h"
#include <mysql/mysql.h>
#include "val.h"
#include "my_con.h"
#include "row.h"


/*
 * Convert a row from result into db API representation
 */
int convert_row(db_con_t* _h, db_res_t* _res, db_row_t* _r)
{
	unsigned long* lengths;
	int i;

	if ((!_h) || (!_res) || (!_r)) {
		LOG(L_ERR, "convert_row(): Invalid parameter value\n");
		return -1;
	}

	ROW_VALUES(_r) = (db_val_t*)pkg_malloc(sizeof(db_val_t) * RES_COL_N(_res));
	ROW_N(_r) = RES_COL_N(_res);
	if (!ROW_VALUES(_r)) {
		LOG(L_ERR, "convert_row(): No memory left\n");
		return -1;
	}

	lengths = mysql_fetch_lengths(CON_RESULT(_h));

	for(i = 0; i < RES_COL_N(_res); i++) {
		if (RES_TYPES(_res) && str2val(RES_TYPES(_res)[i], &(ROW_VALUES(_r)[i]), 
			    ((MYSQL_ROW)CON_ROW(_h))[i], lengths[i]) < 0) {
			LOG(L_ERR, "convert_row(): Error while converting value\n");
			free_row(_r);
			return -3;
		}
	}
	return 0;
}


/*
 * Release memory used by row
 */
int free_row(db_row_t* _r)
{
	if (!_r) {
		LOG(L_ERR, "free_row(): Invalid parameter value\n");
		return -1;
	}

	if (ROW_VALUES(_r)) pkg_free(ROW_VALUES(_r));
	return 0;
}

