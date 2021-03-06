/* 
 * $Id: db_con.c,v 1.1.1.1 2006/08/31 22:40:52 hari Exp $ 
 *
 * Database connection related functions
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


#include <string.h>
#include "../../db/db.h"
#include "../../dprint.h"
#include "../../mem/mem.h"


/*
 * Store name of table that will be used by
 * subsequent database functions
 */
int use_table(db_con_t* _h, const char* _t)
{
	if ((!_h) || (!_t)) {
		LOG(L_ERR, "use_table(): Invalid parameter value\n");
		return -1;
	}

	CON_TABLE(_h) = _t;
	return 0;
}
