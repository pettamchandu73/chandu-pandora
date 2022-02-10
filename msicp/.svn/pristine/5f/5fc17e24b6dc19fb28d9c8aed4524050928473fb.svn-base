/* $Id: racc_mod.c,v 1.3 2007/03/07 10:20:47 hari Exp $
 *
 * racc: Accounting module for WorkSmart
 * Written by: Girish Gopinath
 * Copyright: Pandora Networks (c)2004-2005
 *
 */

#include <time.h>
#include<pthread.h>

#include "../../sr_module.h"
#include "../../mem/mem.h"
#include "../tm/t_hooks.h"
#include "../tm/tm_load.h"
#include "../tm/h_table.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_from.h"

#include "racc.h"
#define RACCDBG

MODULE_VERSION

static void racc_onreq( struct cell* t, int type, struct tmcb_params *ps );
static void tmcb_func( struct cell* t, int type, struct tmcb_params *ps );
static void racc_onrequestfwded (struct cell *, struct sip_msg *, int, void *);
static void racc_onrespout (struct cell *, struct sip_msg *, int, void *);
static void racc_onrespin (struct cell *, struct sip_msg *, int, void *);
static int write_to_fifo (struct racc_call_details *billing_data);
static int create_billing_thread(void);
static void *billing_thread(void *data);
static int insert_billing_session(struct racc_call_details *s);
static int insert_stir_session(struct racc_stir_details *s);
int (*_get_macid)(struct sip_msg* m, char *macid, char* macaddrlen);

/* Handling 302 20/01/2006 Ramu*/
int racc_process_moved_temporarily(struct sip_msg *, char *, char *);
/* Handling 611 calls cdr enteries */
int ws_missedcall_entry_for_rejected_calls(struct sip_msg *, char *, char *);
/* Get the numeric extension */


static int mod_init (void);
static int child_init (int);
static void destroy (void);

/* Variables for FIFO communication */
static char *wsdb_fifo_name;
int is_billing_flag =0;
int fifo_fd;
int nEnableLogs = 0;

pthread_t t;
pthread_attr_t attr;
static sem_t billing_semid;
pthread_mutex_t billing_datalock;
str own_ip ;

/* Stir Shaken FIFO Communication */
static char *stir_fifo_name;
int stir_fifo_fd;
static sem_t stir_semid;
pthread_mutex_t stir_datalock;
pthread_t stir_t;
pthread_attr_t stir_attr;

static int stir_create_billing_thread(void);
static void *stir_billing_thread(void *data);
static int write_to_stir_fifo (struct racc_stir_details *billing_data);
static int insert_stir_session(struct racc_stir_details *s);
/* Stir-shaken end */

char *_specialchars = NULL;

