/* 
 * $Id: val.c,v 1.1.1.1 2006/08/31 22:40:52 hari Exp $ 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "../../dprint.h"
#include "utils.h"
#include "val.h"


/*
 * Convert a string to integer
 */
static inline int str2int(const char* _s, int* _v)
{

	long tmp;

	if (!_s || !_v) {
	       LOG(L_ERR, "str2int: Invalid parameter value\n");
	       return -1;
	}

	tmp = strtoul(_s, 0, 10);
	if ((tmp == ULONG_MAX && errno == ERANGE) || 
	    (tmp < INT_MIN) || (tmp > UINT_MAX)) {
		printf("str2int: Value out of range\n");
		return -1;
	}

	*_v = (int)tmp;
	return 0;
}


/*
 * Convert a string to double
 */
static inline int str2double(const char* _s, double* _v)
{
	if ((!_s) || (!_v)) {
		LOG(L_ERR, "str2double(): Invalid parameter value\n");
		return -1;
	}

	*_v = atof(_s);
	return 0;
}


/* 
 * Convert a string to time_t
 */
static inline int str2time(const char* _s, time_t* _v)
{
	if ((!_s) || (!_v)) {
		LOG(L_ERR, "str2time(): Invalid parameter value\n");
		return -1;
	}

	*_v = mysql2time(_s);
	return 0;
}


/*
 * Convert an integer to string
 */
static inline int int2str(int _v, char* _s, int* _l)
{
	int ret;

	if ((!_s) || (!_l) || (!*_l)) {
		LOG(L_ERR, "int2str(): Invalid parameter value\n");
		return -1;
	}

	ret = snprintf(_s, *_l, "%-d", _v);
	if (ret < 0 || ret >= *_l) {
		LOG(L_ERR, "int2str: Error in sprintf\n");
		return -1;
	}
	*_l = ret;

	return 0;
}


/*
 * Convert a double to string
 */
static inline int double2str(double _v, char* _s, int* _l)
{
	int ret;

	if ((!_s) || (!_l) || (!*_l)) {
		LOG(L_ERR, "double2str(): Invalid parameter value\n");
		return -1;
	}

	ret = snprintf(_s, *_l, "%-10.2f", _v);
	if (ret < 0 || ret >= *_l) {
		LOG(L_ERR, "double2str: Error in snprintf\n");
		return -1;
	}
	*_l = ret;

	return 0;
}


/*
 * Convert time_t to string
 */
static inline int time2str(time_t _v, char* _s, int* _l)
{
	int l;

	if ((!_s) || (!_l) || (*_l < 2))  {
		LOG(L_ERR, "Invalid parameter value\n");
		return -1;
	}

	*_s++ = '\'';
	l = time2mysql(_v, _s, *_l - 1);
	*(_s + l) = '\'';
	*_l = l + 2;
	return 0;
}

/*
 * Does not copy strings
 */
int str2val(db_type_t _t, db_val_t* _v, const char* _s, int _l)
{
	if (!_v) {
		LOG(L_ERR, "str2val(): Invalid parameter value\n");
		return -1;
	}

	if (!_s) {
		memset(_v, 0, sizeof(db_val_t));
		VAL_TYPE(_v) = _t;
		VAL_NULL(_v) = 1;
		return 0;
	}
	VAL_NULL(_v) = 0;

	switch(_t) {
	case DB_INT:
		if (str2int(_s, &VAL_INT(_v)) < 0) {
			LOG(L_ERR, "str2val(): Error while converting integer value from string\n");
			return -2;
		} else {
			VAL_TYPE(_v) = DB_INT;
			return 0;
		}
		break;

	case DB_BITMAP:
		if (str2int(_s, &VAL_INT(_v)) < 0) {
			LOG(L_ERR, "str2val(): Error while converting bitmap value from string\n");
			return -3;
		} else {
			VAL_TYPE(_v) = DB_BITMAP;
			return 0;
		}
		break;
	
	case DB_DOUBLE:
		if (str2double(_s, &VAL_DOUBLE(_v)) < 0) {
			LOG(L_ERR, "str2val(): Error while converting double value from string\n");
			return -4;
		} else {
			VAL_TYPE(_v) = DB_DOUBLE;
			return 0;
		}
		break;

	case DB_STRING:
		VAL_STRING(_v) = _s;
		VAL_TYPE(_v) = DB_STRING;
		return 0;

	case DB_STR:
		VAL_STR(_v).s = (char*)_s;
		VAL_STR(_v).len = _l;
		VAL_TYPE(_v) = DB_STR;
		return 0;

	case DB_DATETIME:
		if (str2time(_s, &VAL_TIME(_v)) < 0) {
			LOG(L_ERR, "str2val(): Error while converting datetime value from string\n");
			return -5;
		} else {
			VAL_TYPE(_v) = DB_DATETIME;
			return 0;
		}
		break;

	case DB_BLOB:
		VAL_BLOB(_v).s = (char*)_s;
		VAL_BLOB(_v).len = _l;
		VAL_TYPE(_v) = DB_BLOB;
		return 0;
	}
	return -6;
}


