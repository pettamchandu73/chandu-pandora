/*
 * $Id: digest.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $
 *
 * Digest credentials parser interface
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


#ifndef DIGEST_H
#define DIGEST_H

#include "digest_parser.h"
#include "../hf.h"         /* struct hdr_field */
#include "../msg_parser.h"


typedef struct auth_body {
	     /* This is pointer to header field containing
	      * parsed authorized digest credentials. This
	      * pointer is set in sip_msg->{authorization,proxy_auth}
	      * hooks.
	      *
	      * This is necessary for functions called after
	      * {www,proxy}_authorize, these functions need to know
	      * which credentials are authorized and they will simply
	      * look into 
	      * sip_msg->{authorization,proxy_auth}->parsed->authorized
	      */
	struct hdr_field* authorized;
	dig_cred_t digest;           /* Parsed digest credentials */
	unsigned char stale;         /* Flag is set if nonce is stale */
} auth_body_t;


/*
 * Errors returned by check_dig_cred
 */
typedef enum dig_err {
	E_DIG_OK = 0,        /* Everything is OK */
	E_DIG_USERNAME  = 1, /* Username missing */
	E_DIG_REALM = 2,     /* Realm missing */
	E_DIG_NONCE = 4,     /* Nonce value missing */
	E_DIG_URI = 8,       /* URI missing */
	E_DIG_RESPONSE = 16, /* Response missing */
	E_DIG_CNONCE = 32,   /* CNONCE missing */
	E_DIG_NC = 64,       /* Nonce-count missing */
	E_DIG_DOMAIN = 128   /* Username domain != realm */
} dig_err_t;


/*
 * Parse digest credentials
 */
int parse_credentials(struct hdr_field* _h);


/*
 * Free all memory associated with parsed
 * structures
 */
void free_credentials(auth_body_t** _b);


/*
 * Print dig_cred structure to stdout
 */
void print_cred(dig_cred_t* _c);


/*
 * Mark credentials as authorized
 */
int mark_authorized_cred(struct sip_msg* _m, struct hdr_field* _h);


/*
 * Get pointer to authorized credentials
 */
int get_authorized_cred(struct hdr_field* _f, struct hdr_field** _h);


/*
 * Check if credentials are correct
 * (check of semantics)
 */
dig_err_t check_dig_cred(dig_cred_t* _c);


#endif /* DIGEST_H */