struct tm_binds tmb;
cmd_function ws_isinlist; 
cmd_function clear_forkgroup_shared_memory;
static cmd_export_t cmds[] = 
{
	{"racc_process_moved_temporarily",  (cmd_function) racc_process_moved_temporarily, 1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
    {"ws_missedcall_entry_for_rejected_calls",(cmd_function) ws_missedcall_entry_for_rejected_calls,1,0,REQUEST_ROUTE|ONREPLY_ROUTE},
	{0, 0, 0, 0, 0}
};

/* Exported parameters */
static param_export_t params[] =
{
	{"wsdb_fifo_name", STR_PARAM, &wsdb_fifo_name},	
	{"is_billing_flag", INT_PARAM, &is_billing_flag},	
	{"enable_logs_for_billingtest", INT_PARAM, &nEnableLogs},	
	{"stir_fifo_name", STR_PARAM, &stir_fifo_name},
	{"specialchars",STR_PARAM, &_specialchars},
	{0, 0, 0 }
};

struct module_exports exports = 
{
	"racc",
	cmds,		/* Exported functions 		*/
	params,		/* Exported parameters 		*/
	mod_init,	/* initialize module 		*/
	0,			/* response function		*/
	destroy,	/* destroy function 		*/
	0,			/* oncancel function 		*/
	child_init	/* per-child init function 	*/
};


static int mod_init(void)
{
	/* From the acc module */
	load_tm_f	load_tm;

	LOG (L_ERR, "racc - initializing...\n");

	/* import the TM auto-loading function */
	if (! (load_tm = (load_tm_f) find_export ("load_tm", NO_SCRIPT, 0))) {
		LOG (L_ERR, "ERROR: (racc): mod_init: can't import load_tm\n"); 
		return -1;
	}
	
	/* let the auto-loading function load all TM stuff */
	if (load_tm (&tmb) == -1) {
		LOG (L_ERR, "ERROR: (racc): Autoloading failed\n"); 
		return -1;
	}
	LOG(L_NOTICE, "INFO: Registering tcmpb\n");

	if (tmb.register_tmcb (0, 0, TMCB_REQUEST_IN, racc_onreq, 0) <= 0){
		LOG (L_ERR, "ERROR: (racc): tmb.register_tmcb failed\n"); 
		return -1;
	}
	ws_isinlist = find_mod_export("worksmart", "ws_isinlist", -1, 0);
	if(ws_isinlist == NULL) {
		LOG(L_ERR , "[mod_init][racc]Unable export  isinlist from Worksmart module \n"); 
		return -1; 
	}
	_get_macid = find_export("get_macid", 2, 0);
	if(!_get_macid){
	   LOG(L_ERR, "Exporting get_macid function from wsutils module failed \n");
		return -1;
	} 
	own_ip = *((str *) find_param_export("worksmart", "own_ip", STR_PARAM));
	if(own_ip.s == NULL) {
		LOG(L_ERR, "Exporting own_ip from worksmart module failed \n");
		return -1;
	}
	LOG(L_ERR, "Exported own_ip < %s > \n",own_ip.s);

	clear_forkgroup_shared_memory = find_mod_export("worksmart", "ws_clear_forkgroup_shared_memory", -1, 0);
	if(clear_forkgroup_shared_memory == NULL) {
		LOG(L_ERR , "[mod_init][racc]Unable export  clear_forkgroup_shared_memory from Worksmart module \n"); 
		return -1; 
	}
	return 0;
}

/* Initializing child */
static int child_init (int rank)
{
    	if((fifo_fd = open(wsdb_fifo_name,O_WRONLY,0777)) <= 0){
    		LOG(L_ERR, "ERROR: racc: db_fifo writer is failed:%s ?\n",wsdb_fifo_name);
    		return -1;
    	}
		if(is_billing_flag){
			if(sem_init(&billing_semid,0,0) == -1){ /* initilizing semaphore */
				LOG(L_ERR,"ERROR: semaphore Inititilization failed\n");
				return -1;
			}
			pthread_mutex_init(&billing_datalock,NULL);
			create_billing_thread();
		}

	/* Added for stir-shaken cdr */
	if((stir_fifo_fd = open(stir_fifo_name,O_WRONLY,0777)) <= 0){
    		LOG(L_ERR, "ERROR: racc: stir_fifo writer is failed:%s ?\n",stir_fifo_name);
    		return -1;
    	}

	if(sem_init(&stir_semid,0,0) == -1){ /* initilizing semaphore */
		LOG(L_ERR,"ERROR: semaphore Inititilization failed\n");
		return -1;
	}
	pthread_mutex_init(&stir_datalock,NULL);
	stir_create_billing_thread();

    	return 0;
}

static void racc_onreq( struct cell* t, int type, struct tmcb_params *ps )
{
	int tmcb_type;

	tmcb_type = TMCB_REQUEST_FWDED | TMCB_RESPONSE_OUT | TMCB_RESPONSE_IN; 
	if (tmb.register_tmcb( 0, t, tmcb_type, tmcb_func, 0 )<=0) {
		LOG(L_ERR,"ERROR:racc:racc_onreq: cannot register additional callbacks\n");
		return;
	}
		
}
static void tmcb_func( struct cell* t, int type, struct tmcb_params *ps )
{
	if (type&TMCB_REQUEST_FWDED) {
		racc_onrequestfwded	(t, ps->req,  ps->code, ps->param);
	}
	else if (type & TMCB_RESPONSE_OUT) {
		/*racc_onrespout (t, ps->req, ps->code, ps->param);*/
		racc_onrespout (t, ps->rpl, ps->code, ps->param);	
	}
	else if ( type & TMCB_RESPONSE_IN  ) {
        	racc_onrespin (t, ps->rpl, ps->code, ps->param);		
	}
}

/* Destroy function */
static void destroy (void)
{
	if(is_billing_flag){
		pthread_mutex_destroy(&billing_datalock); /* destroying mutex */
		sem_destroy(&billing_semid);
	}

	pthread_mutex_destroy(&stir_datalock); /* destroying mutex */
	sem_destroy(&stir_semid);
	
}

/* Replace special characters with spaces */
int Replace_spacial_chars_with_space(char *cBuf, int nLength) {
	int i = 0;
	if(_specialchars == NULL) {
		LOG(L_ERR,"[Replace_spacial_chars_with_space] _specialchars is NULL. check wsicp configuration file\n");
		return -1;
	}
	if((cBuf == NULL) || (nLength <= 0)) {
		LOG(L_ERR,"[Replace_spacial_chars_with_space] we got cname Buff NULL or Length of Buff is less than or ZERO. so returning from here\n");
		return -1;
	}
	for(i=0; i<nLength; i++) {
		if(strchr(_specialchars,cBuf[i]) != NULL) {
			cBuf[i] = ' ';
		}
	}
	return 0;
}

/*
 * Request Forward events: INVITE, CANCEL and BYE
 */
static void racc_onrequestfwded (struct cell *t, struct sip_msg *msg, int code, void *param) 
{
	str uri, wsacdaacallerid;
	struct sip_uri puri;
	struct sip_uri parsed_uri;
	int length = 0,hflag = 0,lcr_flag = 0;
	char from_uri[256]="",*ptr= NULL,*ptr1 = NULL,buffer[128]="";
	struct hdr_field *hf = NULL;
	struct lump *tmp = NULL, *t1 = NULL;
	char DialedNumber[256] = "",*cPtr = NULL,QName[64]="",cBuffer[256]="";
	char cToUri[256]="", cToken[64]="", cLocalCancel[128] = "", wsacdaacallerid_header[128] = "";
	
 	memset(&racc_call , 0 , sizeof(struct racc_call_details));
	wsacdaacallerid.s = "WSACDAACALLERID";
	wsacdaacallerid.len = strlen("WSACDAACALLERID");

    if(msg && msg->extra_info.nOtherDataCenterFlag == 1 ){
        LOG(L_ERR,"[racc_onrequestfwded] OtherDataCenter Call So No Billing Here For This INVITE !!!!!!!\n");
        return ;
    }

	/*Added changes to get proper billing in case of rejected calls */
	if(!strncmp(t->method.s,"CANCEL",6)){
		if(strstr(t->method.s,"sip:fgrp-")){
 
			/* Added below changes to fix shared memory leak  --oct-11-2019*/
			if(strstr(t->method.s,"sip:fgrp-0-")){

				if(msg == NULL) {
					LOG (L_ERR, "[racc_onrequestfwded] msg structure is NULL...\n");
					return ;
				}

  				if (parse_headers (msg, HDR_TO, 0) == -1) {
					   LOG (L_ERR, "[racc_onrequestfwded] Error parsing To header...\n");
					   return ;
				}
				if(!msg->to){
					LOG(L_ERR , "[racc_onrequestfwded] To header field missing !!!!  \n");
					return ;
				}
				if (parse_uri (get_to(msg)->uri.s, get_to(msg)->uri.len, &parsed_uri) < 0) {
					LOG(L_ERR , "[racc_onrequestfwded]  parsing to uri is failed !!!!  \n");
					return ;
				}

				strncpy(cToUri, parsed_uri.user.s, sizeof(cToUri)-1);

				if((ptr = strchr(cToUri,'@'))) {
					*ptr = '\0';
					if((ptr = strrchr(cToUri,'-')) && ++ptr) {
						strncpy(cToken,ptr,sizeof(cToken)-1);
						clear_forkgroup_shared_memory(msg,cToken,NULL);
					}
				}
			}
			/* End */
			return;
		}
	}

	/* adding for handling local cancel */
	if(!msg){
		/* Local(self) cancel processing  --vravi */
		if(code == METHOD_CANCEL){
			if(t){
				racc_call.method = RACC_CANCELLED;
				if((t->callid.s && t->callid.len > 0) && strlen(t->local_branch) >0 ){
					memset(racc_call.callid,0,sizeof(racc_call.callid));
					snprintf(racc_call.callid, sizeof(racc_call.callid)-1,"%.*s", t->callid.len-11,t->callid.s+9);
					memset(racc_call.branch,0,sizeof(racc_call.branch));
					snprintf(racc_call.branch,sizeof(racc_call.branch)-1,"%.*s",strlen(t->local_branch),t->local_branch);
					/*Added to for acdext deletion in case of ACD calls for local cancel */
					memset(racc_call.to,0,sizeof(racc_call.to));
					snprintf(racc_call.to,sizeof(racc_call.to)-1,"%.*s",t->to.len-2,t->to.s);
					racc_call.resp_cause = F_RINGTIMEOUT;
					racc_call.restype = REQUEST_FWDED_LOCAL;
					racc_call.timestamp = time(NULL);
                    racc_call.nMissedCallFlag = 0 ;
					if(is_billing_flag){
						insert_billing_session(&racc_call);
					}else{
						if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0)
						{
							LOG (L_ERR, "[racc_onrequestfwded]ERROR (racc)req writing to fifo is failed?!IN LOCAL CANCEL\n");
						}
					}
				}
			}
		}
		LOG(L_ERR,"[racc_onrequestfwded] LOCAL CANCEL Return...\n");
		return;
	}
	
	racc_call.restype = REQUEST_FWDED;

	STATUS=RACC_UNDEF;
	switch (msg->REQ_METHOD) {
			
		case METHOD_INVITE:
			
			STATUS = RACC_INVITE;
		/* Added for checking "WSACDAACALLERID" header and send 'iscalleriddisplay' parameter in xml protocol for import contact project -- Prashant/Surekha -13-May-2021 */
			if(is_header_exists(msg, wsacdaacallerid.s, wsacdaacallerid.len, wsacdaacallerid_header, sizeof(wsacdaacallerid_header)) == 1){
				racc_call.nWSacdaacallerid = 1;
			}

			break;
	
		case METHOD_CANCEL:
			STATUS = RACC_CANCELLED;
			break;
			
		case METHOD_BYE:
			STATUS = RACC_TERMINATING;
			break;
			
		default:
			STATUS = RACC_UNDEF;
			break;
	}
	
	if (RACC_UNDEF == STATUS){
//		LOG(L_ERR,"[racc_onrequestfwded]request method is undefined:%.*s\n" ,msg->first_line.u.request.uri.len, msg->first_line.u.request.uri.s);
		return;
	}

	/* RPID - Remote Party ID */
	tmp = msg->add_rm;
	if(tmp) {
		for(t1=tmp->next;t1;t1=t1->next) {
			if(t1->before && t1->before->len > 0 && (t1->before->u.value) && (strlen(t1->before->u.value)>0) && !strncmp(t1->before->u.value,"Remote-Party-ID:",16)) {
				ptr = strchr(t1->before->u.value,':');
				if(ptr && ++ptr) {
					memset(cBuffer, 0, sizeof(cBuffer));
					strncpy(cBuffer, ptr, sizeof(cBuffer)-1);
					ptr = strchr(cBuffer,'\r');
					if(ptr) {
						*ptr = '\0';
					}
					ptr = strchr(cBuffer,'\n');
					if(ptr) {
						*ptr = '\0';
					}
					Replace_spacial_chars_with_space(cBuffer,strlen(cBuffer));
					memset(racc_call.rpid,0,sizeof(racc_call.rpid));
					strncpy(racc_call.rpid,cBuffer,sizeof(racc_call.rpid)-1);
				}
			}
		}
	}

	/* Request URI */
	if (msg->new_uri.s && msg->new_uri.len)	uri = msg->new_uri;
	else uri = msg->first_line.u.request.uri;

	if ((parse_uri (uri.s, uri.len, &parsed_uri)) < 0) {
		LOG (L_ERR, "[racc_onrequestfwded]Failed parsing uri\n"); 
		return;
	}
	else {
		/*memset(registraripport , 0 , sizeof(registraripport));
		sprintf(registraripport,"%.*s:%.*s" , parsed_uri.host.len , \
					parsed_uri.host.len?parsed_uri.host.s:"", parsed_uri.port.len , parsed_uri.port.len?parsed_uri.port.s:""  ); 
		ret = ws_isinlist ( msg , "1",registraripport);
		if( ret == 1 ) { 
			return ; 
		} */
		strncpy (racc_call.uri, parsed_uri.user.s, parsed_uri.user.len);
	}
	/*LOG (L_ERR, "[racc][pandora] URI  : %s\n", racc_call.uri);*/
	/* Call ID */ 
	if (msg->callid == NULL) {
		if (-1 == parse_headers (msg, HDR_CALLID, 0)) return;
		if (NULL == msg->callid) return;
	}	
	
	if (! msg->callid || ! msg->callid->body.s)	{
		LOG (L_ERR, "[racc_onrequestfwded]Huh? Call without a callid?!\n");
		return;
	}
	else {
		strncpy (racc_call.callid, msg->callid->body.s, msg->callid->body.len);
	}


	/* To and Tag */
	bzero (&uri, sizeof (uri));
	bzero (&puri, sizeof (puri));
	if(!msg || !msg->to){
		LOG(L_ERR , "[racc_onrequestfwded] To header field missing !!!!  \n");
		return;
	}
	uri = get_to(msg)->uri;	
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
	
		LOG (L_ERR, "[racc_onrequestfwded]Huh? Call without a To header?!\n");
		return;
	}
	else {
		strncpy (racc_call.to, puri.user.s, puri.user.len);
		uri = get_to(msg)->tag_value;
		if (uri.s && uri.len) {
			strncpy (racc_call.totag, uri.s, uri.len);
		}
	}

	/* From and Tag */
	bzero (&uri, sizeof (uri));
	bzero (&puri, sizeof (puri));


	if (parse_from_header(msg) == -1) {
		LOG (L_ERR, "[racc_onrequestfwded]Oh! Failed parsing from header\n");
		return;
	}
	uri = get_from(msg)->uri;
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG (L_ERR, "[racc_onrequestfwded]Huh? Call without a From header?!\n");
		return;
	}
	else {		
		strncpy (racc_call.from, puri.user.s, puri.user.len);
		uri = get_from(msg)->tag_value;
		if (uri.s && uri.len) {
			length = sizeof(racc_call.fromtag);
			if(length > uri.len){
				strncpy(racc_call.fromtag, uri.s, uri.len);
			}else{
				strncpy(racc_call.fromtag, uri.s, length - 1);
			}
		}
	}

	
	if(msg->add_to_branch_len > 0){
		strncpy (racc_call.branch, msg->add_to_branch_s, msg->add_to_branch_len);
//		LOG(L_ERR,"[racc_onrequestfwded].....branch id:%s\n",racc_call.branch);
	}else{
		LOG (L_ERR, "[racc_onrequestfwded]Huh? Call without a branch in via?!\n");
	}	
	
	if (! strncmp (t->method.s, "INVITE", 6)) {
		if(msg->contact != NULL){
			parse_headers (msg, HDR_CONTACT, 0);
			if(msg->contact->body.len > 0){
				strncpy (racc_call.contact, msg->contact->body.s, msg->contact->body.len);
			}
		}
	}

	/* Branch */
