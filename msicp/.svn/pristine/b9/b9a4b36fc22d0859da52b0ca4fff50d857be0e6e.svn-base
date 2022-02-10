
/*		File	: wsutils.c
		Notes   : This is the CONFIDENTIAL property of PANDORA NETWORKS (IND) PVT. LTD.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include <sys/time.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "../../db/db.h"
#include "../mysql/dbase.h"
#include "../../route_struct.h"
#include "../registrar/lookup.h"
#include "../../data_lump.h"
#include "../../dset.h"
#include "../../mem/mem.h"
#include "../../trim.h"
#include "../../msg_translator.h"
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"

#include "../../parser/msg_parser.h"
#include "../../parser/parse_uri.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/contact/contact.h"
#include "../../parser/parse_from.h"

#include "../../resolve.h"
#include "../../proxy.h"

#include "wsutils.h"

MODULE_VERSION

static int mod_init (void);
static int child_init(int _rank );
static void mod_destroy (void);

static void mod_destroy (void)
{
   printf ("Destroying module\n");
}


static cmd_export_t cmds[]={
	{"get_agentid", (cmd_function) get_agentid,   2, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"get_macid",  (cmd_function) get_macid,    2, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{0, 0, 0, 0, 0}
};

static param_export_t params[]={
	{0,0,0}
};
static int mod_init(void)
{
	LOG(L_ERR, "Wsutils initializing !!!!\n");
	return 0;
}

struct module_exports exports =
{
   "wsutils",
    cmds,       /* Exported functions       */
    params,     /* Exported parameters      */
    mod_init,   /* initialize module        */
    0,          /* response function        */
    mod_destroy,    /* destroy function         */
    0,          /* oncancel function        */
    child_init  /* per-child init function  */
};

