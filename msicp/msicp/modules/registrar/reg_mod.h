/* 
 * $Id: reg_mod.h,v 1.1.1.1 2006/08/31 22:40:49 hari Exp $ 
 *
 * registrar module interface
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


#ifndef REG_MOD_H
#define REG_MOD_H

#include "../../parser/msg_parser.h"
#include "../../qvalue.h"
#include "../../usr_avp.h"
#include "../usrloc/usrloc.h"

extern int default_expires;
extern qvalue_t default_q;
extern int append_branches;
extern int case_sensitive;
extern int desc_time_order;
extern int nat_flag;
extern int min_expires;
extern int max_expires;
extern int received_avp;
extern int use_domain;
extern str realm_prefix;
extern float def_q;

extern str rcv_param;
extern int rcv_avp_no;
extern int max_contacts;
extern int retry_after;

usrloc_api_t ul;  /* Structure containing pointers to usrloc functions */

extern int (*sl_reply)(struct sip_msg* _m, char* _s1, char* _s2);

#endif /* REG_MOD_H */