/*	parse_headers (msg, HDR_VIA|HDR_VIA2, 0);
	if (msg->via1) {
		branch = msg->via1->branch;
		if (branch && branch->value.s && branch->value.len) {
			strncpy (racc_call.branch, branch->value.s, branch->value.len);
		}
		LOG (L_ERR, "2.Branch msg->via1:  %s\n", racc_call.branch); 
	}
	else {
		// We dont care even if the branch is absent 
		LOG (L_ERR, "Huh? Call without a branch in via?!\n"); 
	}*/
	racc_call.method = STATUS;
	racc_call.phone = 0;

	/* For fixing  1062 errors  we are getting User agent in case of BYE request --vravi*/
	if(msg->REQ_METHOD == METHOD_BYE && ( -1 != parse_headers(msg ,HDR_USERAGENT,0) && (msg->user_agent && msg->user_agent->body.len>0)))
	{
		snprintf(racc_call.user_agent,sizeof(racc_call.user_agent)-1,"%.*s",msg->user_agent->body.len , msg->user_agent->body.s);
	}

	/* added for missed call savings */
	if(msg->REQ_METHOD == METHOD_CANCEL){
		racc_call.resp_cause = F_CANCEL;
		memset(buffer,0,sizeof(buffer));
		memset(cLocalCancel, 0, sizeof(cLocalCancel));

		/* MSICP will generate LocalCancel header as "Yes" only in routingplan to ringtime case 
		 * UMS will generate LocalCancel header as "Forward" only from streams Forward case 
		 * MSICP will generate Reason header when call connected/voicemail at someother seq in routingplan */

		if(is_header_exists(msg,"LocalCancel",11,cLocalCancel,sizeof(cLocalCancel))){
			if(!strcmp(cLocalCancel,"Yes")){
				racc_call.resp_cause = F_ROUTINGRINGTIMEOUT;
			}else if(!strcmp(cLocalCancel,"Forward")){
				racc_call.resp_cause = F_CANCELFORWARD;
			}
		}

		if(is_header_exists(msg,"Reason",6,buffer,sizeof(buffer))){/*Added changes to know pickup calls in*53,*54 scenarios*/
			if(strstr(buffer,"Call picked by another user")){
				racc_call.resp_cause = F_PICKUP;
			}else if(racc_call.resp_cause != F_CANCELFORWARD && strstr(buffer,"Call completed elsewhere")){  
				/* Added to fix bugid 82527 (Hunt group report patch) -- sep-11-2020*/	
				racc_call.resp_cause = F_CALL_COMPLETED_ELSEWHERE;
			}
		} 
		LOG(L_WARN,"[racc_onrequestfwded]  %s, %s, %d, %s \n" , cLocalCancel, buffer, racc_call.resp_cause,racc_call.callid);
	}

	if(!strncmp(racc_call.to,"lcr-",4)) {
		lcr_flag = 1;
	}

	if (! strncmp (t->method.s, "INVITE", 6)) {

		for(hf = msg->headers; hf; hf=hf->next){
			if(hf->name.len == 8 && !(strncasecmp(hf->name.s,"Replaces",8)) && hf->body.len > 0){
				LOG(L_WARN," As this is WARMTRANSFER call, getting INVITE replaces with uri : '%s', So not inserting into database\n" , racc_call.uri);
				return;
			}
		}
		if(msg->nRoutingPlanExtFlag == 1) {
			racc_call.nRoutingPlanExtFlag= msg->nRoutingPlanExtFlag;
            strncpy(racc_call.cAgentId, msg->cAgentId, sizeof(racc_call.cAgentId) - 1);
            strncpy(racc_call.cGrpName, msg->cGrpName, sizeof(racc_call.cGrpName) - 1);
		}
		
		for (hf=msg->headers; hf; hf=hf->next) {
			if(((hf->name.len == WSWEBCALLINFO) && (!strncasecmp (hf->name.s,"WSWEBCallInfo",WSWEBCALLINFO)))){
				hflag = WSWEBCALLINFO;
				memset(DialedNumber, 0, sizeof(DialedNumber));
				snprintf(DialedNumber, sizeof(DialedNumber)-1, "%.*s", hf->body.len, hf->body.s);
				snprintf(from_uri, sizeof(from_uri)-1, "%.*s", hf->body.len, hf->body.s);
				break;
			}else if(((hf->name.len == WSACDWEB)&& (!strncasecmp (hf->name.s,"X-WSACDWEB",WSACDWEB)))){
				hflag = WSACDWEB;
				memset(DialedNumber, 0, sizeof(DialedNumber));
				snprintf(DialedNumber, sizeof(DialedNumber)-1, "%.*s", hf->body.len, hf->body.s);
				break;
			}
		}
		if(strlen(DialedNumber) > 0){
			if((hflag == WSWEBCALLINFO) && (cPtr = strchr(DialedNumber,'#')) && ++cPtr){
				strncpy(racc_call.DialedNumber,cPtr,sizeof(racc_call.DialedNumber)-1);
			}else if((hflag == WSACDWEB) && (cPtr = strchr(DialedNumber,'#'))){
				*cPtr = '\0';
				/*Added changes for getting QNAME in case of acd calls*/
				if(cPtr && ++cPtr){
					strncpy(QName,cPtr,sizeof(QName)-1);
					if((cPtr = strchr(QName,'#')))
						*cPtr = '\0';
				}
				strncpy(racc_call.DialedNumber,DialedNumber,sizeof(racc_call.DialedNumber)-1);
				strncpy(racc_call.QName,QName,sizeof(racc_call.QName)-1);
			}
		}

		/*Added for getting proper from_uri & CNAM for PSTN/INTE Calls -- 08-08-2018*/
		if(lcr_flag == 1) {
			racc_call.is_direct_call = 1;
			for (hf=msg->headers; hf; hf=hf->next) {
				if(!strncasecmp (hf->name.s, "Transferred_By", 14)) {
					memset(cBuffer,0,sizeof(cBuffer));
					snprintf(cBuffer, sizeof(cBuffer)-1, "%.*s", hf->body.len, hf->body.s);
					break;
				}
			}
			if(!strcmp(cBuffer,"1")){
				if(isalpha(racc_call.from[0])){/*original from uri*/
					memset(from_uri,0,sizeof(from_uri));
					strncpy(from_uri,racc_call.from,sizeof(from_uri)-1);
				}
			}else if(strlen(cBuffer)>0){/*getting from uri from Transferred_By header*/
				memset(from_uri,0,sizeof(from_uri));
				strncpy(from_uri,cBuffer,sizeof(from_uri)-1);
			}
			if(ptr = strchr(from_uri, '#')) {
				*ptr = '\0';
			}

			memset(racc_call.from,0,sizeof(racc_call.from));
			strncpy(racc_call.from,from_uri,sizeof(racc_call.from)-1);

			memset(racc_call.callername,0,sizeof(racc_call.callername));
			strncpy(racc_call.callername,from_uri,sizeof(racc_call.callername)-1);
		}

		/*Added for getting CNAM*/
		if(lcr_flag == 0 && msg->from && msg->from->body.s && msg->from->body.len > 0){
			memset(from_uri,0,sizeof(from_uri));
			snprintf(from_uri, sizeof(from_uri)-1, "%.*s", msg->from->body.len, msg->from->body.s);
			if((ptr=strchr(from_uri,'"')) && ++ptr){
				if((ptr1=strchr(ptr,'"'))){
					*ptr1='\0';
						strncpy(racc_call.callername,ptr,sizeof(racc_call.callername)-1);
				}
			}
		}
		for (hf=msg->headers; hf; hf=hf->next) {
			if(((hf->name.len == 18)  && (!strncasecmp (hf->name.s,"WSInterAccountCall",18)))){
				snprintf(racc_call.AccUser, sizeof(racc_call.AccUser)-1, "%.*s", hf->body.len, hf->body.s);
				break;
			}
		}
	}

	racc_call.timestamp = time(NULL);
    racc_call.nMissedCallFlag = 0 ;
	if(is_billing_flag){
		insert_billing_session(&racc_call);
	}else{
		if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0)
		{
			LOG (L_ERR, "[racc_onrequestfwded]ERROR (racc)req writing to fifo is failed?!\n");
		}
	}
	return;
}

