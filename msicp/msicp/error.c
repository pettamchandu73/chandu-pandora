/*
 * $Id: error.c,v 1.1.1.1 2006/08/31 22:40:48 hari Exp $
 *
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
 * --------
 * 2003-04-04 phrase length corrected not to include trailer 0 (jiri)
 */


#include <stdio.h>
#include "error.h"
#include "str.h"
#include "parser/msg_parser.h"
#include "mem/mem.h"

/* current function's error; */
int ser_error=-1;
/* previous error */
int prev_ser_error=-1;

int err2reason_phrase( 
	int ser_error,  /* current internal msicp error */
	int *sip_error,  /* the sip error code to which msicp 	
					    msicp error will be turned */
	char *phrase,    /* resulting error text */
	int etl, 		/* error text buffer length */
	char *signature ) /* extra text to be appended */
{

	char *error_txt;

	switch( ser_error ) {
		case E_SEND:
			error_txt="Unfortunately error on sending to next hop occurred";
			*sip_error=-ser_error;
			break;
		case E_BAD_ADDRESS:
			error_txt="Unresolvable destination";
			*sip_error=-ser_error;
			break;
		case E_BAD_REQ:
			error_txt="Bad Request";
			*sip_error=-ser_error;
			break;
		case E_BAD_URI:
			error_txt="Sorry, but its a bad URI";
			*sip_error=-ser_error;
			break;
		case E_BAD_TUPEL:
			error_txt="Transaction tuple incomplete";
			*sip_error=-E_BAD_REQ;
			break;
		case E_BAD_TO:
			error_txt="Bad To";
			*sip_error=-E_BAD_REQ;
			break;
		case E_EXEC:
			error_txt="Error in external logic";
			*sip_error=-E_BAD_SERVER;
			break;
		case E_TOO_MANY_BRANCHES:
			error_txt="Too many branches";
			*sip_error=-E_BAD_SERVER;
			break;

	        case E_Q_INV_CHAR:
			error_txt="Invalid character in q parameter";
			*sip_error=-E_BAD_REQ;
			break;

	        case E_Q_EMPTY:
			error_txt="Empty q parameter";
			*sip_error=-E_BAD_REQ;
			break;;

	        case E_Q_TOO_BIG:
			error_txt="q parameter too big";
			*sip_error=-E_BAD_REQ;
			break;

	        case E_Q_DEC_MISSING:
			error_txt="Decimal part missing in q";
			*sip_error=-E_BAD_REQ;
			break;

		case E_OUT_OF_MEM:
		/* dont disclose lack of mem in release mode */
#ifdef DEBUG
			error_txt="Excuse me I ran out of memory";
			*sip_error=500;
			break;
#endif
		default:
			error_txt="Server error while processing request";
			*sip_error=500;
			break;
	}
	return snprintf( phrase, etl, "%s (%d/%s)", error_txt, 
		-ser_error, signature );
}

char *error_text( int code )
{
	switch(code) {

		case 100: return "Trying";
		case 180: return "Ringing";
		case 181: return "Call is Being Forwarded";
		case 182: return "Queued";
		case 183: return "Session Progress";

		case 200: return "OK";

		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Moved Temporarily";
		case 305: return "Use Proxy";
		case 380: return "Alternative Service";

		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy authentication Required";
		case 408: return "Request Timeout";
		case 410: return "Gone";
		case 413: return "Request Entity Too Large";
		case 414: return "Request-URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Unsupported URI Scheme";
		case 417: return "Bad Extension";
		case 421: return "Extension Required";
		case 423: return "Interval Too Brief";
		case 480: return "Temporarily Unavailable";
		case 481: return "Call/Transaction Does not Exist";
		case 482: return "Loop Detected";
		case 483: return "Too Many Hops";
		case 484: return "Address Incomplete";
		case 485: return "Ambiguous";
		case 486: return "Busy Here";
		case 487: return "Request Terminated";
		case 488: return "Not Acceptable Here";
		case 491: return "Request Pending";
	
		case 500: return "Server Internal Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Server Time-out";
		case 505: return "Version not Supported";
		case 513: return "Message Too Large";

		case 600: return "Busy Everywhere";
		case 603: return "Decline";
		case 604: return "Does not Exist Anywhere";
		case 606: return "Not Acceptable";

	}

	if (code>=600) return "Global Failure";
	else if (code>=500) return "Server Failure";
	else if (code>=400) return "Request Failure";
	else if (code>=300) return "Redirection";
	else if (code>=200) return "Successful";
	else if (code>=100) return "Provisional";
	else return "Unspecified";
}

void get_reply_status( str *status, struct sip_msg *reply, int code )
{
	str phrase;

	status->s=0;

	if (reply==0) {
		LOG(L_CRIT, "BUG: get_reply_status called with 0 msg\n");
		return;
	}

	if (reply==FAKED_REPLY) {
		phrase.s=error_text(code);
		phrase.len=strlen(phrase.s);
	} else {
		phrase=reply->first_line.u.reply.reason;
	}
	status->len=phrase.len+3/*code*/+1/*space*/; 
	status->s=pkg_malloc(status->len+1/*ZT */);
	if (!status->s) {
		LOG(L_ERR, "ERROR: get_reply_status: no mem\n");
		return;
	}
	status->s[3]=' ';
	status->s[2]='0'+code % 10; code=code/10;
	status->s[1]='0'+code% 10; code=code/10;
	status->s[0]='0'+code % 10;
	memcpy(&status->s[4], phrase.s, phrase.len);
	status->s[status->len]=0;
}
