#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include "fun.h"
#include "../../dset.h"
#include "../../parser/parse_from.h"
#include "../../mem/mem.h"
#include "../../ip_addr.h"
#include "../../data_lump_rpl.h"
#include <netinet/in.h>
#include "../../action.h"
#include "../../error.h"

extern int _nOrigSeq;
/*tulsi added for warm transfer with media pooling*/
extern int _nSupSeq;
extern int media_failover;

/*
 * Code from nathelper module. 
 *
 *
 * Extract URI from the Contact header field
 */

int get_direct_extension (char *,int);

static inline int
get_contact_uri(struct sip_msg* _m, struct sip_uri *uri, contact_t** _c)
{

    if ((parse_headers(_m, HDR_CONTACT, 0) == -1) || !_m->contact)
        return -1;
    if (!_m->contact->parsed && parse_contact(_m->contact) < 0) {
        LOG(L_ERR, "get_contact_uri: Error while parsing Contact body\n");
        return -1;
    }
    *_c = ((contact_body_t*)_m->contact->parsed)->contacts;
    if (*_c == NULL) {
        LOG(L_ERR, "get_contact_uri: Error while parsing Contact body\n");
        return -1;
    }
    if (parse_uri((*_c)->uri.s, (*_c)->uri.len, uri) < 0 || uri->host.len <= 0) {
        LOG(L_ERR, "get_contact_uri: Error while parsing Contact URI\n");
        return -1;
    }
    return 0;
}

int set_contact_username (struct sip_msg * msg, char * username) {
// Useless method
	int offset, len;
	char *buf = NULL;
	contact_t * c;
	struct lump* anchor;
	struct sip_uri uri;
	char uribuff[100];
	memset(uribuff, 0, sizeof(uribuff));

	if (get_contact_uri(msg, &uri, &c) == -1)
		return -1;
	memcpy(uribuff, c->uri.s, c->uri.len);

	offset = c->uri.s - msg->buf;

	anchor = del_lump(msg, offset, c->uri.len, HDR_CONTACT);

	if (anchor == 0)
		return -1;
	
	parse_uri(uribuff, strlen(uribuff), &uri);
	len = strlen(username) + (uribuff + strlen(uribuff) ) - (uri.user.s + uri.user.len );
	LOG(L_ERR, "Calculated len %d\n", len);

	buf = pkg_malloc(len);
	if (buf == NULL) {
		LOG(L_ERR, "ERROR: fix_nated_contact: out of memory\n");
		return -1;
	}
	
	snprintf(buf, len-1, "%.*s", strlen(username), username);
	strncpy(buf + strlen(username), uri.user.s + uri.user.len, len -  strlen(username));

	if (insert_new_lump_after(anchor, buf, len, HDR_CONTACT) == 0) {
		pkg_free(buf);
		return -1;
	}

	c->uri.s = buf;
	c->uri.len = len;

	return 1;
}
int parse_ws_uri_buffer(char * buffer, int nLength, struct ws_uri * wsuri) {

	memset(wsuri, 0, sizeof(struct ws_uri));

	snprintf(wsuri->parsed, sizeof(wsuri->parsed)-1, "%.*s", nLength, buffer);
	LOG(L_ERR, "[parse_ws_uri_buffer][uri]*-*-*%s*-*-*\n",wsuri->parsed);
	
	wsuri->type = wsuri->parsed;

	wsuri->command = strchr(wsuri->type, WS_SEPERATOR);
	if ( !wsuri->command ) {
		//Parse failed
                wsuri->type = NULL;
		return -1;
	}
	*wsuri->command = '\0'; // Till type
	wsuri->command++;

	wsuri->group = strchr(wsuri->command,WS_SEPERATOR);
	if(!wsuri->group){
		return -1;
	}
	*wsuri->group = '\0'; // Till here group
	wsuri->group++;
	
	wsuri->context = strchr(wsuri->group, WS_SEPERATOR);
	if ( !wsuri->context ) {
		return -1;
	}
	*wsuri->context = '\0'; // Till here context
	wsuri->context++;
		
	wsuri->token = strrchr(wsuri->context, WS_SEPERATOR);
	if ( !wsuri->token) {
		return -1;
	}
	*wsuri->token = '\0';// Till here token
	wsuri->token++;

	return 0;
}