/* Routine for handling the RESPONSEs */
static void racc_onrespout (struct cell *t, struct sip_msg *msg, int code, void *param) 
{
	int res_status;
	str uri;
	struct sip_uri puri;
	char contact_buf[256] = "";
	char temp_contact[256]="";
	char *name,*temp;
	struct via_param *branch; 
	char pocstring[1024] = "";
	struct sip_msg * origmsg = NULL;
	int phone = 1;  /* 0 means softphone/ip phone, 1 means analog/cell phone */
	struct hdr_field *hf = NULL;
	char hold_buf[256] = "",*ptr = NULL,macid_len[16]="";
	char cSrcIp[32] = "", cDstIp[32]="";
	int nSrcPort = 0, nDstPort = 0;
  	memset(&racc_call , 0 , sizeof(struct racc_call_details));
	racc_call.restype = RESPONSE_OUT;
	if(!msg){
		LOG(L_ERR , "[racc_onrespout] Msg structure is NULL,So not processing further!!!!!!! ");
		return;
	}
	/*This code for junk number dialing-tulsi-04-06-08*/
	memset (pocstring , 0 , sizeof(pocstring));
	if (! t->uas.request || FAKED_REPLY == msg)
	{
		if (t != NULL )
		{
			char * pBranch = NULL;
			char * pTemp = NULL;
			if ( t->relaied_reply_branch >= 0) {
				char * msg_buffer = t->uac[t->relaied_reply_branch].request.buffer;
				int nLen = t->uac[t->relaied_reply_branch].request.buffer_len;
				if ( nLen > 0 && msg_buffer != NULL) {
					memcpy(pocstring, msg_buffer, nLen > sizeof(pocstring)?sizeof(pocstring) - 1:nLen);
					pBranch = strstr(pocstring, "branch");
					if ( pBranch ) {
						pTemp = strstr(pBranch, "\r\n");
						if ( pTemp != NULL ) {
							*pTemp = 0;
						}
						pTemp = strchr(pBranch,'=');
						if(pTemp != NULL){
							pTemp ++;
							LOG(L_ERR, "[racc_onrespout] Found Branch: %s\n", pTemp);
						}
					}
				}
			}
			if ( t->uas.request != NULL )
			{
				origmsg = t->uas.request;
				if(origmsg != NULL)
				{
					//racc_call.nNoBilling = param;
					/*Getting the branch value*/
					parse_headers(origmsg, HDR_VIA|HDR_VIA2,0);
					branch =  origmsg->via1->branch;
					if ( branch ) {
						if(((branch->value.s != NULL) && (branch->value.len > 0)))
							strncpy (racc_call.branch, branch->value.s, branch->value.len);
					}
					/*Getting the Call-Id value*/
					if (origmsg->callid == NULL) {
						if (-1 == parse_headers (origmsg, HDR_CALLID, 0)) return;
						if (NULL == origmsg->callid) return;
					}
					if (! origmsg->callid || ! origmsg->callid->body.s){
						LOG (L_ERR, "[racc_onrespout]Huh? Call without a callid?!\n");
						return;
					}
					else {
						if(((origmsg->callid->body.s != NULL) && (origmsg->callid->body.len > 0))){
							strncpy(racc_call.callid, origmsg->callid->body.s, origmsg->callid->body.len);
						}
					}
					/*Getting the tags*/
					bzero (&uri, sizeof (uri));
					bzero (&puri, sizeof (puri));
					if (parse_from_header(origmsg) == -1) {
						LOG (L_ERR, "[racc_onrespout]Oh! Failed parsing from header\n");
						return;
					}
					
					uri = get_from(origmsg)->uri;
					if(((uri.s !=NULL) && (uri.len > 0 ))){
						if (parse_uri(uri.s, uri.len, &puri) < 0) {
							LOG (L_ERR, "[racc_onrespout]Huh? Call without a From header?!\n");
							return;
						}
						else {
								strncpy (racc_call.from, puri.user.s, puri.user.len);
                                uri = get_from(origmsg)->tag_value;
								if (uri.len) {
									strncpy (racc_call.fromtag, uri.s, uri.len);
								}
						}
					}else{
						LOG (L_ERR, "[racc_onrespout]Huh? Call without a From header?!\n");
						return;
					}
				}

				if(code == 603 && t->nStirReject > 0) {
					racc_call.nLcrReject = t->nStirReject;
					t->nStirReject = 0;
				}

				STATUS = RACC_REJECTED;
				racc_call.method = STATUS;
				/*added for missed call changes */
				racc_call.resp_cause = F_REJECT;
				racc_call.timestamp = time(NULL);
                racc_call.nMissedCallFlag = 0 ;
				LOG(L_ERR,"[racc_onrespout]For INVITE,CANCEL methods UDP writing is hapening..:%.*s\r\n",t->method.len , t->method.s);
				/*Don't write blindly..check wether the RESPONSE for INVITE..Tulsi-23-12-08: bugid: 5176*/
				if ((!strncmp (t->method.s, "INVITE", 6)) || (!strncmp (t->method.s, "CANCEL", 6))) {
					LOG(L_ERR,"[racc_onrespout]Junk packet GOT WRITE:%d\r\n",STATUS);
					if(is_billing_flag){
						insert_billing_session(&racc_call);
					}else{
						if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0){
							LOG (L_ERR, "[racc_onrespout]ERROR (racc)req writing to fifo is failed?!\n");
						}
					}
					if ( pTemp != NULL ) {
						strcpy(racc_call.branch, pTemp);
						LOG(L_ERR,"GOT WRITE UDP for Branch:%s\r\n",racc_call.branch);
						if(is_billing_flag){
							insert_billing_session(&racc_call);
						}else{
							if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0){
								LOG (L_ERR, "[racc_onrespout]ERROR (racc)req writing to fifo is failed?!\n");
							}
						}
					}
				}
			}
			return;
		}
	}
	if (! t->uas.request || FAKED_REPLY == msg) {
		LOG (L_ERR, "[racc_onrespout]ERROR (racc): Huh? Reply without a request?!\n");
		return;
	}

	/* Call ID */
	if (msg->callid == NULL) {
		if (-1 == parse_headers (msg, HDR_CALLID, 0)) return;
		if (NULL == msg->callid) return;
	}	
	
	if (! msg->callid || ! msg->callid->body.s)	{
		LOG (L_ERR, "[racc_onrespout]Huh? Call without a callid?!\n");
		return;
	}
	else {
		strncpy(racc_call.callid, msg->callid->body.s, msg->callid->body.len);
	}

	/* Branch */
	parse_headers (msg, HDR_VIA|HDR_VIA2, 0);
	if(msg->via1){
		branch = msg->via1->branch;
		if (branch && branch->value.s && branch->value.len){
			strncpy (racc_call.branch, branch->value.s, branch->value.len);
			/*LOG(L_ERR,"[racc_onrespout]..............response out branch:%s\n",racc_call.branch);	*/
		}
	}else {
		/* We dont care even if the branch is absent */
		LOG (L_ERR, "[racc_onrespout]Huh! Call without a branch?!\n");
	}

	/* Added "own_ip" check logic to fix LTD reports issue(Missing leg in group forking case-TT 198806) --nov-4-2019 */
	memset(cSrcIp, 0, sizeof(cSrcIp));
	memset(cDstIp, 0, sizeof(cDstIp));
	strncpy(cSrcIp, ip_addr2a(&msg->rcv.src_ip), sizeof(cSrcIp)-1);
	strncpy(cDstIp, ip_addr2a(&msg->rcv.dst_ip), sizeof(cDstIp)-1);
	nSrcPort = msg->rcv.src_port;
	nDstPort = msg->rcv.dst_port;
		
	if((nSrcPort == nDstPort) && !strcmp(own_ip.s, cSrcIp) && !strcmp(cSrcIp, cDstIp)) {
		LOG (L_ERR, "[racc_onrespout] Not inserting into fifo..if we got '%d' response from self-ip. Callid:<%s>, branch <%s>\n", code, racc_call.callid, racc_call.branch);
		return;
	}
						
	res_status = code;
	STATUS = RACC_UNDEF;
	switch (res_status)	{
		case 180:
		case 183:
				STATUS = RACC_RINGING;
				break;
		case 200:

			if (! strncmp (t->method.s, "INVITE", 6)) {
				if(parse_headers (msg, HDR_CONTACT, 0)== -1) {
					LOG (L_ERR, "[racc_onrespout]ERROR Unable to Parse Contact HDR\n");
					return;
				}
				if(msg->contact){
					snprintf(contact_buf, sizeof(contact_buf) - 1,"%.*s",msg->contact->body.len, msg->contact->body.s);
					for (hf=msg->headers; hf; hf=hf->next) {
						if (hf->name.len != 11)continue;
						if(!strncasecmp (hf->name.s,"WSCHMetrics",11)){
							snprintf(hold_buf,sizeof(hold_buf)-1,"%.*s",hf->body.len,hf->body.s);
							if((ptr = strstr(hold_buf,"\r\n"))){
									*ptr = '\0';
							}
							break;
						}
					}
					if((strstr(hold_buf,"wschm;"))){
						strcat(contact_buf,hold_buf);
						STATUS = RACC_HOLD;
					}else{
						STATUS = RACC_ACCEPTED;
					}
					strncpy(racc_call.contact,contact_buf,sizeof(racc_call.contact)-1);
					name=strstr(contact_buf,"sip:");
					if (!name) break;
					name+=4;
					temp=strchr(name,'@'); 
					if (!temp) break;
					*temp='\0'; 
					bzero(temp_contact,0);
					strncpy(temp_contact, name, strlen(name));/*one bug is there. if we do copy like this we are missing the last charector. "strlen problem-"tulsi-27-05-08*/
					if (!isdigit(temp_contact[0])){
						phone = 0;
					}
				}else {
					LOG(L_ERR,"racc no contct body in 200 OK\n");
					return;
				}
			}	
			break;

		case 408:
			STATUS = RACC_TIMEDOUT;
			break;
		
		case 302:
		case 486:
		case 603:
		case 503:
			STATUS = RACC_REJECTED;
			break;
		
		default:
			STATUS = RACC_DEFAULT;
			break;
	}

	/* Setting nLcrReject for removing stuck when WSuser blind transfer the call to across account */
	if(code == 603 && t->nStirReject > 0) {
		racc_call.nLcrReject = t->nStirReject;
		t->nStirReject = 0;
	}
	
	/* Make life easy */
	if (STATUS == RACC_UNDEF){
		LOG (L_ERR, "[racc_onrespout]method of response is undefined!:%s\n",msg->first_line.u.request.uri.s);
		return;
	}
	racc_call.timestamp = time(NULL);

	/* To and Tag */
	bzero (&uri, sizeof (uri));
	bzero (&puri, sizeof (puri));
	if(!msg || !msg->to){
		LOG(L_ERR , "[racc_onrespout] To header field missing !!!!  \n");
		return;
	}
	uri = get_to(msg)->uri;
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG (L_ERR, "[racc_onrespout]Huh? Call without a To header?!\n");
		return;
	}
	else {
		strncpy (racc_call.to, puri.user.s, puri.user.len);
		uri = get_to(msg)->tag_value;
		if (uri.s && uri.len) {
			strncpy (racc_call.totag, uri.s, uri.len);
		}
	}	
	
	/*LOG(L_WARN,"RACC::Response To_TAG: %s\n",ttag_field);*/
	/* From and Tag */
	bzero (&uri, sizeof (uri));
	bzero (&puri, sizeof (puri));
	if (parse_from_header(msg) == -1) {
		LOG (L_ERR, "[racc_onrespout]Oh! Failed parsing from header\n");
		return;
	}
	
	uri = get_from(msg)->uri;
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG (L_ERR, "[racc_onrespout]Huh? Call without a From header?!\n");
		return;
	}
	else {		
		strncpy (racc_call.from, puri.user.s, puri.user.len);
		uri = get_from(msg)->tag_value;
		if (uri.len) {
			strncpy (racc_call.fromtag, uri.s, uri.len);
		}
	}
	
	if(STATUS == RACC_ACCEPTED){
		snprintf(macid_len,sizeof(macid_len)-1,"%d",sizeof(racc_call.macid));
		memset(racc_call.macid,0,sizeof(racc_call.macid));
		_get_macid(msg,racc_call.macid,macid_len);
	}

	racc_call.method = STATUS;
	racc_call.phone = phone;
	racc_call.timestamp = time(NULL);
    racc_call.nMissedCallFlag = 0 ;
    		
	if(is_billing_flag){
		insert_billing_session(&racc_call);
	}else{
		if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0)
			LOG (L_ERR, "[racc_onrespout]ERROR (racc)req writing to fifo is failed?!\n");
	}
	return;
}