int get_macid(struct sip_msg* _m , char * macaddr, char * macaddr_len)
{
	    char cBuf[2048] = "", macid[16] = "", cBuf1[256] = "";
	    char *ptr = NULL,server_header[64]="";
	    int mac_len = atoi(macaddr_len);
		str callid;
		struct hdr_field *hf = NULL;

	    if(!_m){
		    LOG(L_ERR,"SIP msg is NULL\n");
		    return -1;
	    }

	    memset(&cBuf,0,sizeof(cBuf));
	    memset(&cBuf1,0,sizeof(cBuf1));
	    memset(&macid,0,sizeof(macid));
		if(_m->user_agent && _m->user_agent->body.len > 0 && _m->user_agent->body.s)
		{
				if((!strstr(_m->user_agent->body.s,"Cisco/SPA")) && ( strstr(_m->user_agent->body.s,"Cisco") || strstr(_m->user_agent->body.s,"CSCO/6"))) {
				  if(_m && _m->callid && _m->callid->body.len > 0) {
				             callid.s = _m->callid->body.s;
				             callid.len = _m->callid->body.len;
				             trim_trailing(&callid);
				             snprintf(cBuf, sizeof(cBuf)-1, "%.*s",callid.len,callid.s);
				             snprintf(cBuf1,14,"%s",cBuf);
				             if((ptr = strchr(cBuf1,'-'))){
				             *ptr = '\0';
				             ptr++;
					          if(ptr){
					          snprintf(macid,sizeof(macid) - 1,"%s%s",cBuf1,ptr);
					          }
				            }
				   }
				  if(strlen(macid)<=0){
				           if(_m->from && _m->from->body.len > 0){
			                    snprintf(cBuf, sizeof(cBuf)-1,"%.*s",_m->from->body.len,_m->from->body.s);
								if((ptr = strstr(cBuf, "tag=")))
			                    {
			                        ptr = ptr + 4;
			                        if(ptr)
			                        {
			                            *(ptr+12) = '\0';
			                            snprintf(macid, sizeof(macid) - 1, "%s", ptr);
			                        }
			 			        }
               				 }
				 }
		}else if(strstr(_m->user_agent->body.s,"Polycom")) {
		     snprintf(cBuf, sizeof(cBuf)-1, "%.*s", _m->user_agent->body.len,_m->user_agent->body.s);
		     ptr = cBuf+strlen(cBuf)-12;
		     if(ptr && (*(ptr-1) == '_')){
		     snprintf(macid,sizeof(macid) - 1, "%s", ptr);
		 	}
	    }else if(strstr(_m->user_agent->body.s,"snom-m3")) {
	        if((ptr = strstr(_m->user_agent->body.s , "MAC="))) {
              ptr= ptr + 4;
            if(ptr){
 	              snprintf(macid,sizeof(macid) - 1, "%s", ptr);
	              *(macid+12) = '\0';
	   		}
	    } 
		}else if(strstr(_m->user_agent->body.s,"snom")) {
	        if(_m && _m->callid && _m->callid->body.len > 0) {
	           callid.s = _m->callid->body.s;
	           callid.len = _m->callid->body.len;
	           snprintf(cBuf, sizeof(cBuf)-1, "%.*s",callid.len,callid.s);
	           if((ptr = strchr(cBuf,'@'))) {
	           		ptr = cBuf+strlen(cBuf)-12;
 					if(ptr){
		                snprintf(macid,sizeof(macid) - 1, "%s", ptr);
				    }
                }
		    }
	    if(strlen(macid) <= 0){
	          struct hdr_field *hf = NULL;
	          for (hf=_m->headers; hf; hf=hf->next) {
	              if ((hf->name.len == 14) && (strncasecmp (hf->name.s, "X-Serialnumber", 14) == 0)){
	                  if(hf->body.len > 0 && hf->body.s){
	                         snprintf(macid,sizeof(macid) - 1, "%.*s", hf->body.len,hf->body.s);
	                  }
	                  break;
	              }
	          }
	   	}
	   }else if(strstr(_m->user_agent->body.s,"Aastra")){
	        struct hdr_field *hf = NULL;
	        for (hf=_m->headers; hf; hf=hf->next) {
	        if ((hf->name.len == 10) && (strncasecmp (hf->name.s, "Aastra-Mac", 10) == 0)){
		        if(hf->body.s){
	    		    snprintf(macid,sizeof(macid) - 1, "%.*s", hf->body.len,hf->body.s);
	        	}
	        	break;
		    }
	   }
	  if(strlen(macid) <= 0){
	       if(_m->contact && _m->contact->body.len > 0){
	               snprintf(cBuf, sizeof(cBuf)-1, "%.*s",_m->contact->body.len, _m->contact->body.s);
	                if((ptr = strrchr(cBuf, '>'))){
	                     *ptr = '\0';
	                     ptr = ptr-12;
		                  if(ptr){
		                      snprintf(macid,sizeof(macid) - 1, "%s", ptr);
   						  }
  	                }
	    	    }
      		 }
		}

		else if (strstr(_m->user_agent->body.s,"LG-Nortel")){
		    ptr = strchr(_m->user_agent->body.s,'_');
		      if(ptr){
		          ptr++;
		          if(ptr){
		              strncpy(macid,ptr,sizeof(macid));
		          }
				 if((ptr=strchr(macid,'_'))){
				        *ptr ='\0';
			     }
			  }
        }
		else if (strstr(_m->user_agent->body.s,"DPH-")){
		   if(_m && _m->callid && _m->callid->body.len > 0) {
		     callid.s = _m->callid->body.s;
		     callid.len = _m->callid->body.len;
		     snprintf(cBuf, sizeof(cBuf)-1, "%.*s",callid.len,callid.s);
		     if((ptr = strstr(cBuf, "REGISTER_"))) {
		             ptr = ptr + 9;
		             if(ptr) {
		                strncpy(macid,ptr,sizeof(macid));
		             }
		     	if((ptr=strchr(macid,'_'))) {
		            *ptr ='\0';
		     	}
		     }
	      }
		}
		else if (strstr(_m->user_agent->body.s,"Patton Smartlink MATA") || strstr(_m->user_agent->body.s,"Panasonic")) {
		    snprintf(cBuf, sizeof(cBuf)-1, "%.*s", _m->user_agent->body.len,_m->user_agent->body.s);
		    ptr = cBuf + (strlen(cBuf) - 13);
		    if(ptr){
		          snprintf(macid,sizeof(macid) - 1, "%s", ptr);
		            if((ptr = strchr(macid, ')')) || (ptr = strchr(macid, '>'))) {
		             *ptr = '\0';
		            }
            }
        }else if (strstr(_m->user_agent->body.s,"Mitel")){ /* Added mitel parsing changes --vravi/gajay */
            snprintf(cBuf, sizeof(cBuf)-1, "%.*s", _m->user_agent->body.len,_m->user_agent->body.s);
            ptr = strrchr(cBuf,' ');
             if(ptr){
              ptr++;
              if(ptr){
                strncpy(macid,ptr,sizeof(macid));
	          }
		    }
        }else if(strstr(_m->user_agent->body.s,"Yealink")){
			memset(cBuf , 0 ,sizeof(cBuf));
			snprintf(cBuf, sizeof(cBuf)-1, "%.*s", _m->user_agent->body.len,_m->user_agent->body.s);
			ptr = cBuf+strlen(cBuf)-12;
			if(ptr && (*(ptr-1) == ' ')) {
				snprintf(macid,sizeof(macid) - 1, "%s", ptr);
			}
        }else if(strstr(_m->user_agent->body.s,"Fanvil") || strstr(_m->user_agent->body.s,"Kumo")){
			memset(cBuf , 0 ,sizeof(cBuf));
			snprintf(cBuf, sizeof(cBuf)-1, "%.*s", _m->user_agent->body.len,_m->user_agent->body.s);
			ptr = cBuf+strlen(cBuf)-12;
			if(ptr && ((*(ptr-1) == ' ') || (*(ptr-1) == '_'))) {
				snprintf(macid,sizeof(macid) - 1, "%s", ptr);
			}
        }
	    snprintf(macaddr,mac_len, macid);
	  }
		else
	    {
	       /*Adding for Server Header for getting User Agent Details*/
			for (hf=_m->headers; hf; hf=hf->next) {
			if(hf->name.len!=6 || strncasecmp(hf->name.s,"Server",6)!=0){
				continue;
		    }
		   		memset(server_header,0x00,sizeof(server_header));
				snprintf(server_header,sizeof(server_header)-1,"%.*s",hf->body.len,hf->body.s);
			    break;
			}
			if(strlen(server_header) > 0 && strstr(server_header,"Aastra")!=NULL){
				if(_m->contact && _m->contact->body.len > 0){
					snprintf(cBuf, sizeof(cBuf)-1, "%.*s", _m->contact->body.len,_m->contact->body.s);
						if((ptr = strrchr(cBuf,'>'))){
							*ptr = '\0';
		                    ptr=ptr-12;
		                        if(ptr){
				                      snprintf(macid,sizeof(macid) - 1, "%s", ptr);
								}
						}
					snprintf(macaddr,mac_len, macid);
				}else{
				LOG(L_ERR,"[wsutils::get_macid] contact not exist\n");
                }
			}
			else if(strlen(server_header) > 0 && !strstr(server_header,"Cisco/SPA") && strstr(server_header,"Cisco")!=NULL){
				if(_m && _m->callid && _m->callid->body.len > 0) {
					callid.s = _m->callid->body.s;
					callid.len = _m->callid->body.len;
					trim_trailing(&callid);
                    snprintf(cBuf, sizeof(cBuf)-1, "%.*s",callid.len,callid.s);
                    snprintf(cBuf1,14,"%s",cBuf);
						if((ptr = strchr(cBuf1,'-'))){
							*ptr = '\0';
							ptr++;
								if(ptr){
		                         snprintf(macid,sizeof(macid) - 1,"%s%s",cBuf1,ptr);
								}
						}
				}
				if(strlen(macid)<=0){
					if(_m->to && _m->to->body.len > 0){
						snprintf(cBuf, sizeof(cBuf)-1,"%.*s",_m->to->body.len,_m->to->body.s);
						if((ptr = strstr(cBuf, "tag=")))
                        {
                               ptr = ptr + 4;
	                           if(ptr)
	                           {
	                             *(ptr+12) = '\0';
								 snprintf(macid, sizeof(macid) - 1, "%s", ptr);
								 snprintf(macaddr,mac_len, macid);
							   }
						}
					}
				}
			}else if(strlen(server_header) > 0 && strstr(server_header, "Cisco/SPA")){
                if((ptr = strrchr(server_header, '-')) && ++ptr){
                    snprintf(macid, sizeof(macid) - 1, "%s", ptr);
                    ptr = NULL;
                }
                snprintf(macaddr, mac_len, macid);
            }

		}

	    return 0;
}

int get_agentid(struct sip_msg *msg , char *agentid, char *s2)
{
	int nPhonetype = 0;
	char cTmp[256] = "", *cPtr = NULL;
	
	if(!agentid) 
	{
		LOG(L_ERR, "[get_agentid]Agentid is NULL..\n");
		return -1;
	}
	
	if((cPtr = strchr(agentid,'-')))
	{
		*cPtr = '@';
		strncpy(cTmp ,cPtr,sizeof(cTmp)-1);
		
		if(cPtr && isdigit(cPtr[-2])) {
			cPtr = cPtr -2;
		}else if(cPtr && isdigit(cPtr[-1])) {
			cPtr--;
		}
		
		if(cPtr){
			nPhonetype = atoi(cPtr);
		}
		
		if(nPhonetype > 0 && nPhonetype < 32) {
			*cPtr = '\0';
			strcat(agentid,cTmp);
		}
		
		while((cPtr = strchr(agentid,'-')))
		{
			*cPtr = '.';
		}
	}
	return 0;
}

static int child_init(int _rank ) {
		LOG(L_ERR, "Child Init called :\n");
	    return 0;
}