/*
 * Used when converting result from a query
 */
int val2str(MYSQL* _c, db_val_t* _v, char* _s, int* _len)
{
	int l;
	char* old_s;

	if (!_c || !_v || !_s || !_len || !*_len) {
		LOG(L_ERR, "val2str(): Invalid parameter value\n");
		return -1;
	}

	if (VAL_NULL(_v)) {
		if (*_len < sizeof("NULL")) {
			LOG(L_ERR, "val2str: Buffer too small\n");
			return -1;
		}
		*_len = snprintf(_s, *_len, "NULL");
		return 0;
	}
	
	switch(VAL_TYPE(_v)) {
	case DB_INT:
		if (int2str(VAL_INT(_v), _s, _len) < 0) {
			LOG(L_ERR, "val2str(): Error while converting string to int\n");
			return -2;
		} else {
			return 0;
		}
		break;

	case DB_BITMAP:
		if (int2str(VAL_BITMAP(_v), _s, _len) < 0) {
			LOG(L_ERR, "val2str(): Error while converting string to int\n");
			return -3;
		} else {
			return 0;
		}
		break;

	case DB_DOUBLE:
		if (double2str(VAL_DOUBLE(_v), _s, _len) < 0) {
			LOG(L_ERR, "val2str(): Error while converting string to double\n");
			return -4;
		} else {
			return 0;
		}
		break;

	case DB_STRING:
		l = strlen(VAL_STRING(_v));
		if (*_len < (l * 2 + 3)) {
			LOG(L_ERR, "val2str(): Destination buffer too short\n");
			return -5;
		} else {
			old_s = _s;
			*_s++ = '\'';
			_s += mysql_real_escape_string(_c, _s, VAL_STRING(_v), l);
			*_s++ = '\'';
			*_s = '\0'; /* FIXME */
			*_len = _s - old_s;
			return 0;
		}
		break;

	case DB_STR:
		l = VAL_STR(_v).len;
		if (*_len < (l * 2 + 3)) {
			LOG(L_ERR, "val2str(): Destination buffer too short\n");
			return -6;
		} else {
			old_s = _s;
			*_s++ = '\'';
			_s += mysql_real_escape_string(_c, _s, VAL_STR(_v).s, l);
			*_s++ = '\'';
			*_s = '\0';
			*_len = _s - old_s;
			return 0;
		}
		break;

	case DB_DATETIME:
		if (time2str(VAL_TIME(_v), _s, _len) < 0) {
			LOG(L_ERR, "val2str(): Error while converting string to time_t\n");
			return -7;
		} else {
			return 0;
		}
		break;

	case DB_BLOB:
		l = VAL_BLOB(_v).len;
		if (*_len < (l * 2 + 3)) {
			LOG(L_ERR, "val2str(): Destination buffer too short\n");
			return -8;
		} else {
			old_s = _s;
			*_s++ = '\'';
			_s += mysql_escape_string(_s, VAL_STR(_v).s, l);
			*_s++ = '\'';
			*_s = '\0';
			*_len = _s - old_s;
			return 0;
		}			
		break;

	default:
		DBG("val2str(): Unknown data type\n");
		return -9;
	}
	/*return -8; --not reached*/
}
