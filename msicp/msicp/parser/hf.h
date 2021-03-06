/* 
 * $Id: hf.h,v 1.1.1.1 2006/08/31 22:40:56 hari Exp $ 
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
 * ---------
 * 2003-02-28 scratchpad compatibility abandoned (jiri)
 * 2003-01-27 next baby-step to removing ZT - PRESERVE_ZT (jiri)
 * 2003-05-01 HDR_ACCEPT added (janakj)
 */


#ifndef HF_H
#define HF_H

#include "../str.h"
#include "../comp_defs.h"


/* Header types and flags */
#define HDR_EOH         	     -1   /* End of header found */
#define HDR_ERROR                     0   /* Error while parsing */
#define HDR_VIA                       1   /* Via header field */
#define HDR_VIA1                      1   /* First Via header field */
#define HDR_VIA2               (1 <<  1)  /* only used as flag*/
#define HDR_TO                 (1 <<  2)  /* To header field */
#define HDR_FROM               (1 <<  3)  /* From header field */
#define HDR_CSEQ               (1 <<  4)  /* CSeq header field */
#define HDR_CALLID             (1 <<  5)  /* Call-Id header field */
#define HDR_CONTACT            (1 <<  6)  /* Contact header field */
#define HDR_MAXFORWARDS        (1 <<  7)  /* MaxForwards header field */
#define HDR_ROUTE              (1 <<  8)  /* Route header field */
#define HDR_RECORDROUTE        (1 <<  9)  /* Record-Route header field */
#define HDR_CONTENTTYPE        (1 << 10)  /* Content-Type header field */
#define HDR_CONTENTLENGTH      (1 << 11)  /* Content-Length header field */
#define HDR_AUTHORIZATION      (1 << 12)  /* Authorization header field */
#define HDR_EXPIRES            (1 << 13)  /* Expires header field */
#define HDR_PROXYAUTH          (1 << 14)  /* Proxy-Authorization hdr field */
#define HDR_SUPPORTED          (1 << 15)  /* Supported  header field */
#define HDR_PROXYREQUIRE       (1 << 16)  /* Proxy-Require header field */
#define HDR_UNSUPPORTED        (1 << 17)  /* Unsupported header field */
#define HDR_ALLOW              (1 << 18)  /* Allow header field */
#define HDR_EVENT              (1 << 19)  /* Event header field */
#define HDR_ACCEPT             (1 << 20)  /* Accept header field */
#define HDR_ACCEPTLANGUAGE     (1 << 21)  /* Accept-Language header field */
#define HDR_ORGANIZATION       (1 << 22)  /* Organization header field */
#define HDR_PRIORITY           (1 << 23)  /* Priority header field */
#define HDR_SUBJECT            (1 << 24)  /* Subject header field */
#define HDR_USERAGENT          (1 << 25)  /* User-Agent header field */
#define HDR_ACCEPTDISPOSITION  (1 << 26)  /* Accept-Disposition hdr field */
#define HDR_CONTENTDISPOSITION (1 << 27)  /* Content-Disposition hdr field */
#define HDR_DIVERSION          (1 << 28)  /* Diversion header field */
#define HDR_RPID               (1 << 29)  /* Remote-Party-ID header field */
#define HDR_OTHER              (1 << 30)  /* Some other header field */


/* returns true if the header links allocated memory on parse field */
#define hdr_allocs_parse( _hdr ) \
	(((_hdr)->type)&(HDR_VIA|HDR_TO|HDR_FROM|HDR_CONTACT|HDR_ROUTE|\
		HDR_RECORDROUTE|HDR_AUTHORIZATION|HDR_EXPIRES|HDR_PROXYAUTH|\
		HDR_EVENT|HDR_ACCEPT|HDR_CONTENTDISPOSITION|HDR_DIVERSION|HDR_RPID))



/* 
 * Format: name':' body 
 */
struct hdr_field {   
	int type;               /* Header field type */
	str name;               /* Header field name */
	str body;               /* Header field body (may not include CRLF) */
	int len;		/* length from hdr start until EoHF (incl.CRLF) */
	void* parsed;           /* Parsed data structures */
	struct hdr_field* next; /* Next header field in the list */
};


/* frees a hdr_field structure,
 * WARNING: it frees only parsed (and not name.s, body.s)
 */
void clean_hdr_field(struct hdr_field* hf);


/* frees a hdr_field list,
 * WARNING: frees only ->parsed and ->next
 */
void free_hdr_field_lst(struct hdr_field* hf);

void dump_hdr_field( struct hdr_field* hf );

#endif /* HF_H */