int ws_missedcall_entry_for_rejected_calls (struct sip_msg *msg, char *uri, char *cSix11extensionsitename) {
    str uri1;
    struct sip_uri puri;
    struct via_param *branch;
    int len=0;
    char *ptr = NULL ;
    char cContactData[256]="";
    char cHeaderValue[128] = "";

    memset(&racc_call , 0 , sizeof(struct racc_call_details));  
    len = strlen(uri);
    if(len) {
        strncpy (racc_call.uri, uri, len);
    }

    racc_call.nMissedCallFlag = 1 ;
    if(cSix11extensionsitename != NULL && strlen(cSix11extensionsitename) > 0  &&  !strncmp(cSix11extensionsitename,"Voicemail:",10)){
        // example-> Voicemail:vcgsvcvsdgvh Here we are identifying ths case from block destination as voicemail and separating data from buffer  [17/08/2019]
        cSix11extensionsitename = cSix11extensionsitename + 10;
        racc_call.nMissedCallFlag = 2 ;
    }
    if( cSix11extensionsitename != NULL && strlen(cSix11extensionsitename) > 0){
        snprintf(racc_call.cSix11extensionsitename, sizeof(racc_call.cSix11extensionsitename)-1,"%s", cSix11extensionsitename);
	if(!strncmp(cSix11extensionsitename, "SPAM",4)) {
		if(is_header_exists(msg,"X-Verification-Info",19,cHeaderValue,sizeof(cHeaderValue))){
			if(strstr(cHeaderValue,"rep-score:yes")){
				if(!strncmp(cSix11extensionsitename, "SPAM#1",6)) {
					racc_call.reasoncode =  F_SPAM_PLAY_MSG_AND_DISCONNECT;
				}else {
					racc_call.reasoncode =  F_SPAM_REJECT;
				}
			}else{
				racc_call.reasoncode =  F_BLACKLISTED;
			}
		}
		racc_call.nMissedCallFlag = 3;
	}
    }  
    /* Call ID */
    if (msg->callid == NULL) {
        if (-1 == parse_headers (msg, HDR_CALLID, 0)) return 1;
        if (NULL == msg->callid) return 1;
    }


    if (! msg->callid || ! msg->callid->body.s) {
        LOG (L_ERR, "[ws_missedcall_entry_for_rejected_calls]Huh? Call without a callid?!\n");
        return 1;
    }
    else {
        strncpy(racc_call.callid, msg->callid->body.s, msg->callid->body.len);
    }

    /* To and Tag */
    bzero (&uri1, sizeof (uri1));
    bzero (&puri, sizeof (puri));
    if(msg->contact->body.len > 0){
        memset(cContactData, 0, sizeof(cContactData));
        snprintf(cContactData, sizeof(cContactData)-1,"%.*s", msg->contact->body.len, msg->contact->body.s);
        if((ptr = strchr(cContactData,'<')))
        {
            if(++ptr)
            {
                memset(racc_call.contact, 0, sizeof(racc_call.contact));
                strncpy(racc_call.contact, ptr, sizeof(racc_call.contact)-1);
                if((ptr = strrchr(racc_call.contact,'>')))
                {
                    *ptr = '\0';
                }
            }
        }
    }

    uri1 = get_to(msg)->uri;
    if (parse_uri(uri1.s, uri1.len, &puri) < 0) {
        LOG (L_ERR, "[ws_missedcall_entry_for_rejected_calls]Huh? Call without a To header?!\n");
        return 1;
    }
    else {
        strncpy (racc_call.to, puri.user.s, puri.user.len);
        uri1 = get_to(msg)->tag_value;
        if (uri1.s && uri1.len) {
            strncpy (racc_call.totag, uri1.s, uri1.len);
        }
    }

    bzero (&uri1, sizeof (uri1));
    bzero (&puri, sizeof (puri));
    if (parse_from_header(msg) == -1) {
        LOG (L_ERR, "[ws_missedcall_entry_for_rejected_calls]Oh! Failed parsing from header\n");
        return 1;
    }

    uri1 = get_from(msg)->uri;
    if (parse_uri(uri1.s, uri1.len, &puri) < 0) {
        LOG (L_ERR, "[ws_missedcall_entry_for_rejected_calls]Huh? Call without a From header?!\n");
        return 1;
    }
    else {
        strncpy (racc_call.from, puri.user.s, puri.user.len);
        uri1 = get_from(msg)->tag_value;
        if (uri1.len) {
            strncpy (racc_call.fromtag, uri1.s, uri1.len);
        }
    }

    /* Branch */
    parse_headers (msg, HDR_VIA|HDR_VIA2, 0);
    if(msg->via1){
        branch = msg->via1->branch;
        if (branch && branch->value.s && branch->value.len){
            strncpy (racc_call.branch, branch->value.s, branch->value.len);
            LOG(L_ERR,"[ws_missedcall_entry_for_rejected_calls]..............response out branch:%s\n",racc_call.branch);
        }
    }else {
        /* We dont care even if the branch is absent */
        LOG (L_ERR, "[ws_missedcall_entry_for_rejected_calls]Huh! Call without a branch?!\n");
    }
    racc_call.method = RACC_INVITE;
    racc_call.phone = 1;
    racc_call.timestamp = time(NULL);
    if(is_billing_flag){
        insert_billing_session(&racc_call);
    }else{
        if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0)
            LOG (L_ERR, "ERROR [ws_missedcall_entry_for_rejected_calls](racc)req writing to fifo is failed?!\n");
    }
    return 1;
}

