/*
 * $Id: mf_funcs.h,v 1.1.1.1 2006/08/31 22:40:48 hari Exp $
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


#ifndef _MF_FUNCS_H
#define _MF_FUNCS_H


#include "../../parser/msg_parser.h"
#include "../../dprint.h"
#include "../../config.h"
#include "../../str.h"


int decrement_maxfwd( struct sip_msg* msg, int nr_val, str *str_val );
int add_maxfwd_header( struct sip_msg* msg , unsigned int val );
int is_maxfwd_present( struct sip_msg* msg, str *mf_value );

#endif

