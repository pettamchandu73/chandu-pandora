/*
 * $Id: parse_contact.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $
 *
 * Contact header field body parser
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
 *
 * History:
 * -------
 *  2003-03-25 Adapted to use new parameter parser (janakj)
 */


#ifndef PARSE_CONTACT_H
#define PARSE_CONTACT_H

#include <stdio.h>
#include "../hf.h"
#include "../../str.h"
#include "../msg_parser.h"
#include "contact.h"


typedef struct contact_body {
	unsigned char star;    /* Star contact */
	contact_t* contacts;   /* List of contacts */
} contact_body_t;


/*
 * Parse contact header field body
 */
int parse_contact(struct hdr_field* _h);


/*
 * Free all memory
 */
void free_contact(contact_body_t** _c);


/*
 * Print structure, for debugging only
 */
void print_contact(FILE* _o, contact_body_t* _c);


/*
 * Contact header field iterator, returns next contact if any, it doesn't
 * parse message header if not absolutely necessary
 */
int contact_iterator(contact_t** c, struct sip_msg* msg, contact_t* prev);


#endif /* PARSE_CONTACT_H */