int racc_process_moved_temporarily(struct sip_msg *msg, char *uri, char *p2) {
    	str uri1;
    	struct sip_uri puri;
    	struct via_param *branch;
    	int len=0;
		char from_uri[256]="",*ptr= NULL,*ptr1 = NULL,DialedNumber[256] = "";
		struct hdr_field *hf = NULL;
   
	if(!msg){
		LOG(L_ERR , "[racc_process_moved_temporarily] msg structure is NULL, So not processing !!!!!!!\n");
		return 1;
	}
     	memset(&racc_call , 0 , sizeof(struct racc_call_details));

    	len = strlen(uri);
    	if(len) {
        	strncpy (racc_call.uri, uri, len);
    	}
        if ( p2 != NULL && strlen(p2) > 0 && !strncmp(p2,"CALLERIDBLOCKVOICEMAIL",22)){
         racc_call.nVoicmailCalleridBlock  = 1;
        }

		if(p2 && strlen(p2)>0 && !strcmp(p2,"ClassOfService")) {
			racc_call.resp_cause = F_INV;
		}

    	/* Call ID */
    	if (msg->callid == NULL) {
        	if (-1 == parse_headers (msg, HDR_CALLID, 0)) return 1;
        	if (NULL == msg->callid) return 1;
    	}


    	if (! msg->callid || ! msg->callid->body.s) {
        	LOG (L_ERR, "[racc_process_moved_temporarily]Huh? Call without a callid?!\n");
        	return 1;
    	}
    	else {
			snprintf(racc_call.callid , sizeof(racc_call.callid)-1 , "%.*s" , msg->callid->body.len , msg->callid->body.s );
      //strncpy(racc_call.callid, msg->callid->body.s, msg->callid->body.len);
    	}

		LOG (L_ERR, "[racc_process_moved_temporarily] msg->callid->body.len '%d' , msg->callid->body.s '%s' \n",msg->callid->body.len , msg->callid->body.s);
		LOG (L_ERR, "[racc_process_moved_temporarily] racc_call.callid '%s' , msg->callid '%.*s' \n",racc_call.callid,msg->callid->body.len,msg->callid->body.s);
    	/* To and Tag */
    	bzero (&uri1, sizeof (uri1));
    	bzero (&puri, sizeof (puri));
	if(!msg || !msg->to){
		LOG(L_ERR , "[racc_process_moved_temporarily] To header field missing !!!!  \n");
		return 1;
	}

    	uri1 = get_to(msg)->uri;
    	if (parse_uri(uri1.s, uri1.len, &puri) < 0) {
        	LOG (L_ERR, "[racc_process_moved_temporarily]Huh? Call without a To header?!\n");
        	return 1;
    	}
    	else {
			snprintf(racc_call.to , sizeof(racc_call.to) , "%.*s" , puri.user.len , puri.user.s);
        	uri1 = get_to(msg)->tag_value;
        	if (uri1.s && uri1.len) {
            		strncpy (racc_call.totag, uri1.s, uri1.len);
        	}
    	}

    	bzero (&uri1, sizeof (uri1));
    	bzero (&puri, sizeof (puri));
    	if (parse_from_header(msg) == -1) {
        	LOG (L_ERR, "[racc_process_moved_temporarily]Oh! Failed parsing from header\n");
        	return 1;
    	}
    
     	uri1 = get_from(msg)->uri;
    	if (parse_uri(uri1.s, uri1.len, &puri) < 0) {
        	LOG (L_ERR, "[racc_process_moved_temporarily]Huh? Call without a From header?!\n");
        	return 1;
    	}
    	else {
        	strncpy (racc_call.from, puri.user.s, puri.user.len);
        	uri1 = get_from(msg)->tag_value;
        	if (uri1.len) {
            		strncpy (racc_call.fromtag, uri1.s, uri1.len);
        	}
    	}

    	/* Branch */
    	parse_headers (msg, HDR_VIA|HDR_VIA2, 0);
    	if(msg->via1){
        	branch = msg->via1->branch;
        	if (branch && branch->value.s && branch->value.len){
            		strncpy (racc_call.branch, branch->value.s, branch->value.len);
            		LOG(L_ERR,"[racc_process_moved_temporarily]..............response out branch:%s\n",racc_call.branch);
        	}
    	}else {
        	/* We dont care even if the branch is absent */
        	LOG (L_ERR, "[racc_process_moved_temporarily]Huh! Call without a branch?!\n");
    	}
	
		/*Added for getting CNAM*/
		if(msg->from && msg->from->body.s && msg->from->body.len > 0){
			memset(from_uri,0,sizeof(from_uri));
			snprintf(from_uri, sizeof(from_uri)-1, "%.*s", msg->from->body.len, msg->from->body.s);
			if((ptr=strchr(from_uri,'"')) && ++ptr){
				if((ptr1=strchr(ptr,'"'))){	
					*ptr1='\0';
					strncpy(racc_call.callername,ptr,sizeof(racc_call.callername)-1);
				}
			}
		}

		if(strlen(racc_call.uri) > 0 && (!strncmp(racc_call.uri,"opr-",4) || !strncmp(racc_call.uri,"ivr-",4) || !strncmp(racc_call.uri,"nop-",4))){
			racc_call.method = RACC_302_INVITE;
		}else{
			racc_call.method = RACC_TEMPMOVED;
		}

		if( racc_call.method == RACC_TEMPMOVED){
			for (hf=msg->headers; hf; hf=hf->next) {
				if(((hf->name.len == WSWEBCALLINFO) && (!strncasecmp (hf->name.s,"WSWEBCallInfo",WSWEBCALLINFO)))){
					memset(DialedNumber, 0, sizeof(DialedNumber));
					snprintf(DialedNumber, sizeof(DialedNumber)-1, "%.*s", hf->body.len, hf->body.s);
					if(strlen(DialedNumber) > 0){
						if((ptr = strchr(DialedNumber,'#')) && ++ptr){
							memset(racc_call.DialedNumber,0,sizeof(racc_call.DialedNumber));
							strncpy(racc_call.DialedNumber,ptr,sizeof(racc_call.DialedNumber)-1);
						}
					}
					break;
				}
			}
			for (hf=msg->headers; hf; hf=hf->next) {
				if(((hf->name.len == 18)  && (!strncasecmp (hf->name.s,"WSInterAccountCall",18)))){
					memset(racc_call.AccUser, 0,sizeof(racc_call.AccUser));
					snprintf(racc_call.AccUser, sizeof(racc_call.AccUser)-1, "%.*s", hf->body.len, hf->body.s);
					break;
				}
			}
		}

    	racc_call.phone = 1;
		racc_call.timestamp = time(NULL);
        racc_call.nMissedCallFlag = 0 ;	
		if(is_billing_flag){
			insert_billing_session(&racc_call);
		}else{
			if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0)
        		LOG (L_ERR, "ERROR [racc_process_moved_temporarily](racc)req writing to fifo is failed?!\n");
		}
    	return 1;
}