int parse_ws_uri(struct sip_msg * msg, struct ws_uri * wsuri, char *d_uri,char *token) 
{
	struct sip_uri uri;
	char * pTemp;
	int dirTemp=0;
	int len=0;
	char extension[256] = "";
	struct to_body * from;
	memset(wsuri, 0, sizeof(struct ws_uri));

	strncpy(wsuri->parsed, d_uri, sizeof(wsuri->parsed)-1);
	LOG(L_ERR, "[parse_ws_uri][uri]*-*-*%s*-*-*\n",wsuri->parsed);
	/* If the uri absents any thing like (command||group||context,ex:cmd-2--0004567 return -1  Date:21/10/05 --Gopinath*/
    if (strstr (wsuri->parsed, "--"))  {
		LOG(L_ERR, "[parse_ws_uri][uri] Failed ..... *-*-*%s*-*-*\n",wsuri->parsed);
        strncpy(wsuri->parsed, "invtype",sizeof(wsuri->parsed)-1);
        wsuri->type = wsuri->parsed;
		return 1;
	}
	dirTemp = atoi(wsuri->parsed);
	len = strlen(wsuri->parsed);	
	bzero(extension, sizeof(extension));
	if ( len == 3 && dirTemp > 0 ) {
		strncpy(extension, wsuri->parsed,sizeof(extension)-1);
		LOG(L_ERR, "[parse_ws_uri][uri]*-*-*I RCVD 3 Digit Extension*-*-*\n");
		switch (dirTemp) {
			case 211:
				LOG (L_ERR,"211 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;	
			case 311:
                                LOG (L_ERR,"311 Feature\n");
				strncpy(wsuri->parsed, "dir-",sizeof(wsuri->parsed)-1);  //Type
                                break;
			case 411:
                                LOG (L_ERR,"411 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;
			case 511:
                                LOG (L_ERR,"511 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;
			case 611:
                                LOG (L_ERR,"611 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;
			case 711:
                                LOG (L_ERR,"711 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;
			case 811:
                                LOG (L_ERR,"811 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;
			case 911:
                                LOG (L_ERR,"911 Feature\n");
				strncpy(wsuri->parsed, "inv-",sizeof(wsuri->parsed)-1);
                                break;		
			default:
				strncpy(wsuri->parsed, "ext-",sizeof(wsuri->parsed)-1);  //Type
				break;
			
		}
		strcat(wsuri->parsed, extension); //Command
		strcat(wsuri->parsed, "-");
		strcat(wsuri->parsed, "0-"); /* Dummy group:- Girish*/
		if ( msg && msg->from && (!msg->from->parsed) ) {
			parse_from_header(msg);
		}
		from=get_from(msg);
		if (from == NULL) {
	            send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
	            return -1;
	       	}
		parse_uri(from->uri.s, from->uri.len, &uri);
		memset(extension,0x00,sizeof(extension));
		snprintf(extension,sizeof(extension)-1,"%.*s" ,uri.user.len, uri.user.s);
		LOG(L_ERR, "[parse_ws_uri][frombody] %s\n", extension);
		pTemp = strchr(extension, WS_SEPERATOR);
		if (NULL == pTemp) return -1;
		pTemp++;
		strcat(wsuri->parsed, pTemp); //Context
		strcat(wsuri->parsed, "-"); 
		strcat(wsuri->parsed,token); //Token
		LOG(L_ERR, "[parse_ws_uri] %s\n", wsuri->parsed);
	}	
	else if ( (len == 4||len ==5) && dirTemp > 0 ) {
		strncpy(extension, wsuri->parsed,sizeof(extension)-1);
		strncpy(wsuri->parsed, "ext-",sizeof(wsuri->parsed)-1); //Type
		strcat(wsuri->parsed, extension); //Command
		strcat(wsuri->parsed, "-");
		strcat(wsuri->parsed, "0-"); /* Dummy group:- Girish*/
		if ( !msg->from->parsed ) {
			parse_from_header(msg);
		}

		from=get_from(msg);
		if (from == NULL) {
	            send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		    return -1;
		}
		parse_uri(from->uri.s, from->uri.len, &uri);
	
		memset(extension,0x00,sizeof(extension));
		snprintf(extension,sizeof(extension)-1,"%.*s" ,uri.user.len, uri.user.s);
		LOG(L_ERR, "[parse_ws_uri][frombody] %s\n", extension);
		pTemp = strchr(extension, WS_SEPERATOR);
		if (NULL == pTemp) return -1;
		pTemp++;
		strcat(wsuri->parsed, pTemp); //Context
		strcat(wsuri->parsed, "-"); 
		strcat(wsuri->parsed,token); //Token
		LOG(L_ERR, "[parse_ws_uri] %s\n", wsuri->parsed);
	}
	else if (!strncmp(wsuri->parsed,"mwi",3)) {
		strcat(wsuri->parsed, "-");
                strcat(wsuri->parsed, "0-"); /* Dummy group:- Ramu */
                if ( !msg->from->parsed ) {
                        parse_from_header(msg);
                }
                from=get_from(msg);
                if (from == NULL) {
                    send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
                    return -1;
                }
                parse_uri(from->uri.s, from->uri.len, &uri);
				memset(extension,0x00,sizeof(extension));
				snprintf(extension,sizeof(extension)-1,"%.*s" ,uri.user.len, uri.user.s);
                LOG(L_ERR, "[parse_ws_uri][frombody] %s\n", extension);
                pTemp = strchr(extension, WS_SEPERATOR);
                if (NULL == pTemp) return -1;
                pTemp++;
                strcat(wsuri->parsed, pTemp); //Context
                strcat(wsuri->parsed, "-"); 
                strcat(wsuri->parsed,token); //Token
                LOG(L_ERR, "[parse_ws_uri] %s\n", wsuri->parsed);
	}

	wsuri->type = wsuri->parsed;
	
	wsuri->command = strchr(wsuri->type, WS_SEPERATOR);
	if ( !wsuri->command ) {
		//Parse failed
		wsuri->type = NULL;
		return -1;
	}
	*wsuri->command = '\0'; // Till here type
	wsuri->command++;
	
	wsuri->group = strchr(wsuri->command,WS_SEPERATOR);
	if(!wsuri->group){
		return -1;
	}
	*wsuri->group = '\0'; // Till here group
	wsuri->group++;
	
	wsuri->context = strchr(wsuri->group, WS_SEPERATOR);
	if ( !wsuri->context ) {
		return -1;
	}
	*wsuri->context = '\0'; // Till here context
	wsuri->context++;
	
	wsuri->token = strrchr(wsuri->context, WS_SEPERATOR);
	if ( !wsuri->token) {
		return -1;
	}
	*wsuri->token = '\0';// Till here token
	wsuri->token++;
	return 1;
}

/*Modified function for extended group -------*/
#if 0
int worksmart_process_acd_f (struct sip_msg * msg, db_con_t * h, db_func_t ws_dbf) 
{

//	db_func_t ws_dbf;
	int nAcdEnabled = 0, nExtendedGroup = 0, nGroup = 0, nSiteId;
	db_res_t *res=NULL;
	db_row_t row;
	char *pTemp;
	char query[500], newuri[100] ;
	struct ws_uri wsuri;

//	cmd-1-worksmart-com-token
#if 0
	sprintf(query, "Contact: <sip:%s@%s:%d>\r\n", "set-by-me", ip_addr2a(&msg->rcv.src_ip), msg->rcv.src_port);
	LOG(L_ERR, "[worksmart_process_acd] Here\n");	
#else
	// uri: cmd-1-context-token

	if ( parse_ws_uri(msg, &wsuri, 0) < 0 ) {
		LOG(L_ERR, "[worksmart_process_acd] parse_ws_uri failed\n");
		return -1;
	}
	

	if ( strcmp(wsuri.type, "cmd") ) {
		LOG(L_ERR, "[worksmart_process_acd] Invalid uri for ACD\n");
		return -1;
	}
	
	pTemp = strchr (wsuri.context, WS_SEPERATOR);
	LOG(L_ERR, "[worksmart_process_acd] Command Checking for Extended-1\n");
	if ( !pTemp )
		return -1;
	LOG (L_ERR,"[worksmart-acd] Context %s \n",pTemp);
	pTemp++;
	LOG (L_ERR,"[worksmart-acd] Context %s \n",pTemp);

	LOG(L_ERR, "[worksmart_process_acd] Before Query\n");
	wsuri.context = pTemp;
	fprintf (stderr ,"Context %s pTemp %s \n",wsuri.context ,pTemp);
	sprintf (query, "SELECT siteinfo_new.siteid, extensiongroupcode, acdenabled, extendedgroup "
			 "  FROM extendedpbxivrcommand, special, siteinfo_new"
			 " WHERE special.siteid = extendedpbxivrcommand.siteid"
			 "   AND special.code = extensiongroupcode"
			 "   AND special.siteid = siteinfo_new.siteid"
			 "   AND replace(replace(sitename,'.','-'),'_','-') = '%s'"
			 "   AND command = %s AND groupcode = %s", wsuri.context, wsuri.command, wsuri.group);
	ws_dbf.raw_query (h, query, &res);
	if(res==NULL){
		LOG(L_ERR,"[worksmart_route_to_ivr2]'res' is NULL");
		return -1;
	}
	LOG(L_ERR, "[worksmart_process_acd] After Query\n");
	if ( RES_ROW_N(res) < 1 ) {
		ws_dbf.free_result (h, res);
		return -1;
	}

	row = RES_ROWS (res)[0];
	if (! res || ! row.values)
	{
		LOG(L_ERR, "[worksmart_process_acd] Cannot retrieve command map\n");
		ws_dbf.free_result(h, res);
		return -1;
	}
	nSiteId = row.values[0].val.int_val;
	nGroup = row.values[1].val.int_val;
	nAcdEnabled = row.values[2].val.int_val;
	nExtendedGroup = row.values[3].val.int_val;
	
	ws_dbf.free_result(h, res);
	
	if ( !(nAcdEnabled ^ nExtendedGroup) ) {
		LOG(L_ERR, "[worksmart_process_acd] ACD and Extended group settings are not mutually exclusive.\n");
		return -1;
	}
	if ( !( nAcdEnabled || nExtendedGroup) )	{
		return -1;
	}
	if ( nAcdEnabled ) {
		sprintf(newuri, "acd-0-%d%03d-%s", nSiteId, nGroup, wsuri.token );
	} else if ( nExtendedGroup ) {
		int nGreeting = get_greeting(nGroup, wsuri.context);
		if ( nGreeting == 0 ) {
			// Forward this to group voicemail
			return -2;
		}
		// Get current greeting number
		//ivr-group-greeting-context-token
		sprintf(newuri, "ivr%c%d%c%d%c%s%c%s",WS_SEPERATOR, nGroup, WS_SEPERATOR, nGreeting, WS_SEPERATOR, wsuri.context, WS_SEPERATOR, wsuri.token);
	} else {
		// Should not reach this place!
		LOG(L_ERR, "[worksmart_process_acd] Impossible happend\n");
		return -1;
	}
	sprintf(query, "Contact: <sip:%s@%s:%d>\r\n", newuri, ip_addr2a(&msg->rcv.src_ip), msg->rcv.src_port);
#endif
	LOG(L_ERR,"append_to_reply : %s\n", query);
    if ( add_lump_rpl( msg, query, strlen(query), LUMP_RPL_HDR)==0 )
    {
        LOG(L_ERR,"ERROR:append_to_reply : unable to add lump_rl\n");
        return -1;
    }

//	set_contact_username(msg, newuri);
	return 1;
}
#endif

int get_greeting(int nGroup, char * context) {
	return 0;
}

int is_3pcc(struct sip_msg * msg ) {
	struct to_body * from_body;
	char tag[100];

	if ( -1 == parse_from_header (msg) ) {
		LOG(L_ERR, "[is_3pcc] Parsing From failed Hint:1\n");
		return 0;
	}
	from_body = get_from(msg);
	if ( !from_body ) {
		LOG(L_ERR, "[is_3pcc] Parsing From failed Hint:2\n");
		return 0;
	}
	if ( from_body->tag_value.s && from_body->tag_value.len > 4 ) {
		bzero(tag, sizeof(tag));
		snprintf(tag, sizeof(tag)-1, "%.*s", from_body->tag_value.len, from_body->tag_value.s);
		if ( !strncmp((char *)from_body->tag_value.s + from_body->tag_value.len - 4, "3pcc",4)) {
			return 1;
		}
	}
	//3PCC call
	return 0;
}

int is_3pccacdsup(struct sip_msg * msg ) {
	char tag[128]="";
	struct to_body * from_body;

	if ( -1 == parse_from_header (msg) ) {
		LOG(L_ERR, "[is_3pccacdsup] Parsing From failed Hint:1\n");
		return 0;
	}
	from_body = get_from(msg);
	if ( !from_body ) {
		LOG(L_ERR, "[is_3pccacdsup] Parsing From failed Hint:2\n");
		return 0;
	}
	if ( from_body->tag_value.s && from_body->tag_value.len > 4 ) {
		bzero(tag, sizeof(tag));
		snprintf(tag, sizeof(tag)-1, "%.*s", from_body->tag_value.len, from_body->tag_value.s);
		if (!strncmp((char *)from_body->tag_value.s + from_body->tag_value.len - 6, "acdsup",6))
			return 2;
		else if (!strncmp((char *)from_body->tag_value.s + from_body->tag_value.len - 4, "3pcc",4))
			return 1;
	}
	return 0;
}

/*EXT - EXT Call Via MS --- Vijay Feb2009 */
enum UriType get_uri_type(struct sip_msg *msg, struct ws_uri * wsuri, char *uri,char *token,char *prio1,int uri_len,int token_len) 
{
	char * types[] = {"ext", "cmd", "xfer", "cnf", "acdext", "pst", "pkp", "con", "dir", "inv", "vmln", "vmlo", "agt", "did","mwi", "ptt","wsdis", "obxfer","act","moc","vmd","vmo","iext","icom","bla","gext", "wsace","nine11", "acdxfer","ctc","wsctcb","pcr","fgrp", "six11","lcr", NULL};
	int ni = 0;
	enum UriType uritype = Invalid_Uri;
	char newheader[256]="";
	char tmpuri[256]="",tmpuri1[256]="",cSitename[128]="",cTempfrom[128]="",*ptr=NULL,*ptr1=NULL;
	struct hdr_field *hf = NULL;
	struct sip_uri furi;
	struct to_body * from;
	int nj = 0;
	int nInvalid = 0;
	int nExten = -1;
	char *temp;
	struct ws_uri wsuri1,wsuri2;

	if ( parse_ws_uri (msg, wsuri, uri,token) == 1) {
		int nJunk;
		uritype = WS_URI;
		
		if(wsuri->command && !strcmp(wsuri->command,"611"))
		{
			wsuri->type =  types[33];
		}
	
		LOG (L_ERR, "[get_uri_type] %s\n", wsuri->type);
		for ( ni = 0; types[ni] != NULL; ni++ ) {
			if ( !strcmp(wsuri->type, types[ni])) {

				// Matched

				uritype = ni;
				LOG (L_ERR, "[get_uri_type]2.Debug  %d \n",uritype);
				break;
			}
		}
		
		switch ( uritype ) {
			case Extension:
				//Process ext call
				nJunk = atoi(wsuri->command);
				/* 9990 to 9999 are Feature_Request numbers. Remaining numbers can assingn to users -- Nov-16-2019*/
				if ( nJunk >= 9990 && nJunk <= 9999 ) {
					uritype = Feature_Request;
					LOG (L_ERR, "[get_uri_type] Feature Request\n");
				}
				LOG (L_ERR, "[get_uri_type] Extension\n");
				break;
			
			default:
				break;
		};
	} else {
		// Not a valid wsuri. Great.
		int nJunk, nLength;
		nLength = strlen(uri) ;

		if ( ( nJunk = atoi(uri)) && strstr(uri, "-") == NULL) {
			// Some numeric. Could be direct extension or pstn number.
			if ( nJunk >= 10 && nJunk <= 80 && nLength == 2) {
				// Frame a call pickup uri
				uritype = Pickup_Request;
				LOG (L_ERR, "[get_uri_type] Pickup_Request\n");
			} else if (!strncmp(uri, international_prefix, prefix_length)) {
				// International Call
				LOG (L_ERR, "[get_uri_type] International_Phnumber\n");
				uritype = International_Phnumber;
			/* else if ( nLength == 10 || nLength == 7 || nLength == 11 ) */
			} else if (nLength >= 7 && nLength <= 11 ) {
			// Added to check international dialing
				uritype = Phone_Number;
				LOG (L_ERR, "[get_uri_typeR Phone_Number 7\n");
			} else if ( nLength >= 10 ) {
				// PSTN Numbers
				uritype = Phone_Number;
				LOG (L_ERR, "[get_uri_type] Phone_Number 1\n");
			} else {
				// Invalid Number for PBX
				uritype = Invalid_Uri;
				LOG (L_ERR, "[get_uri_type] Invalid_Uri\n");
			}
		} else {

			for ( ni =0; ni < nLength; ni++) {
				if ( uri[ni] >= '0' && uri[ni] <= '9' ) {
					tmpuri[nj] = uri[ni];
					nj++;
				} else if ( uri[ni] != '-' ) {
					// Against intuitive dialing rules. It could be regular SIP URI  like user-context
					nInvalid = 1;
					break;
				}
			}
			if ( nInvalid ) {
				LOG (L_ERR, "[get_uri_type] Local_User and User: %s\n",uri); 
				if(!ws_ctccall_flag && is_3pcc(msg)){
					uritype = Local_User;
					LOG(L_ERR, "[get_uri_type] 3pcc call\n");
				}else{
					nExten = get_direct_extension(uri,0);
					if(nExten == -1) {
						LOG (L_ERR, "[get_uri_type] Neither 3pcc nor a valid user\n");
						uritype = Invalid_Uri;
					} else {
						// get sitename and frame ext call
						temp =  strchr(uri,'-');
						if(!temp)
							return -1;
						temp++;
						memset(tmpuri,0,sizeof(tmpuri));
						snprintf(tmpuri,sizeof(tmpuri)-1, "ext-%d-0-%s-%s",nExten,temp,token);
						if (parse_ws_uri_buffer(tmpuri,strlen(tmpuri),wsuri)==0) {
							uritype = Extension;
						} else {
							uritype = Invalid_Uri;
						}
						LOG (L_ERR, "[get_uri_type*] Local_User and sitename %s\n",temp);
					}
				}
 			} else {
				tmpuri[nj] = 0;
				uritype = Phone_Number;
				strncpy(uri, tmpuri,uri_len-1);
				LOG (L_ERR, "[get_uri_type] Phone_Number 2\n");
			}
		}
	}

	/* Added for ACD supervisory and CTC routing changes --vravi */
	if(ws_ctccall_flag){
		if((uritype ==  CTC_Request) && (is_3pccacdsup(msg)== 1 )){

			if (!parse_ws_uri_buffer(uri,strlen(uri),&wsuri2)) {	
		 		if(isdigit(wsuri2.group[0]) && (strlen(wsuri2.group) <=5) && isdigit(wsuri2.context[0]) && (atoi(wsuri2.context)==0)){
	                LOG(L_WARN, "[get_uri_type] Not Adding Headers for 3pcc call as it is PSTN call\n");
	            }else{
					LOG(L_WARN, "[get_uri_type] Adding Headers for Auto Answer on 3pcc call\n");
					strncpy(newheader,"Alert-Info: Auto Answer\r\n",sizeof(newheader)-1);
					str cmd={newheader,strlen(newheader)};
					if (append_hf) {
						if(append_hf(msg, (char *)&cmd,0)!=1)
							LOG(L_WARN,"[get_uri_type]header 1 append_hf failure\n");
					}
					memset(newheader,0x00,sizeof(newheader));
					snprintf(newheader,sizeof(newheader)-1,"Call-Info:<sip:%s>\\;answer-after=0\r\n",proxy_domain);
					str cmd1={newheader,strlen(newheader)};
					if (append_hf) {
						if(append_hf(msg, (char *)&cmd1,0)!=1)
							LOG(L_WARN,"[get_uri_type] header2 append_hf failure\n");
					}
				}
			}
			/* Added fixes for CTC exten exten Dialing --vravi */
			from = get_from(msg);
			if(from){
				parse_uri(from->uri.s, from->uri.len, &furi);
				memset(cTempfrom,0x00,sizeof(cTempfrom));
	    	    snprintf(cTempfrom,sizeof(cTempfrom)-1,"%.*s" ,furi.user.len, furi.user.s);
				if((ptr = strchr(cTempfrom,'-')) && ++ptr){
	        		snprintf(cSitename,sizeof(cSitename)-1,"%s" ,ptr);
				}else{
	        		for (hf=msg->headers; hf; hf=hf->next) {
	            		if(!strncasecmp (hf->name.s,"WSWEBCallInfo",13)){
							memset(cTempfrom,0x00,sizeof(cTempfrom));
	    	    			snprintf(cTempfrom,sizeof(cTempfrom)-1,"%.*s" ,hf->body.len, hf->body.s);
							if((ptr = strchr(cTempfrom,'-')) && ++ptr){
								if((ptr1 = strchr(ptr,'#'))){
									snprintf(cSitename, sizeof(cSitename)-1, "%.*s",ptr1-ptr, ptr);
									cSitename[ptr1-ptr]='\0';
								}else							
    	    						snprintf(cSitename,sizeof(cSitename)-1,"%s" ,ptr);
							}
    	            		break;
    	       	 		}
    	    		}
				}
			}
			if(isdigit(wsuri->group[0]) && strlen(wsuri->group) <= 5){
    	    	nExten = atoi(wsuri->group);
				memset(tmpuri,0,sizeof(tmpuri));
				if(atoi(prio1) != 1 || (atoi(wsuri->context)==0))
					snprintf(token,token_len-1,"%s",wsuri->token);
    	       	snprintf(tmpuri,sizeof(tmpuri)-1, "iext-%d-0-%s-%s",nExten,cSitename,token);
    	        if (parse_ws_uri_buffer(tmpuri,strlen(tmpuri),wsuri)==0) {
					 LOG (L_ERR, "[get_uri_type] CTC MAKING IT AS IEXT >\n");
    	             uritype = IEXTCALL;
    	        } else {
					uritype = Invalid_Uri;
    	        }
			}
		}else if(uritype ==  WSCTCBCALL){
	
			if (!parse_ws_uri_buffer(uri,strlen(uri),&wsuri1)) {
				memset(tmpuri,0,sizeof(tmpuri));
				snprintf(tmpuri,sizeof(tmpuri)-1,"%s-%s",wsuri1.group,wsuri1.context);
			}
			nExten = get_direct_extension(tmpuri,0);
			if(nExten == -1) {
				LOG (L_ERR, "[get_uri_type] WSCTCBCALL Not a  valid  worldsmart user\n");
				uritype = Invalid_Uri;
			} else  {
				// get sitename and frame ext call
				temp =  strchr(tmpuri,'-');
				if(!temp)
					return -1;
				temp++;
				memset(tmpuri1,0,sizeof(tmpuri1));
				/* Added fixes for multiple tokens in case of CTC buddy dialing routing plan cases */
				if(wsuri1.token && (strlen(wsuri1.token) >0))
					snprintf(tmpuri1,sizeof(tmpuri1)-1, "ext-%d-0-%s-%s",nExten,temp,wsuri1.token);
				else
					snprintf(tmpuri1,sizeof(tmpuri1)-1, "ext-%d-0-%s-%s",nExten,temp,token);
				if (parse_ws_uri_buffer(tmpuri1,strlen(tmpuri1),wsuri)==0) {
					uritype = Extension;
				} else {
					uritype = Invalid_Uri;
				}
			}
		}
	}
	/* Added this to fix null token issue in case of PSTN caller side blind transfer * --vravi/dinesh */
	if( wsuri && (! wsuri->token)){
		wsuri->token = token;
		LOG (L_ERR, "[get_uri_type] NULL TOKEN FOUND SO ADDING TOKEN AGAIN  %s \n",wsuri->token);
	}
	LOG (L_ERR, "[get_uri_type]1.Debug  %d \n",uritype);
	return uritype;
}

static int is_in_mediapoollist(char *pRouteIp){
	int ret = 0;
	if(pRouteIp == NULL){
		return ret;
	}
	if(isInList(mediaips,pRouteIp)){
		ret = 1;
	}
	return ret;
}


/* Changed on May 25: Added failure routing for PSTN calls
 * This is a temporary fix, and works only with the 36 box.
 * This has to be replaced immediately with the actual bug fixes in the B2BUA
 */
int route_to_server(struct sip_msg *msg, char * phone, char * server , int sflag)
{
	LOG(L_ERR, "[route_to_server]  phone = '%s'\n server='%s'\n \n sflag = '%d'\n", phone,server,sflag);
	char fulluri[100];
	int nMediapool=0;
	bzero (fulluri, sizeof (fulluri));
	if ( !msg || !phone || !server )
   	{
		send_reply(msg, 500, "Internal Server Error, hint - 1");
		return 1;	
	}	
	
	strncpy(fulluri, "sip:",sizeof(fulluri)-1);
	strcat(fulluri, phone);
	strcat(fulluri, "@");
	strcat(fulluri, server);
	clear_branches();
	if (set_uri2(msg, fulluri) > 0)
   	{
		/*This if added by tulsi. for checking the media server pool call. especially warm transfers
		 * * Here one bug i am expecting if more than 3 media severs. For this we need to implement here islist condition*/
		nMediapool = is_in_mediapoollist(server);
		//if ((strcmp(server, umslb_proxy) == 0) || (strcmp(server, umslb_proxy1) == 0) || (strcmp(server, umslb_proxy2) == 0))
		if ((nMediapool == 1))
		{
			if(_nSupSeq > 5)
			{
					LOG(L_ERR, "[route_to_server] _nSupSeq:%d\n",_nSupSeq);
					if (t_on_failure)
					{
							/*t_on_failure(msg, (char *)(20), NULL);*/
							t_on_failure(msg, (char *)(20 + _nSupSeq), NULL);
					}
					media_failover = 0;
			}else {
					LOG(L_ERR, "[route_to_server] Came from media server pool but, _nSupSeq is not matching:%d\n",_nSupSeq);
					if (t_on_failure)
						t_on_failure(msg, (char *)(10 + _nOrigSeq), NULL);
			}
		}else if ((sflag == MS)&&((strcmp(server, umslb_proxy) == 0) || (strcmp(server, umslb_proxy1) == 0)))
		{
			LOG(L_ERR, "[route_to_server] MS FAILOVER\n");
			if (t_on_failure)
			{
				//t_on_failure(msg, (char *)(10), NULL);
				t_on_failure(msg, (char *)(10 + _nOrigSeq), NULL);
			}
		}
		else if ((sflag == OMS)&&((strcmp(server, umslb_proxy) == 0) || (strcmp(server, umslb_proxy1) == 0)))
		{
			LOG(L_ERR, "[route_to_server] OMS FAILOVER\n");
			if (t_on_failure)
			{
				//t_on_failure(msg, (char *)(12), NULL);
				LOG(L_ERR, "[route_to_server] _nOrigSeq = '%d'\n", _nOrigSeq);
				t_on_failure(msg, (char *)(15 + _nOrigSeq), NULL);
			}
		}
		else if ((sflag == MS)&& ((strcmp(server, wsicp_primary) == 0) || (strcmp(server, wsicp_secondary) == 0)))
		{
			if (t_on_failure)
			{
				t_on_failure(msg, (char *)(28 + _nOrigSeq), NULL);
			}
		}
		/*else if ((strcmp(server, umslb_proxy) == 0) || (strcmp(server, umslb_proxy1) == 0))
		{
			if (t_on_failure)
			{
				//t_on_failure(msg, (char *)(11), NULL);
				t_on_failure(msg, (char *)(20 + _nOrigSeq), NULL);
			}
		}*/

		if (_nOrigSeq != 1 && _nOrigSeq !=6) /*This && added by tulsi for media pool*/
		{
			append_branch(msg, NULL, 0, 0, 0, 0);
		}

		/* We are calling t_relay in worksmart.c, no need to call here --Ramu 05/01/05 */
		//      call_t_relay (msg);
	}
	else
	{
		send_reply(msg, 500, "Internal Server Error, hint - 1");
	}

	if (((sflag == MS) && (!strncmp(server,umslb_proxy,strlen(umslb_proxy)))) ||((sflag == ACDMS) && (!strncmp(server,umslb_proxy,strlen(umslb_proxy)))))
	{
		/* Dont use MediaProxy, we are handling NAT @ wsmserver as well as wsacdserver -Ramu 10/03/06*/	
		return 2;
	}	
	else
	{
		/* Outbound Call, use mediaproxy */
		return 1;		
	}
}


int set_uri2 (struct sip_msg *msg, char * phone)
{
	int len, ret;
	
	if (msg->new_uri.s) {
		pkg_free(msg->new_uri.s);
		msg->new_uri.len=0;
	}
	msg->parsed_uri_ok=0;
	len=strlen(phone);
	LOG(L_ERR, "[set_uri2] URI Length: %d\n", len);
	msg->new_uri.s=pkg_malloc(len+1);
	if (msg->new_uri.s==0){
		LOG(L_ERR, "ERROR: do_action: memory allocation"
						" failure\n");
		ret=E_OUT_OF_MEM;
		return -1;
	}
	memcpy(msg->new_uri.s, phone, len);
	msg->new_uri.s[len]=0;
	msg->new_uri.len=len;

    return 1;
}

/* This Routine is modified on 18th October 2005. The aim is to maintain a single version of
 *   Worksmart module, irrespective of the country in which the server is located - Gopinath
 */
int normalize( char * phone, char * buffer, int nBufferLen,int phone_len) 
{
	char * pTemp = NULL;
	int nLen = strlen (phone);
	char * pCurrent = phone;
	if ((pTemp = strstr(phone, "-")) == NULL) {
		switch (country_code) {
			/* In USA, all national numbers are of the same length (10 digits) */	
			case USA:	
				if (10 == nLen) snprintf (buffer, nBufferLen - 1, "%s%s", national_prefix, phone);
				else strncpy (buffer, phone, nBufferLen - 1);
				break;
			/* We dont have specific length for UK, there can be 9,10,11 digit numbers */	
			case UK:
				if (phone[0] == national_prefix[0])	strncpy (buffer, phone, nBufferLen - 1);
				else	snprintf (buffer, nBufferLen - 1, "%s%s", national_prefix, phone);
				break;
			/* Other countries dialing go here */		
			default:
				return 0;
		}		
	}
	else {
		buffer[0] = '\0';
		do {
			*pTemp = '\0';
			strcat(buffer, pCurrent);
			pCurrent = pTemp+1;
			pTemp++;
	    }while ((pTemp = strstr(pTemp, "-")) != NULL );
		if ( pTemp == NULL && pCurrent != NULL ) {
			strcat(buffer, pCurrent);
		}
		memset(phone,0,phone_len);
		strncpy(phone, buffer,phone_len-1);
		return normalize(phone, buffer, nBufferLen,phone_len);
	}

	return strlen(buffer);
}

int call_t_relay(struct sip_msg * msg) 
{
	//append_branch(msg, msg->first_line.u.request.uri.s, msg->first_line.u.request.uri.len, 0, 0, 0);
	
	if (t_relay)
		return t_relay(msg, NULL, NULL);
	return -1;
}
int send_reply(struct sip_msg * msg, int nCode, char * pMessage) {
	if ( sl_send_reply ) {
		return sl_send_reply(msg, (char *)nCode, pMessage);
	} else {
		LOG(L_ERR, "sl_send_reply not found");
	}
	return -1;
}
int get_db_int(db_val_t value) {
	switch ( value.type ) {
		case DB_INT:
			return value.val.int_val;
			break;
		case DB_STRING:
			return atoi(value.val.string_val);
			break;
		case DB_STR:
		{
			char str[10];
			bzero(str, sizeof(str));
			memcpy(str, value.val.str_val.s, value.val.str_val.len);
			return atoi(str);
		}
			break;
		default:
			LOG(L_ERR, "[get_db_int] You are going wrong here!!!!\n");
			return 0;
			break;
	};
}
/*!
 * Project : CNAM
 * Date: 29-10-2009
 * Usage: Send PUBLISH method with Caller-ID header to 3pcc server 
 * Frame the PUBLISH method ,with adding Caller-ID header 
 * Caller-ID header Contains Phonenumber and Siteid(For billing purpose)
 * */
int send_publish_message(char *cid,int sid,char *server,char *ownip)
{

	char r_uri[256]		= "",
		 tobuf[256]     = "",
		 ctabuf[256]    = "",
		 frombuf[256]   = "",
		 callerid[256]= "";

	str str_to       = {0,0},
		str_from     = {0,0},
		msg_type     = {"PUBLISH",7},
		str_callerid = {0,0},
		str_ruri={0,0};
	
	snprintf(frombuf,sizeof(frombuf),"<sip:worldsmart@%s>",ownip);
	snprintf(ctabuf,sizeof(ctabuf),"<worldsmart@%s>",ownip);
	snprintf(tobuf,sizeof(tobuf),"<3pcc@%s>",server);
	snprintf(r_uri,sizeof(r_uri),"sip:3pcc@%s",server);
	snprintf (callerid , sizeof(callerid) , "Caller-ID: %s-%d",cid,sid);

	str_to.s = tobuf;
	str_to.len = strlen(tobuf);
	str_from.s = frombuf;
	str_from.len = strlen(frombuf);
	str_ruri.s = r_uri;
	str_ruri.len = strlen(r_uri);
	str_callerid.s = callerid;
	str_callerid.len = strlen(callerid);

	LOG(L_ERR,"[ws_cnam_request]Sending Request 3pcc server %s\n",callerid);
	_tmb.t_request(&msg_type,&str_ruri,&str_to, &str_from, &str_callerid, NULL,0, NULL);

	return 0;
}

int is_header_exists(struct sip_msg *msg,char *header_name,int header_name_len,char *header_value,int header_value_len){
	int nret = 0;
	struct hdr_field *hf = NULL;
	/* Checking for header */
	for(hf=msg->headers; hf; hf=hf->next){
		if (hf->name.len == header_name_len && ( !strncasecmp (hf->name.s, header_name, header_name_len))){
			snprintf(header_value,header_value_len -1,"%.*s",hf->body.len,hf->body.s);
				nret = 1;
				break;
		}
	}
	return nret;
}

int add_header(struct sip_msg *msg,char *header,int header_len)
{
	str cmd1;
	cmd1.s= header;
	cmd1.len = header_len;

	if (append_hf) {
		if(append_hf(msg, (char *)&cmd1,0)==1){
			LOG(L_ERR,"[add_header] header append_hf success\n");
		}else{
			LOG(L_ERR,"[add_header] header append_hf failure\n");
		}
	}
	return 0;
}


/*Checking special symbols from contact and from headers ,if found send negative response*/
int Validate_Contact(struct sip_msg *msg){

	str from_uri;
	struct sip_uri parsed_uri;
	char sipcontact[128]="",*ptr = NULL,*con =NULL,cContact[128]="",cFrom_uri[128]="";
	int i=0;

	if(!msg || !msg->from){
		LOG(L_ERR,"[Validate_contact]any of the above value is NULL\n");
		return -1;
	}	
	if(!msg->from->parsed && (parse_from_header(msg) == -1) ){
		LOG (L_ERR, "[Validate_contact]Oh! Failed parsing from header\n");
		return -1;
	}

	from_uri = get_from(msg)->uri;
	if (parse_uri(from_uri.s, from_uri.len, &parsed_uri)){
		LOG (L_WARN, "[Validate_contact] Failed to parse FROM uri\n");
		return -1;
	}

	if(parsed_uri.user.s && strlen(parsed_uri.user.s) >0){
		memset(cFrom_uri,0,sizeof(cFrom_uri));
		snprintf(cFrom_uri,sizeof(cFrom_uri)-1,"%.*s",parsed_uri.user.len ,parsed_uri.user.s);
	}
	
	if(msg->contact != NULL && parse_headers (msg, HDR_CONTACT, 0) != -1){
		if(msg->contact->body.len > 0){
			memset(sipcontact,0,sizeof(sipcontact));
			snprintf(sipcontact,sizeof(sipcontact)-1,"%.*s",msg->contact->body.len,msg->contact->body.s);
			if(strlen(sipcontact) >0){
				if((ptr = strrchr(sipcontact,'@'))){
                      *ptr='\0';
					if((con = strrchr(sipcontact,':')) && (++con)){/* Replaced Here strchr to strrchr (for '@' and ':' case) to fix -"Block invalid character in contact header"  --Balaji/Swaroopa [04/03/2019] */
						memset(cContact,0,sizeof(cContact));
						strncpy(cContact,con,sizeof(cContact)-1);
					}
				}
			}
		}
	}

	for (i=0;cContact[i];i++) {
		if (! isalnum (cContact[i])) {
			if (strchr (Contact_invalid_Chars, cContact[i])) {
				LOG (L_ERR, "[Validate_contact] Invalid URI: %s %c (Contact header with special characters)\n", cContact,cContact[i]);
				return -1;
			}
		}
	}
	for (i=0;cFrom_uri[i];i++) {
		if (! isalnum (cFrom_uri[i])) {
			if (strchr (Contact_invalid_Chars, cFrom_uri[i])) {
				LOG (L_ERR, "[Validate_contact] Invalid URI: %s %c (From header with special characters)\n", cFrom_uri,cFrom_uri[i]);
				return -1;
			}
		}
	}
	return 1;
}