/*Added responsein function to handle direct responses to msicp --vravi/saidulu */
static void racc_onrespin (struct cell *t, struct sip_msg *msg, int code, void * param)
{
	struct sip_uri puri;
	struct via_param *branch;
   	char strmethod[24]="",cSource[128]="",*ptr=NULL;
	char lcontact[256]="",cViaips[256]="";
	str method;
	str uri;
	STATUS = RACC_UNDEF;
	int nGroup = 0;

	if(msg && (parse_headers(msg, HDR_CSEQ, 0) == -1)){
		LOG(L_ERR,"[racc_onrespin] ERROR:racc  Either MSG or cseq is not there\r\n");
		return;
	}	
	
	/*Gettind Cseq value*/
	bzero (&method, sizeof (method));
	method = get_cseq(msg)->method;
	if(method.s != NULL && method.len > 0){
		strncpy (strmethod, method.s, method.len);
	}else{
		LOG(L_ERR,"[racc_onrespin]ERROR:racc:racc_onreq <method.s is NULL>");
		return;
	}			

	if(strncmp (strmethod, "INVITE", 6) )
	{
		return;
	}
	
	memset(&racc_call , 0 , sizeof(struct racc_call_details));
	racc_call.restype = RESPONSE_IN;
			
	/*Getting the branch value*/
	parse_headers(msg, HDR_VIA|HDR_VIA2,0);
	branch =  msg->via1->branch;
	
	if ( branch ) {
		if(((branch->value.s != NULL) && (branch->value.len > 0))){
				strncpy (racc_call.branch, branch->value.s, branch->value.len);
		}
	}
	/*Getting the Call-Id value*/
	if (msg->callid == NULL) {
		if (-1 == parse_headers (msg, HDR_CALLID, 0)) return;
			if (NULL == msg->callid) return;
	}
	
	if (! msg->callid || ! msg->callid->body.s){
		LOG (L_ERR, "[racc_onrespin]Huh? Call without a callid?!\n");
		return;
	}
	else {
		if(((msg->callid->body.s != NULL) && (msg->callid->body.len > 0))){
			strncpy(racc_call.callid, msg->callid->body.s, msg->callid->body.len);
		}
	}
	
	/*Getting the tags*/
	bzero (&uri, sizeof (uri));
	bzero (&puri, sizeof (puri));
	if (parse_from_header(msg) == -1) {
		LOG (L_ERR, "[racc_onrespin]Oh! Failed parsing from header\n");
		return;
	}
			
	uri = get_from(msg)->uri;
	if(((uri.s !=NULL) && (uri.len > 0 ))){
		if (parse_uri(uri.s, uri.len, &puri) < 0) {
			LOG (L_ERR, "[racc_onrespin]Huh? Call without a From header?!\n");
			return;
		}
		else {
			strncpy (racc_call.from, puri.user.s, puri.user.len);
			uri = get_from(msg)->tag_value;
			if (uri.len) {
				strncpy (racc_call.fromtag, uri.s, uri.len);
			}
		}
	}else{
		LOG (L_ERR, "[racc_onrespin]Huh? Call without a From header?!\n");
		return;
	}
	bzero (&uri, sizeof (uri));
	bzero (&puri, sizeof (puri));
	if(!msg || !msg->to){
		LOG (L_ERR, "[racc_onrespin] No TO header present in the response.\n");
		return;
	}
	
	uri = get_to(msg)->uri;
	if (uri.len <= 0 || uri.s == NULL){
		LOG (L_ERR, "[racc_onrespin] No TO uri present.\n");
			return;
	}
	if (parse_uri(uri.s, uri.len, &puri) < 0) {
		LOG (L_ERR, "[racc_onrespin] Parsing TO URI failed.\n");
		return;
	} else {
		strncpy (racc_call.to, puri.user.s, puri.user.len);
	}
	if(parse_headers(msg, HDR_USERAGENT, 0) == -1){
		LOG(L_ERR,"[racc_onrespin] user_agent header is not found\r\n");
	}
	if (parse_headers (msg, HDR_CONTACT, 0)== -1) {
		LOG(L_WARN,"[racc_onrespin] Unable to parse Contact HDR \n");
	}
	
	if (msg->contact) {
		if(msg->contact->body.len > 0){
			snprintf(racc_call.contact, sizeof(racc_call.contact)-1,"%.*s", msg->contact->body.len,msg->contact->body.s);
		}
	}
	/*Added changes for delete acc_active stucks */
	if(ws_ctccall_flag && ((code == 486 || code ==603) && racc_call.fromtag && strstr(racc_call.fromtag,CTC_TAG) && racc_call.to && !strncmp(racc_call.to,CTCTYPE,4))){
		if((ptr=strchr(racc_call.to,'-')) && ++ptr){
			strncpy(cSource,ptr,sizeof(cSource)-1);
			if((ptr=strchr(cSource,'-')) && ++ptr){
				if(!isdigit(ptr[0]))
					racc_call.restype = RESPONSE_OUT;
			}
		}
	}

	/*Added changes to avoid query firing in case of nested fork group calls with 404 response --Anusha*/
    if(code == 404 && strstr(t->method.s,"sip:fgrp-")){
        return;
    }

	racc_call.timestamp = time(NULL);
    racc_call.nMissedCallFlag = 0 ;
	nGroup = (code/100) * 100;
	switch ( nGroup )
	{
		case 400:
		case 500:
		case 600:

			if(code == 603 && msg &&  msg->headers && msg->headers->name.s && (ptr = strstr(msg->headers->name.s, "X-Reject-Reason" ))) {
					if (strstr(ptr,"invalid-dialled-number")){
							racc_call.nLcrReject = 5;
					}else{
							racc_call.nLcrReject = 2;
					}
			}

				if((code == 486 || code == 603)){
					racc_call.method = RACC_REJECTED;
					racc_call.resp_cause=F_REJECT;
					if(code == 486) {
						racc_call.nResCode = 486;
					}
				}else if(code == 408){
					racc_call.method = RACC_TIMEDOUT;
					racc_call.resp_cause=F_TIMEOUT;
				}else if(code == 487){
					racc_call.resp_cause=F_DEFAULT;
					break;
				}else if((code == 403) && (strncmp(racc_call.to,"pst-",4) == 0)){
					racc_call.method = RACC_REJECTED;
					racc_call.nResCode = 403;
					racc_call.resp_cause=F_SERVICE_UNAVILABLE;					
				}else{
					racc_call.resp_cause=F_OTHER;
					racc_call.method = RACC_REJECTED;
				}

				if(is_billing_flag){
					insert_billing_session(&racc_call);
				}else{
					if(write(fifo_fd, (char *)&racc_call,sizeof(racc_call))< 0){
						LOG (L_ERR, "[racc_onrespin]ERROR (racc)req writing to fifo is failed?!\n");
					}
				}
				break;
		default:
				STATUS = RACC_UNDEF;
				break;
	}

	if( code == 200 ){
			memset(&racc_stir , 0 , sizeof(struct racc_stir_details));
			if (msg->contact) {
					if(msg->contact->body.len > 0){
							snprintf(racc_stir.contact_ip, sizeof(racc_stir.contact_ip) - 1, "%.*s", msg->contact->body.len, msg->contact->body.s);
					}
			}

			memset(cViaips , 0 , sizeof(cViaips));
			if(is_header_exists(msg, "X-ORIGINAL-VIAS", 15, cViaips, sizeof(cViaips))){
					strncpy(racc_stir.via_ips, cViaips, sizeof(racc_stir.via_ips)-1);
			}

			memset(lcontact , 0 , sizeof(lcontact));
			is_header_exists(msg, "LocalContact", 12 , lcontact, sizeof(lcontact));
			strncpy(racc_stir.lcontact_ip, lcontact, sizeof(racc_stir.lcontact_ip)-1);
			strncpy(racc_stir.req_uri, racc_call.to, sizeof(racc_stir.req_uri)-1);
			strncpy(racc_stir.callid, racc_call.callid, sizeof(racc_stir.callid)-1);
			insert_stir_session(&racc_stir);

	}
						
	return;
}


static int  write_to_fifo (struct racc_call_details *billing_data){

	if(billing_data && write(fifo_fd, (char *)billing_data,sizeof(racc_call))< 0)
	{
		LOG (L_ERR, "[write_to_fifo]ERROR (racc)req writing to fifo is failed?!IN LOCAL CANCEL\n");
		return 0;
	}

	return 1;
}

static int create_billing_thread(void){

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

 	if(pthread_create(&t,&attr,billing_thread,NULL)){ /* creating billing  thread */

		LOG (L_ERR, "[create_billing_thread Failed to create a threadn");
            return 0;
     }
	 return 1;
}


int print_queue_details()
{

	struct racc_call_details *temp = NULL;
	int count = 0;

	temp = billing_data_head ;

	while(temp != NULL){
		count++;
		temp = temp->next;
	}

	return count;
}


static void *billing_thread(void *data)
{

    struct racc_call_details *temp = NULL;

	LOG (L_ERR, "[billing_thread] Billing thread Started  !!!!");

    do{
		sem_wait(&billing_semid);  /* waiting for semaphore */
		pthread_mutex_lock (&billing_datalock);
		temp = billing_data_leaf;
		if(nEnableLogs == 1 && temp != NULL) {
			LOG (L_ERR, "[billing_thread] BILLING_TEST ===== callid : %s, method : %d, branch = %s count  %d  !!!!\n", temp->callid, temp->method, temp->branch, print_queue_details());
		}
		if(temp == billing_data_head){
			billing_data_head = NULL;
			pthread_mutex_unlock (&billing_datalock);
		}else { 
			pthread_mutex_unlock (&billing_datalock);
			if(temp && temp->prev ) {
				if (temp->prev != billing_data_head){
					temp->prev->next = NULL;		
				}else{
					pthread_mutex_lock (&billing_datalock);
					temp->prev->next = NULL;
					pthread_mutex_unlock (&billing_datalock);
				}
				billing_data_leaf =temp->prev;
			}
		}
		if(temp){
			write_to_fifo(temp);
			free(temp);
			temp = NULL;
		}
	}while(1);
	return NULL ;
}

static int insert_billing_session(struct racc_call_details *s){


  struct racc_call_details *temp = NULL,*billing_data = NULL;

    billing_data = (struct racc_call_details *) malloc (sizeof(struct racc_call_details));  /* Creating memory */
    if(billing_data == NULL){
		LOG (L_ERR, "[insert_billing_session] Unable to Allocate memory !!!!");
        return -1;
    }

	memset(billing_data , 0 , sizeof(struct racc_call_details));
	billing_data->method = s->method;
	billing_data->phone = s->phone;
	billing_data->resp_cause = s->resp_cause;
	billing_data->restype = s->restype;
	billing_data->timestamp = s->timestamp;
    billing_data->nMissedCallFlag= s->nMissedCallFlag ;
    billing_data->reasoncode= s->reasoncode;
    billing_data->nVoicmailCalleridBlock = s->nVoicmailCalleridBlock ;
    billing_data->nResCode = s->nResCode;
    billing_data->nLcrReject = s->nLcrReject;
    billing_data->nWSacdaacallerid = s->nWSacdaacallerid;
	if(s->uri && strlen(s->uri) > 0){
		strncpy(billing_data->uri,s->uri,sizeof(billing_data->uri)-1);
	}
	if(s->to && strlen(s->to) > 0){
		strncpy(billing_data->to,s->to,sizeof(billing_data->to)-1);
	}
	if(s->from && strlen(s->from) > 0){
		strncpy(billing_data->from,s->from,sizeof(billing_data->from)-1);
	}
	if(s->callid && strlen(s->callid)>0){
		strncpy(billing_data->callid,s->callid,sizeof(billing_data->callid)-1);
	}
	if(s->branch && strlen(s->branch) >0){
		strncpy(billing_data->branch,s->branch,sizeof(billing_data->branch)-1);
	}
	if(s->fromtag && strlen(s->fromtag) >0){
		strncpy(billing_data->fromtag,s->fromtag,sizeof(billing_data->fromtag)-1);
	}
	if(s->totag && strlen(s->totag) >0){
		strncpy(billing_data->totag,s->totag,sizeof(billing_data->totag)-1);
	}
	if(s->contact && strlen(s->contact) > 0){
		strncpy(billing_data->contact,s->contact,sizeof(billing_data->contact)-1);
	}
	if(s->user_agent && strlen(s->user_agent) >0){
		strncpy(billing_data->user_agent,s->user_agent,sizeof(billing_data->user_agent)-1);
	}
	if(s->macid && strlen(s->macid)>0){
		strncpy(billing_data->macid,s->macid,sizeof(billing_data->macid)-1);
	}
	if(s->callername && strlen(s->callername) >0){
		strncpy(billing_data->callername,s->callername,sizeof(billing_data->callername)-1);
	}
	if(s->DialedNumber && strlen(s->DialedNumber) >0){
		strncpy(billing_data->DialedNumber,s->DialedNumber,sizeof(billing_data->DialedNumber)-1);
	}
	if(s->QName && strlen(s->QName) >0){
		strncpy(billing_data->QName,s->QName,sizeof(billing_data->QName)-1);
	}
	if(s->AccUser && strlen(s->AccUser) >0){
		strncpy(billing_data->AccUser,s->AccUser,sizeof(billing_data->AccUser)-1);
	}
	if(s->is_direct_call) {
		billing_data->is_direct_call = s->is_direct_call;
	}
	if(s->nRoutingPlanExtFlag == 1 ) {
		billing_data->nRoutingPlanExtFlag= s->nRoutingPlanExtFlag;
        strncpy(billing_data->cAgentId, s->cAgentId, sizeof(billing_data->cAgentId) - 1);
        strncpy(billing_data->cGrpName, s->cGrpName, sizeof(billing_data->cGrpName) - 1);
	}
    if(s->cSix11extensionsitename && strlen(s->cSix11extensionsitename) >0){
        strncpy(billing_data->cSix11extensionsitename,s->cSix11extensionsitename,sizeof(billing_data->cSix11extensionsitename)-1);
    }
	if(s->rpid && strlen(s->rpid)>0) { 
		strncpy(billing_data->rpid,s->rpid,sizeof(billing_data->rpid)-1);
	}

	pthread_mutex_lock (&billing_datalock);

        billing_data->next = NULL;
		billing_data->prev = NULL;

        temp = billing_data_head;
        if(temp == NULL){
            billing_data_head = billing_data;
			billing_data_leaf = billing_data;
        }
        else{
			/* Always making new node as head of the list */
			billing_data->next = billing_data_head;
			billing_data_head->prev = billing_data;	
			billing_data_head = billing_data;
							
          }

	pthread_mutex_unlock (&billing_datalock);

 	sem_post(&billing_semid);
	return 0;
}

int is_header_exists(struct sip_msg *msg,char *header_name,int header_name_len,char *header_value,int header_value_len){
	int nret = 0;
	struct hdr_field *hf = NULL;
	/* Checking for header */
	parse_headers(msg, HDR_EOH, 0);
	memset(header_value, 0, header_value_len);
	for(hf=msg->headers; hf; hf=hf->next){
		if (hf->name.len == header_name_len && ( !strncasecmp (hf->name.s, header_name, header_name_len))){
			snprintf(header_value,header_value_len -1,"%.*s",hf->body.len,hf->body.s);
			nret = 1;
			break;
		}
	}           
	return nret;
} 

static int stir_create_billing_thread(void){

	pthread_attr_init(&stir_attr);

	pthread_attr_setdetachstate(&stir_attr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(&stir_t, &stir_attr, stir_billing_thread,NULL)){ /* creating billing  thread */
		LOG (L_ERR, "[stir_create_billing_thread] Failed to create a threadn");
		return 0;
	}
	return 1;
}

static void *stir_billing_thread(void *data)
{

    struct racc_stir_details *temp = NULL;

	LOG (L_ERR, "[stir_billing_thread] Billing thread Started  !!!!");

    do{
		sem_wait(&stir_semid);  /* waiting for semaphore */
		pthread_mutex_lock (&stir_datalock);
		temp = stir_data_leaf;
		if(temp == stir_data_head){
			stir_data_head = NULL;
			pthread_mutex_unlock (&stir_datalock);
		}else {
			pthread_mutex_unlock (&stir_datalock);
			if(temp && temp->prev ) {
				if (temp->prev != stir_data_head){
					temp->prev->next = NULL;
				}else{
					pthread_mutex_lock (&stir_datalock);
					temp->prev->next = NULL;
					pthread_mutex_unlock (&stir_datalock);
				}
				stir_data_leaf =temp->prev;
			}
		}
		if(temp){
			write_to_stir_fifo(temp);
			free(temp);
			temp = NULL;
		}
	}while(1);
	return NULL ;
}

static int insert_stir_session(struct racc_stir_details *s){

	struct racc_stir_details *temp = NULL, *stir_data = NULL;

	if(s == NULL) {
		LOG (L_ERR, "[insert_stir_session] racc_str_details is NULL !!!!\n");
		return -1;
	}

	stir_data = (struct racc_stir_details *) malloc (sizeof(struct racc_stir_details));  /* Creating memory */
	if(stir_data == NULL){
		LOG (L_ERR, "[insert_stir_session] Unable to Allocate memory !!!!\n");
		return -1;
	}

	memset(stir_data , 0 , sizeof(struct racc_stir_details));

	if(s->timestamp) {
			stir_data->timestamp = s->timestamp;
	}
	if(s->callid && strlen(s->callid)) {
			strncpy(stir_data->callid, s->callid, sizeof(stir_data->callid)-1);
	}
	if(s->req_uri && strlen(s->req_uri)) {
			strncpy(stir_data->req_uri, s->req_uri, sizeof(stir_data->req_uri)-1);
	}
	if(s->contact_ip && strlen(s->contact_ip)) {
			strncpy(stir_data->contact_ip, s->contact_ip, sizeof(stir_data->contact_ip)-1);
	}
	if(s->lcontact_ip && strlen(s->lcontact_ip)) {
			strncpy(stir_data->lcontact_ip, s->lcontact_ip, sizeof(stir_data->lcontact_ip)-1);
	}
	if(s->via_ips && strlen(s->via_ips)) {
			strncpy(stir_data->via_ips, s->via_ips, sizeof(stir_data->via_ips)-1);
	}

	pthread_mutex_lock (&stir_datalock);

	stir_data->next = NULL;
	stir_data->prev = NULL;

	temp = stir_data_head;
	if(temp == NULL){
		stir_data_head = stir_data;
		stir_data_leaf = stir_data;
	}
	else{
		/* Always making new node as head of the list */
		stir_data->next = stir_data_head;
		stir_data_head->prev = stir_data;
		stir_data_head = stir_data;
	}

	pthread_mutex_unlock (&stir_datalock);

	sem_post(&stir_semid);
	return 0;
}

static int  write_to_stir_fifo (struct racc_stir_details *stir_data){

	if(write(stir_fifo_fd, (char *)stir_data,sizeof(racc_stir))< 0)
	{
		LOG (L_ERR, "[write_to_stir_fifo]ERROR (racc)req writing to stir fifo is failed?!IN LOCAL CANCEL\n");
		return 0;
	}

	return 1;
}

