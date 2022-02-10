#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include "wsfifo.h"
#include "wsfifodb.h"
#include "dprint.h"

char *wsdb_international_prefix = "011";
int wsdb_international_prefix_len = 3;

extern MYSQL *db_live_handle;
extern MYSQL *db_wh_handle;
extern MYSQL *db_acc_active;
extern MYSQL *db_wspartners; 
extern MYSQL *db_contact_handle;

extern char who[25];
extern char omsserver[25];
extern char omsserver1[25];
extern int queryretrycount;

extern int maxnoerr;
extern int retrymaxnoerr;
extern int errbuff[254];
extern int retryerrbuff[254];
extern int keepactivecallrecord;
extern int nServiceid;
int erno;
int code;
int mysqlerrno;

extern int connection_flag;
extern int server_sockfd;
extern int _nIsRunning;
extern int  pt_timer;
extern char primary_presence_ip[128];
extern int  primary_presence_port;

/*Billing Engine Changes..Tulsi*/
extern float PulseRate;
/*International and internal billing flags --kkmurthy 2010-10-01*/
extern int intllookupflag;
extern int internalbilling;
extern int singlecdrtable;  //Added for VF changes

/*fetching missedcall sleep time from configuration file */
extern int missedcall_sleep; 
extern int missedcall_querytime;
extern char user_agent[256];
extern int ws_ctccall_flag;   // Added for 3pcc calls
extern int nDataCenterID;
extern int fwd_delay_time;

extern int Reporting_flag;
extern void *ACMQ;

extern char servertz[32];
extern char cTemplate_Table_For_LCR[32];
extern char cPrefix_IntPstTable[32];
extern int LcrCallType;
extern int nRoutingPlanEXT_CallType;
extern int nXferCallType;
extern int nCancelAnotherCallType;
extern int nEnableSleepPatch;

/* Usage Billing changes */
extern int Commonseat;
extern int Virtual_Seat;
extern int Business_Voice_Basic;

extern int nSpamRejectCode;
extern int nCallerIdBlockRejectCode;
extern char cEndPonitCdrTemplate[64];
extern char cEndPonitCdrPrefix[64];

static int parse_ws_fifo_uri_buffer(char * buffer, int nLength, struct ws_uri * wsuri) 
{
	char *ptr = NULL;
	if(!wsuri){
		LOG(L_ERR, "[wsfifo::parse_ws_fifo_uri_buffer]is wsuri NULL?\n");
		return -1;
	}
	
	memset(wsuri, 0, sizeof(struct ws_uri));
	strncpy(wsuri->parsed, buffer,sizeof(wsuri->parsed)-1);
	wsuri->type = wsuri->parsed;
   	wsuri->command = strchr(wsuri->type, WS_SEPERATOR);
   	if ( !wsuri->command ) {
   	/*Parse failed*/
	LOG(L_ERR, "[wsfifo::parse_ws_fifo_uri_buffer]parse_ws_fifo_uri_buffer][wsacc_call.uri]parsing is failed*-*-*%s*-*-*\n",wsuri->parsed);
       	wsuri->type = NULL;
       	return -1;
   	}
   	*wsuri->command = '\0'; /* Till here type*/
   	wsuri->command++;

   	wsuri->group = strchr(wsuri->command,WS_SEPERATOR);
   	if(!wsuri->group){
    	return -1;
    }
    *wsuri->group = '\0'; /* Till here group*/
    wsuri->group++;

    wsuri->context = strchr(wsuri->group, WS_SEPERATOR);
    if ( !wsuri->context ) {
       	return -1;
    }
    *wsuri->context = '\0'; /* Till here context*/
    wsuri->context++;

	wsuri->token = strrchr(wsuri->context, WS_SEPERATOR);
    if ( !wsuri->token) {
       	return -1;
    }
    *wsuri->token = '\0';/* Till here token*/
    wsuri->token++;
	/*Added below for missed call changes having wspib in PSTN Inbound tokens */
	strncpy(wsuri->actual_token,wsuri->token,sizeof(wsuri->actual_token)-1);
	if(ptr = strstr(wsuri->token,"wspib")){
		ptr=ptr+5;
		if(ptr){
			wsuri->token = ptr;
		}
	}
    return 1;	
}

static enum UriType get_fifo_uri_type(char *buffer,  int length,struct ws_uri * wsuri) {
	char * types[] = {"ivr", "opr",  "cmd", "cnf", "acd", "xfer", "vml", "vmr", "rec", "ext", "dir", "vmln", "vmlo", "agt", "pst", "pkp","mwi", "did", "fax","ptt","oms","xfer","gext","inbxfer","lcr",NULL};
	int ni = 0;
	enum UriType uritype = Invalid_Uri;

	if ( parse_ws_fifo_uri_buffer (buffer,strlen(buffer), wsuri) == 1) {
		uritype = WS_URI;
		for ( ni = 0; types[ni] != NULL; ni++ ) {
			if ( !strcmp(wsuri->type, types[ni])) {
				/* Matched*/
				uritype = ni;
				break;
			}
		}

	} else {
		/* Not a valid wsuri. Great.*/
		int nJunk, nLength;
		nLength = strlen(buffer) ;
		if ( ( nJunk = atoi(buffer)) && strstr(buffer, "-") == NULL) {
			/* Some numeric. Could be direct extension or pstn number.*/
			if ( nJunk >= 10 && nJunk <= 80 && nLength == 2) {
				/* Frame a call pickup uri*/
				uritype = Pickup_Request;
				LOG( L_INFO, "[wsfifo::parse_ws_fifo_uri_buffer] Pickup_Request\n");
			} else if (buffer && !strncmp(buffer, wsdb_international_prefix, wsdb_international_prefix_len)) {
		                uritype = International_Phnumber;
		                LOG( L_INFO, "[wsfifo::get_fifo_uri_type] International_Phnumber\n");
			} else if (nLength >= 7 && nLength <= 12 ) {
				/* PSTN Numbers*/
				uritype = Phone_Number;
				LOG( L_INFO, "[RACC::get_fifo_uri_type] Phone_Number 1\n");
			} else {
				/* Invalid Number for PBX*/
				uritype = Invalid_Uri;
				LOG( L_ERR, "[wsfifo::get_fifo_uri_type] Invalid_Uri\n");
			}
		}else {
			char tmpuri[255];
			int nj = 0;
			int nInvalid = 0;
			for ( ni =0; ni < nLength; ni++) {
				if ( buffer[ni] >= '0' && buffer[ni] <= '9' ) {
					tmpuri[nj] = buffer[ni];
					nj++;
				} else if ( buffer[ni] != '-' ) {
					/* Against intuitive dialing rules. It could be regular SIP URI  like user-context*/
					nInvalid = 1;
					break;
				}
			}
			if ( nInvalid ) {
				uritype = Local_User;
			} else {
				tmpuri[nj] = 0;
				uritype = Phone_Number;
				strncpy(buffer, tmpuri,length-1);
				LOG( L_INFO, "[wsfifo::get_fifo_uri_type] Phone_Number 2\n");
			}
		}
	}
	return uritype;
}

int GetTollFreeCodes(char *phonenumber)
{
	char sqlcommand [128]="";
	char tempphone[256]="";
	char temp[8]="";
	int nRet = -1;
	char *siteidqry = "SELECT code FROM state_codes WHERE sid= 46";
	//char *siteidqry = "SELECT sc.code AS code FROM states s, state_codes sc WHERE s.sname = 'TollFree' AND s.sid = sc.sid";
	int i=0;
	int found = 0;

	MYSQL_ROW row;
	MYSQL_RES *res;
	
	if(!isdigit(phonenumber[0])){
		if (phonenumber[0] == '+') {
			strncpy(tempphone, &phonenumber[1],sizeof(tempphone));
			LOG(L_WARN, "[wsfifo::GetTollFreeCodes] E164 Format : %s\n",tempphone);
		} else {
    		LOG(L_INFO,"[wsfifo::GetTollFreeCodes] No use to check is this Toll Free or not for <%s>\r\n",phonenumber);
    		return found;
		}
	}else{
		strncpy(tempphone, phonenumber,sizeof(tempphone));
	}

	bzero (sqlcommand, sizeof (sqlcommand));
	snprintf (sqlcommand, sizeof (sqlcommand),siteidqry);
	
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::GetTollFreeCodes]Failed qry : '%s'\n",sqlcommand);
		return;
	}

	res = mysql_store_result(db_live_handle);
	if (res && mysql_num_rows(res) > 0) {
		for(i=0;i<mysql_num_rows(res);i++){
			row = mysql_fetch_row(res);
			if(row[0] != NULL){
				bzero(temp,sizeof(temp));
				snprintf(temp,sizeof(temp)-1,"1%s",row[0]);
				if((!strncmp(row[0],tempphone,strlen(row[0]))) || (!strncmp(temp,tempphone,strlen(temp))) ){
					//LOG( L_INFO, "[wsfifo::GetTollFreeCodes]Founded Toll Free Number**%s** with the NPA **%s**\r\n",tempphone,row[0]);
					found = 1;
					break;
				}
			}
		}
		mysql_free_result(res);
	}else{
		if(res){
			mysql_free_result(res);
		}
		LOG(L_ERR, "[wsfifo::GetTollFreeCodes] ERROR TOLL FREE Query **%s**\n", sqlcommand);
	}
	return found;
}

void get_sitename(char *sitename , char *grp , char *outbuffer, int outbuf_len)
{
	char buf[128] = "";
	char *replace = NULL;
	
	memset(buf , 0 , sizeof(buf));

	/* Added for fixing msicpfiforeader crash on 06032013 --vravi*/
	if(!sitename){
		LOG(L_ERR, "[wsfifo:get_sitename] There is no Sitename ..returning \n");
		return;
	}	
	/*below condition will remove any integer value in sitename if it appears at first like '2-pbxtesting' */
	if(isdigit(sitename[0])) {
		sitename = strchr(sitename , '-');
		if(sitename) {
			sitename++;
		}
	}
	
	if(sitename) {
		while(replace = strchr(sitename , '-')) {
			if(replace) {
				*replace = '.';
			}
		}
	}
	if(sitename && grp && !isdigit(grp[0]) && strncmp(grp , "transfer",8)) {
		snprintf(buf , sizeof(buf)-1 , "%s.%s" , sitename,grp);
	}
	else strncpy(buf , sitename , sizeof(buf)-1);
	
	snprintf(outbuffer , outbuf_len-1 , "%.*s" , sizeof(buf)-1 , buf);
}

void get_wsagent_id(char *inbuffer,char *outbuffer , char *ext, int nLen,int outbuf_len)
{
	int len=0;
	char buf[128]= "";
	char *ptr = NULL;
	struct ws_uri wsuri;
	struct wscallratedetails wscrd;

	if(!inbuffer){
		LOG(L_ERR,"source buffer is NULL\n");
		return;
	}

	if(parse_ws_fifo_uri_buffer(wsacc_call.to , strlen(wsacc_call.to), &wsuri) == 1) {
		if(!strncmp(wsuri.type, "cmd",3) || !strncmp(wsuri.type,"vmo",3) || !strncmp(wsuri.type,"vmd",3)) {
			strncpy(ext ,wsuri.group,nLen);
		}
		else if(strncmp(wsuri.type, "moc",3) && (!ws_ctccall_flag || (ws_ctccall_flag && strncmp(wsuri.type, "ctc",3)))) { /* Not copying wsuri.command in case of 3pcc calls*/
			strncpy(ext ,wsuri.command, nLen);
		}
		else if(ws_ctccall_flag && !strncmp(wsuri.type, "ctc",3)){ /* Copying wsuri.group in case of 3pcc calls*/
			if(isdigit(wsuri.group[0])){
				strncpy(ext ,wsuri.group, nLen);
			}
			else {
				if(!strncmp(wsacc_call.fromtag,"gcp-",4) || !strncmp(wsacc_call.fromtag,"dcp-",4)){
					strncpy(ext,&wsacc_call.DialedNumber[3],nLen);
				}
				else
					strncpy(ext ,wsuri.command, nLen);
			}
		}
	}
	/*Added below if condition for routing plan check whether PSTN USER is in 1st priority*/
	if(inbuffer && strlen(inbuffer) == 11) {
		if(strstr(wsacc_call.to , "iext-") || strstr(wsacc_call.to , "ext-")) {
			get_sitename(wsuri.context , wsuri.group,outbuffer,outbuf_len);
			return;	
		}
	}
	else if(inbuffer && !strncmp(inbuffer, "911_", 4) ) { /*Added the condition for 911 callback calls (req_uri : '911_MACID_username-pbxtesting')*/
		ptr = strrchr(inbuffer , '_');
		if(ptr) {
			ptr ++ ;
			if(ptr){
				strncpy(buf , ptr , sizeof(buf)-1);
			}
		}
	}
	else
		strncpy(buf , inbuffer , sizeof(buf)-1);
	if(strstr(inbuffer , "oms-") || strstr(inbuffer , "ext-") || strstr(inbuffer , "obxfer-") || strstr(inbuffer , "xfer-") || strstr(inbuffer , "ptt-") || strstr(inbuffer , "acd-") ||  strstr(inbuffer , "opr-") || strstr(inbuffer , "ivr-") || strstr(inbuffer,"nop-")) {

		if (parse_ws_fifo_uri_buffer(buf, strlen(buf), &wsuri) == 1) {
			memset(buf , 0 ,sizeof(buf));
			if(!strncmp(wsuri.group , "transfer" ,8) || !strncmp(wsuri.group , "0",1)) {
				if(!strncmp(wsuri.type , "xfer", 4) || !strncmp(wsuri.type , "ptt",3) || !strncmp(wsuri.type , "ext" , 3) || !strncmp(wsuri.type , "obxfer" , 6)) {
					strncpy(ext ,wsuri.command ,nLen);
					get_sitename(wsuri.context , wsuri.group,outbuffer,outbuf_len);
					return ;
				}
				if(strstr(wsacc_call.to , "pst-")) {
					if(parse_ws_fifo_uri_buffer(wsacc_call.to , strlen(wsacc_call.to), &wsuri) == 1) {
						if(strncmp(wsuri.group , "0",1)) {
							if(strlen(wsuri.group) == 10) {
								memset(buf , sizeof(buf), 0);
								snprintf(buf , sizeof(buf)-1 , "1%s" , wsuri.group);
								strncpy(ext , buf, nLen);
							}
							else  {
								if(!isdigit(wsuri.group[0])){/* Added for hash transfer to PSTN */
									snprintf(outbuffer,nLen,"%s-%s",wsuri.group,wsuri.context);
									get_agentid(outbuffer);
								}else{/*BLIND Transfer to PSTN */
									strncpy(ext , wsuri.group, nLen);
									get_sitename(wsuri.context , NULL , outbuffer,outbuf_len);
								}
							}
							return ;
						}
						else if(!isdigit(wsacc_call.from[0])){
							/* # transfer to PSTN from caller */
							if(strstr(wsacc_call.from , "iext-") || strstr(wsacc_call.from , "ext-")) {
								if(parse_ws_fifo_uri_buffer(wsacc_call.from , strlen(wsacc_call.from), &wsuri) == 1) {
									strncpy(ext , wsuri.command, nLen);
									get_sitename(wsuri.context , wsuri.group,outbuffer,outbuf_len);
									return ;
								}
							} else {
								strncpy(buf , wsacc_call.from , sizeof(buf)-1);
							}
						}
					}
				}
				else if(strstr(wsacc_call.from , "mext-") || strstr(wsacc_call.from , "gext-") || strstr(wsacc_call.from , "opr-")) {
					if(parse_ws_fifo_uri_buffer(wsacc_call.from, strlen(wsacc_call.from), &wsuri) == 1) {
						strncpy(ext , wsuri.command, nLen);
						get_sitename(wsuri.context , wsuri.group,outbuffer,outbuf_len);
						return ;
					}
				} 
				else if(isdigit(wsacc_call.from[0])) {/*Call transfer to acd group from PSTN*/
					if(parse_ws_fifo_uri_buffer(wsacc_call.from, strlen(wsacc_call.from), &wsuri) == 1) {
						if( wsuri.type && wsuri.group ) {
							strncpy(ext ,wsuri.type , nLen);
							get_sitename(wsuri.group,wsuri.context,outbuffer,outbuf_len);
							return ;
						}
					}
				}
				else strncpy(buf , wsacc_call.from , sizeof(buf)-1);
			}else if(isdigit(wsuri.group[0]) && strncmp(wsuri.group , "0" , 1)){ /*ivr + hunt group diaster to PSTN * --vravi/swaroopa */
				if(!strncmp(wsuri.type,"xfer",4) || !strncmp(wsuri.type,"acd",3) || !strncmp(wsuri.type,"opr",3) || !strncmp(wsuri.type , "ivr",3) || !strncmp(wsuri.type , "nop" ,3)) {
					/* Added for warm transfer to group did's(HUNT,ACD,IVR) --swaroopa*/
					if(strstr(wsacc_call.to,"pst-")) {
						if(parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri) == 1) {
							if(strlen(wsuri.command) ==10) {
								snprintf(ext , nLen,"1%s",wsuri.command);
							}
							else {
								strncpy(ext , wsuri.command , nLen);
							}
							get_sitename(wsuri.context , NULL , outbuffer,outbuf_len);
							return;
						}
					}
				} else {
					strncpy(ext , wsuri.group, nLen);
					get_sitename(wsuri.context , NULL , outbuffer,outbuf_len);
					return;
				}
			}
			else {
				snprintf(buf , sizeof(buf) -1 ,"%s@%s" , wsuri.group , wsuri.context);	
			}
		}
	}
	
	get_agentid(buf);
	snprintf(outbuffer , outbuf_len-1 , "%.*s" , sizeof(buf)-1 , buf);
}

int Get611CalleeInfo(struct wscallratedetails *wscrd ,char *phonenumber,int nExtension,  char * cSitename)
{
    int ret = 0 , nRet = -1, i = 0;
    char tempphone[256]="";
    char sqlcommand [2048]="";

    char *serviceidqry1 = "select p.siteid,p.extensions,p.id,p.agentid,REPLACE(sn.sitename,'.','_'),'','',ag.id from pbxusers p, siteinfo_new sn, agent ag where sn.sitename = p.sitename and ag.siteid = sn.siteid and ag.agentid = p.agentid and  p.sitename = '%s' and p.extensions = '%d' " 
        " UNION "
        " select sp.siteid,sp.extensions,'','',REPLACE(sn.sitename,'.','_'),sp.code,sp.title,'' from special sp, siteinfo_new sn  where sp.siteid = sn.siteid and  sn.sitename = '%s' and sp.extensions = %d " 
        " UNION "
        " select sp.siteid,sp.extensions,'','',REPLACE(sn.sitename,'.','_'),sp.code,sp.title,'' from special sp, siteinfo_new sn  where sp.siteid = sn.siteid and  sn.sitename = '%s' and sp.code = %d ";

    char *serviceidqry = "select p.siteid,p.extensions,p.id,p.agentid,REPLACE(sn.sitename,'.','_'),'','',ag.id  from accountphonenumbers a,siteinfo_new sn, pbxusers p, agent ag  where sn.siteid = p.siteid  and sn.siteid = a.siteid and a.siteid = p.siteid and a.extensions = p.extensions and  p.siteid = ag.siteid and ag.agentid = p.agentid and   a.phnumber = '%s'"
        " UNION " 
        "select sp.siteid,sp.extensions,'','',REPLACE(sn.sitename,'.','_'),sp.code,sp.title,'' from accountphonenumbers a,siteinfo_new sn, special sp where sn.siteid = sp.siteid  and sn.siteid = a.siteid and a.siteid = sp.siteid and a.extensions = sp.extensions and phnumber = '%s' "
        " UNION "
        "select sp.siteid,sp.extensions,'','',REPLACE(sn.sitename,'.','_'),sp.code,sp.title,'' from accountphonenumbers a,siteinfo_new sn, special sp where sn.siteid = sp.siteid  and sn.siteid = a.siteid and a.siteid = sp.siteid and a.extensions = sp.code and phnumber = '%s'  "
         " UNION "
         " select uc.siteid,uc.extension,'','',REPLACE(sn.sitename,'.','_')sitename ,'','','' from accountphonenumbers a,siteinfo_new sn, uc_apps uc where sn.siteid = uc.siteid  and sn.siteid = a.siteid and a.siteid = uc.siteid and a.extensions = uc.extension and phnumber = '%s' " ;
    
    char *serviceidqry2 = "select p.siteid,p.extensions,p.id,p.agentid,REPLACE(sn.sitename,'.','_'),'','', ag.id from accountphonenumbers a,siteinfo_new sn, pbxusers p, agent ag  where sn.siteid = p.siteid  and sn.siteid = a.siteid and a.siteid = p.siteid and a.extensions = p.extensions and  p.siteid = ag.siteid and ag.agentid = p.agentid and   a.phnumber = '1%s'"
           " UNION "
           "select sp.siteid,sp.extensions,'','',REPLACE(sn.sitename,'.','_'),sp.code,sp.title,'' from accountphonenumbers a,siteinfo_new sn, special sp where sn.siteid = sp.siteid  and sn.siteid = a.siteid and a.siteid = sp.siteid and a.extensions = sp.extensions and phnumber = '1%s' "
           " UNION "
           " select sp.siteid,sp.extensions,'','',REPLACE(sn.sitename,'.','_'),sp.code,sp.title,'' from accountphonenumbers a,siteinfo_new sn, special sp where sn.siteid = sp.siteid  and sn.siteid = a.siteid and a.siteid = sp.siteid and a.extensions = sp.code and phnumber = '1%s'  "
           " UNION "
           " select uc.siteid,uc.extension,'','',REPLACE(sn.sitename,'.','_')sitename,'','','' from accountphonenumbers a,siteinfo_new sn, uc_apps uc where sn.siteid = uc.siteid  and sn.siteid = a.siteid and a.siteid = uc.siteid and a.extensions = uc.extension and phnumber = '1%s' " ;

    MYSQL_ROW row;
    MYSQL_RES *res=NULL;
    
    memset(sqlcommand, 0, sizeof(sqlcommand));
    memset(tempphone, 0, sizeof(tempphone));
     
    if(phonenumber == NULL || wscrd == NULL){
        LOG(L_ERR,"[wsicpfifo::Get611CalleeInfo] Error phonenumber <%p> wscrd <%p>",phonenumber,wscrd);
        return ret;
    }

    if (phonenumber[0] == '+') {
        strncpy(tempphone,&phonenumber[1],sizeof(tempphone));
        LOG(L_WARN, "[Get611CalleeInfo] E164 Format : %s\n",tempphone);
    } else {
        strncpy(tempphone,phonenumber,sizeof(tempphone));
    }

    if((strlen(phonenumber) > 0) && (!isdigit(tempphone[0]))){
        LOG(L_ERR,"[wsicpfifo::Get611CalleeInfo] We can't find assignto for <%s>",phonenumber);
        return ret;
    }

    if(cSitename !=NULL && strlen(cSitename) > 0){
        SITENAME_PARSE(cSitename);
         snprintf(sqlcommand,sizeof(sqlcommand),serviceidqry1,cSitename,nExtension,cSitename,nExtension,cSitename,nExtension); 
    }else if(strlen(tempphone) == 10){
        snprintf(sqlcommand,sizeof(sqlcommand),serviceidqry2,tempphone,tempphone,tempphone,tempphone);
    } else{
        snprintf(sqlcommand,sizeof(sqlcommand),serviceidqry,tempphone,tempphone,tempphone,tempphone);
    }

    nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
    if(nRet == -1)
    {
        LOG( L_INFO, "[wsicpfifo::Get611CalleeInfo]Failed query : '%s'\n",sqlcommand);
        return;
    }

    res = mysql_store_result(db_acc_active);
    if (res && mysql_num_rows(res) > 0) {

        memset(wscrd->ownerdid,0,sizeof(wscrd->ownerdid));
        strncpy(wscrd->ownerdid,tempphone,sizeof(wscrd->ownerdid)-1);
        row = mysql_fetch_row(res);
        if(row[0] != NULL){
            wscrd->siteid = atoi(row[0]);
        }
        if(row[1] != NULL){
            wscrd->ownerext = atoi(row[1]);
        }
        if(row[2] != NULL){
            wscrd->pbxid = atoi(row[2]);
        }
        if(row[3] != NULL){
            strncpy(wscrd->agentid,row[3],sizeof(wscrd->agentid)-1);
        }

        if(row[4] != NULL){
            strncpy(wscrd->sitename,row[4],sizeof(wscrd->sitename)-1);
        }
        if(row[5] != NULL){
            wscrd->groupcode = atoi(row[5]);
        }
        if(row[6] != NULL){
            strncpy(wscrd->group_title,row[6],sizeof(wscrd->group_title)-1);
        }
        if(row[7] != NULL){
            wscrd->nagtId = atoi(row[7]);
        }
        ret = 1;
    }

    if(res){
        mysql_free_result(res);
        res = NULL;
    }
    return ret;
}

void get_grouptitle(struct wscallratedetails *wscrd,int nGrp_flag)
{
	int nRet = 0,nFlag =0,is_opr_flag=0;
	char sqlcommand[1024]= "",*ptr=NULL,buf[32]="",touri[256]="",cGcode[128]="";
	MYSQL_ROW row;
	MYSQL_RES *res;
	struct ws_uri wsuri_to;

	strncpy(touri,wsacc_call.to,sizeof(touri)-1);	
	if((parse_ws_fifo_uri_buffer(touri,strlen(touri),&wsuri_to) != 1 || ( wsuri_to.type && strlen(wsuri_to.type) && strcmp(wsuri_to.type,"acdxfer") &&  strncmp(wsuri_to.type,"acdext",6)  && !nGrp_flag))){
		LOG(L_ERR,"[get_grouptitle]call is not acd or ivr..so returning here\n");
		return;
	}

	memset(sqlcommand,0, sizeof (sqlcommand));
	if(wsuri_to.context){
		while((ptr = strchr(wsuri_to.context,'-'))){
			*ptr = '.';
		}
	}
	if(nGrp_flag == 1){
		if(wsuri_to.command){
			if(strcmp(wsuri_to.command, "01") && strcmp(wsuri_to.command, "001")){/*if not operator then calling get_cmd_plan\n*/
				if(get_cmd_plan(&wsuri_to,wscrd,&nFlag)){
					if(nFlag == 1){
						return; /*Group Details are already inserted from get_cmd_plan so return here*/
					}else if(nFlag ==2){/*destination is operator have to be call get_operator_plan*/
						is_opr_flag = 1;
					}
				}
			}
			if(!strcmp(wsuri_to.command, "01") || !strcmp(wsuri_to.command, "001") || is_opr_flag){/*destination is operator have to be call get_operator_plan*/
				if(get_operator_plan(&wsuri_to,cGcode,sizeof(cGcode))){
					if(strlen(cGcode) > 0 && !isdigit(cGcode[0])){
						return;		/*Here ivr operator as user,so need to insert Group Details,so return here*/
					}else if(strlen(cGcode) > 0 && isdigit(cGcode[0])){/*Group Details have to be inserted since ivr with another group(hunt,acd or ivr...)*/
						snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_DID , cGcode,cGcode,wsuri_to.context,wsuri_to.context,cGcode,cGcode);
					}
				}
			}
	 	}
	}else if(!strncmp(wsuri_to.type,"acdext",6) && strlen(wsacc_call.QName)>0){
		snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_TO_ACD ,wsacc_call.QName,wsuri_to.context);
	}else if(nGrp_flag==2){
		if((parse_ws_fifo_uri_buffer(wsacc_call.uri,strlen(wsacc_call.uri),&wsuri_to) == 1)){
			if(wsuri_to.context){
				while((ptr = strchr(wsuri_to.context,'-')))
					*ptr = '.';
			}
			if(wsuri_to.type && (!strncmp(wsuri_to.type,"ivr",3) ||!strncmp(wsuri_to.type,"nop",3) || !strncmp(wsuri_to.type,"vml",3))){
                snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_DID , wsuri_to.command,wsuri_to.command,wsuri_to.context,wsuri_to.context,wsuri_to.command,wsuri_to.command);
			}else if(wsuri_to.type && !strncmp(wsuri_to.type,"opr",3)){
                snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_DID , wsuri_to.group,wsuri_to.group,wsuri_to.context,wsuri_to.context,wsuri_to.group,wsuri_to.group);
			}
		}
	}else if(nGrp_flag==3){
		if(wsuri_to.type && (!strncmp(wsuri_to.type,"gext",4))){
			snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_DID , wsuri_to.command,wsuri_to.command,wsuri_to.context,wsuri_to.context,wsuri_to.command,wsuri_to.command);
		}
	}else if(nGrp_flag==4){            // Added for AA to PSTN case
		if(wsuri_to.type && (!strncmp(wsuri_to.type,"pst",3))){
			snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_DID , wsuri_to.command,wsuri_to.group,wsuri_to.context,wsuri_to.context,wsuri_to.command,wsuri_to.command);
		}
	}else{
		snprintf(sqlcommand , sizeof(sqlcommand)-1 ,QUERY_GET_GRPTITITLE_DID , wsacc_call.DialedNumber,wsacc_call.DialedNumber,wsuri_to.context,wsuri_to.context,wsacc_call.DialedNumber,wsacc_call.DialedNumber);
	}
	if(strlen(sqlcommand) <= 0){
		LOG(L_ERR,"No need to process ExecuteDbQuery ,so retrun here\n");
		return;
	}
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1){
		LOG(L_INFO,"[get_grouptitle]Failed qry : '%s'\n",sqlcommand);
		return;
	}
	
	res = mysql_store_result(db_live_handle);
	if (!res || mysql_num_rows(res) <=0) {
		if(res) {
			mysql_free_result(res);
			res=NULL;
		}
         LOG(L_ERR,"[get_grouptitle]call is not acd or ivr..so returning here-1\n");
		return;
	}

	if(row = mysql_fetch_row(res))
	{
		if(row[0]) {
			memset(wscrd->group_title,0,sizeof(wscrd->group_title)-1);
			strncpy(wscrd->group_title,row[0],sizeof(wscrd->group_title)-1);
		}
		if(row[1]){
			wscrd->groupcode=atoi(row[1]);
		}
	}

	if(res) {
		mysql_free_result(res);
		res=NULL;
	}

	return;
}

void get_wsagent_details(struct wscallratedetails *wscrd, char *agentname , char *ext,int nGrp_flag)
{
	int nRet = 0;
	char sqlcommand[2048]= "" , *ptr = NULL, cSitename[128] = "";
	char agentid[128] = "";
	int nFlag = 0,nGrp = 0;
	MYSQL_ROW row;
	MYSQL_RES *res;
       
	if(agentname == NULL) {
		LOG(L_ERR,"[wsicpfifo::get_wsagent_details]Error agentname is NULL <%p>\n",agentname);
		return;
	}

	memset(agentid , 0 , sizeof(agentid));
	strncpy(agentid , agentname , sizeof(agentid)-1);

	memset(sqlcommand,0, sizeof (sqlcommand));

	if(ext && strlen(ext) > 0) {
		if((ptr = strchr(agentname, '@')) && ++ptr) {
			strncpy(cSitename , ptr, sizeof(cSitename)-1);
			while((ptr = strchr(cSitename, '-'))) 
				*ptr = '.';
			snprintf(sqlcommand,sizeof(sqlcommand)-1, AGENTQRYTWO , cSitename, ext, cSitename, ext, ext, cSitename, ext, ext, cSitename , ext,ext,agentname);
		}
		else {
			while((ptr = strchr(agentname, '-'))) 
				*ptr = '.';
			snprintf(sqlcommand,sizeof(sqlcommand)-1, AGENTQRYTWO , agentname , ext, agentname, ext, ext, agentname, ext, ext, agentname , ext,ext,agentname);
		}
	} 
	else {
		snprintf(sqlcommand, sizeof(sqlcommand)-1, AGENTQRYONE , agentid);
	}
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::get_wsagent_details]Failed qry : '%s'\n",sqlcommand);
		return;
	}

	res = mysql_store_result(db_live_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			nFlag = atoi(row[0]);
		}

		if(row[1] != NULL){
			wscrd->pbxid = atoi(row[1]);
		}

		if(row[2] != NULL){
			wscrd->siteid = atoi(row[2]);
		}
		if(nFlag == 3 || nFlag == 4) {
			if(row[3]) {
				wscrd->groupcode = atoi(row[3]);
			}
		}
		
		if(wscrd->pbxid == 0) {
			if(row[3] != NULL)
				 wscrd->pbxid = atoi(row[3]);
		}

		if(row[4]) {
			memset(wscrd->group_title,0,sizeof(wscrd->group_title));
			strncpy(wscrd->group_title,row[4],sizeof(wscrd->group_title)-1);
		}
	}
	
	if(res){
		mysql_free_result(res);
	}
	if(!strncmp(wsacc_call.to,"pcr",3)){/* group title and group code does not have parked returned call*/
		memset(wscrd->group_title,0,sizeof(wscrd->group_title));
		wscrd->groupcode = 0;
	}
	get_grouptitle(wscrd,nGrp_flag);	
}

void GetSiteId(struct wscallratedetails *wscrd, char *sitename)
{
	char sqlcommand [128]="";
	char orgsite[128] = "";
	int i=0;
	int nRet = -1;
	MYSQL_ROW row;
	MYSQL_RES *res;
	char *siteidqry = "SELECT siteid,saasaccount,tdiff FROM siteinfo_new WHERE sitename='%s'";

	if(sitename == NULL){
		LOG(L_ERR,"[wsfifo::GetSiteId]Error sitename is NULL <%p>",sitename);
		return;
	}
	/*Get Siteid based on this sitename..*/
	strncpy(orgsite,sitename,sizeof(orgsite)-1);
	GET_ORGSITENAME;

	snprintf (sqlcommand, sizeof (sqlcommand),siteidqry,orgsite);

	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::GetSiteId]Failed qry : '%s'\n",sqlcommand);
		return;
	}

	res = mysql_store_result(db_acc_active);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0] != NULL){
			wscrd->siteid = atoi(row[0]);
		}
		if(row[1] != NULL){
			wscrd->accounttype = atoi(row[1]);
		}
		if(row[2] != NULL && strlen(row[2]) >0){
			memset(wscrd->tdiff,0,sizeof(wscrd->tdiff));
			strncpy(wscrd->tdiff,row[2],sizeof(wscrd->tdiff)-1);
		}
		mysql_free_result(res);
	}else{
		if(res){
			mysql_free_result(res);
		}
		LOG(L_ERR, "[wsfifo::GetSiteId] ERROR in SITEID Query: %s\n", sqlcommand);
	}
}

static void GetPoolDetails(struct wscallratedetails *wscrd, char *cDialedNumber,char *cFrom,int dialedno_len)
{
	int nRet= -1;
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;
	char sqlcommand [512]="",poolnumber[64]="";

	if(!wscrd || !cDialedNumber || !cFrom){
		LOG(L_ERR,"[wsicpfifo::GetPoolDetails]Error wscrd <%p> owner <%s>",wscrd,cDialedNumber);
		return;
	}

	memset(sqlcommand,0,sizeof(sqlcommand));
	memset(poolnumber,0,sizeof(poolnumber));

	strncpy(poolnumber,cDialedNumber,sizeof(poolnumber)-1);
	if(poolnumber[0] == '+'){
		strncpy(poolnumber,&poolnumber[1],sizeof(poolnumber)-1);
	}
	
	if(strlen(poolnumber) == 7){
		Get_Real_Did(poolnumber,cFrom);
		memset(cDialedNumber,0,dialedno_len);
		snprintf(cDialedNumber , dialedno_len - 1 , "%.*s" , sizeof(poolnumber)-1 , poolnumber);
	}

	snprintf (sqlcommand, sizeof (sqlcommand)-1,QUERY_GET_POOL_DETAILS,wscrd->siteid,poolnumber,poolnumber);
	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG( L_INFO, "[wsicpfifo::GetPoolDetails]Failed query : '%s'\n",sqlcommand);
		return;
	}

	res = mysql_store_result(db_acc_active);

	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0] != NULL){
			memset(wscrd->orgname,0,sizeof(wscrd->orgname));
			strncpy(wscrd->orgname,row[0],sizeof(wscrd->orgname)-1);
		}
		if(row[1] != NULL){
			memset(wscrd->poolname,0,sizeof(wscrd->poolname));
			strncpy(wscrd->poolname,row[1],sizeof(wscrd->poolname)-1);
		}
		if(row[2] != NULL){
			memset(wscrd->aliasdid,0,sizeof(wscrd->aliasdid));
			strncpy(wscrd->aliasdid,row[2],sizeof(wscrd->aliasdid)-1);
		}
	}
	if(res){
		mysql_free_result(res);
		res = NULL;
	}
}

int Get_Real_Did(char *cPoolnumber,char *cFrom){
	
	int nRet = -1;
	char sqlcommand[256]="",callerid[16]="";
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;

	if(!cPoolnumber || !cFrom){
		LOG(L_ERR,"[wsicpfifo::Get_Real_Did] Empty details,so return here \n",cPoolnumber,cFrom);
		return -1;
	}
	snprintf (sqlcommand, sizeof (sqlcommand)-1,CALLERID_QUERY,cFrom);
	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG( L_INFO, "[wsicpfifo::Get_Real_did]Failed query : '%s'\n",sqlcommand);
		return -1;
	}

	res = mysql_store_result(db_acc_active);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0] != NULL){
			strncpy(callerid,row[0],4);
			strcat(callerid,cPoolnumber);
			strncpy(cPoolnumber,callerid,sizeof(callerid)-1);
		}
	}
	if(res){
		mysql_free_result(res);
		res = NULL;
	}
	return 1;	
}

void GetServiceId(struct wscallratedetails *wscrd, char *owner)
{
	char sqlcommand [512]="";
	char tempowner[256]="";
	char *replace=NULL;
	char *ptr = NULL;
	int nRet = -1;
	MYSQL_ROW row;
	MYSQL_RES *res;

	if(wscrd == NULL || owner == NULL){
		LOG(L_ERR,"[wsfifo::GetServiceId]Error wscrd <%p> owner <%p>",wscrd,owner);
		return;
	}
	
    memset(sqlcommand, 0, sizeof(sqlcommand));
    memset(tempowner, 0, sizeof(tempowner));
	/*Tulsi: Here we can check the parse wsuri case..But no use for group cases..why because, we will get like vmo-0,vmo-x(1-9),
	 * cmd-01,cmd-x(1 to 9),acd-0,etc...If this query fails also not an issue, why because we can get these details from finding 
	 * call rate query. But there pbxid we won't get. Anyway we won't get the pbxid for the groups. 
	 * So, no issue if this query fails also for group cases...don't get confuse when checking logs..*/

	strncpy(tempowner,owner,sizeof(tempowner));
	get_agentid(tempowner);

	/*Tulsi: splitted because of 13 warnings. I discussed with laalesh, he told it is risky-30-07-09*/
	if(isdigit(tempowner[0])){
			snprintf (sqlcommand, sizeof (sqlcommand),QUERYFOREXT,wscrd->siteid,tempowner);
	}else{
			snprintf (sqlcommand, sizeof (sqlcommand),QUERYFORAGENTID,wscrd->siteid,tempowner);
	}
	
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::GetServiceId]Failed qry : '%s'\n",sqlcommand);
		return;
	}
	res = mysql_store_result(db_live_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0] != NULL){
			wscrd->serviceid = atoi(row[0]);
		}
		if(row[1] != NULL){
			wscrd->ownerext = atoi(row[1]);
		}
		if(row[2] != NULL){
			strncpy(wscrd->agentid , row[2] , sizeof(wscrd->agentid)-1);
		}
		if(row[3] != NULL){
			wscrd->pbxid = atoi(row[3]);
		}
		if(row[4]){
			memset(wscrd->ownerdid,0,sizeof(wscrd->ownerdid));
			if(!atoi(row[4])){
				snprintf(wscrd->ownerdid,sizeof(wscrd->ownerdid)-1,"%d",wscrd->ownerext);
			}else{
				strncpy(wscrd->ownerdid,row[4],sizeof(wscrd->ownerdid)-1);
			}
		}
		if(row[5]){
			wscrd->nagtId = atoi(row[5]);
		}
		mysql_free_result(res);
	}else{
		if(res){
			mysql_free_result(res);
		}
		LOG(L_INFO,"[wsfifo::GetServiceId] ERROR in SERVICEID Query :%s\r\n",sqlcommand);
	}
}

void GetCallRate(struct wscallratedetails *wscrd, char *tempagentid)
{
	char sqlcommand [1024]="";
	char agentid[256] = "";
	float callrate = 0.0;
	int nRet = -1;
	MYSQL_ROW row;
	MYSQL_RES *res;
	char *replace=NULL;
	char *ptr = NULL;

	if(wscrd == NULL || tempagentid == NULL){
		LOG(L_ERR,"[wsfifo::GetCallRate] Anyone in this NULL no use execute the below code..wscrd :<%p> tempagentid: <%p>\r\n",wscrd,tempagentid);
		return;
	}

	/*Remove here if any starts with '+'*/
	if (tempagentid[0] == '+') {
		strncpy(agentid, &tempagentid[1],sizeof(agentid));
		LOG(L_WARN, "[wsfifo::GetCallRate] E164 Format : %s\n",agentid);
	}else{
		strncpy(agentid,tempagentid,sizeof(agentid));
	}

	if((replace = strchr(agentid,'-'))){
		*replace = '@';
		if((ptr=strchr(agentid,'1')) || (ptr=strchr(agentid,'2'))){
			*ptr='\0';
			ptr++;
			strcat(agentid,ptr);
		}
		while((replace = strchr(agentid,'-'))){
			*replace = '.';
		}
	}
	
	/*Tulsi: Don't remove limit and don't change order by with cr.serviceid*/
	char * getcallrateforagentid = " "
			"SELECT cr.nonsaas_rate, cr.saas_rate, cr.pulse, cr.billtype,ag.serviceid,cr.serviceid AS sid "
			"FROM siteinfo_new sn, callrates cr, agent ag "
			"WHERE cr.calltypeid = %d "
			"AND sn.siteid = %d "
			"AND (ag.agentid= '%s') "
			"AND sn.siteid = ag.siteid "
			"AND (ag.serviceid = cr.serviceid OR cr.serviceid = 0) "
			"UNION "
			"(SELECT nonsaas_rate, saas_rate, pulse, billtype, -2,serviceid AS sid "
			"FROM callrates "
			"WHERE calltypeid = 28 " 
			"AND serviceid = -2) "
			"ORDER BY sid DESC LIMIT 1";

	/*Tulsi:Don't get confuse here, this query will give rates for normal extensions. If the group extention will give the second query*/
	char * getcallrateforext = " "
			"(SELECT cr.nonsaas_rate, cr.saas_rate, cr.pulse,cr.billtype,ag.serviceid "
			"FROM siteinfo_new sn, callrates cr, agent ag "
			"WHERE cr.calltypeid = %d "
			"AND (ag.serviceid = cr.serviceid OR cr.serviceid = 0) "
			"AND sn.siteid = %d "
			"AND ag.extensions = %s "
			"AND sn.siteid = ag.siteid) "
			"UNION "
			"(SELECT cr.nonsaas_rate, cr.saas_rate, cr.pulse, cr.billtype, -1 "
			"FROM siteinfo_new sn, callrates cr,special sp "
			"WHERE cr.calltypeid = %d "
			"AND sn.siteid = sp.siteid AND cr.serviceid = 0 "
			"AND sp.code = %s "
			"AND sn.siteid = %d) "
			"UNION "
			"(SELECT nonsaas_rate, saas_rate, pulse, billtype, -2 "
			"FROM callrates "
			"WHERE calltypeid = 28 " 
			"AND serviceid = -2) "
			"ORDER BY serviceid DESC LIMIT 1";



	/*Tulsi: I splitted because of the 208 warnings are comming after execution.I discussed with laalesh, he told dangerous-30-07-09*/
	if(isdigit(agentid[0])){
		snprintf (sqlcommand, sizeof (sqlcommand),getcallrateforext,wscrd->calltype,wscrd->siteid,agentid,wscrd->calltype,agentid,wscrd->siteid);
	}else{
		snprintf (sqlcommand, sizeof (sqlcommand),getcallrateforagentid,wscrd->calltype,wscrd->siteid,agentid);
	}
	
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::GetCallRate]Failed qry : '%s'\n",sqlcommand);
		return;
	}

	res = mysql_store_result(db_live_handle);
	if (res && mysql_num_rows(res) > 0)
	{
		row = mysql_fetch_row(res);
		if(row[0] != NULL){
			wscrd->nonsaasrate = atof(row[0]);
		}
		if(row[1] != NULL){
			wscrd->saasrate = atof(row[1]);
		}
		if(row[2] != NULL){
			wscrd->pulserate = atof(row[2]);
		}
		if(row[3] != NULL){
			wscrd->billtype = atoi(row[3]);
		}
		if(row[4] != NULL){
			wscrd->serviceid = atoi(row[4]);
		}
		mysql_free_result(res);
	}
	else{
		if(res){
			mysql_free_result(res);
		}
		/*If this log comes..our panterra loosing the bill for this call flow..so take imediate action..Tulsi:31-07-09*/
		LOG(L_INFO,"[wsfifo::GetCallRate]RATE ERROR: We need to check why query failed. It will give result for only agentid or agent extensions or group extentions.So, check at what case it failed.If this log comes..our panterra loosing the bill for this call flow..so take imediate action..Tulsi\r\n");
		LOG(L_INFO,"Failed Query:%s\r\n",sqlcommand);
	}
}

void GetFinalCallType(struct wscallratedetails *wscrd,char *from, char *to)
{
	int finalcalltype = 0;
	int tollfreeFlag = -1;

	if(from == NULL || to == NULL){
		LOG(L_ERR,"[wsfifo::GetFinalCallType] Error NULL from:<%p> to:<%p>",from,to);
		return;
	}

	if((strlen(to) > 6)){
		/*Get here is this number is toll free or not*/
		if(GetTollFreeCodes(to) == 1){
			/*This number is Toll Free Number. Change the call type if which is applicable..*/
			tollfreeFlag = 1;
		 }
	}
	switch (wscrd->calltype)
	{
		/*3*/
		case PSTN_INBOUND:
			if(tollfreeFlag == 1)
			{
				finalcalltype = TOLLFREE_INBOUND;/*9*/
			}else{
				finalcalltype =PSTN_INBOUND;/*3*/
			}
			tollfreeFlag = -1;
			break;
			/*4*/
		case PSTN_OUTBOUND:
			if(tollfreeFlag == 1)
			{
				finalcalltype = TOLLFREE_OUTBOUND;/*24*/
			}else{
				finalcalltype = PSTN_OUTBOUND;/*4*/
			}
			tollfreeFlag = -1;
			break;
			/*7*/
		case PSTN_FORWARD:
			finalcalltype = PSTN_FORWARD;/*7*/
			tollfreeFlag = -1;
			break;
			/*10*/
		case CONFERENCING:
			if(tollfreeFlag == 1)
			{
				finalcalltype = TOLLFREE_CONF;/*22*/
			}else{
				finalcalltype = CONFERENCING;/*10*/
			}
			tollfreeFlag = -1;
			break;
			/*12*/
		case INBOUNDFAX:
			if(tollfreeFlag == 1)
			{
				finalcalltype = TOLLFREE_FAX;/*23*/
			}else{
				finalcalltype = INBOUNDFAX;/*11*/
			}
			tollfreeFlag = -1;
			break;
			/*14*/
		case SMARTCASTPSIB:
			if(tollfreeFlag == 1)
			{
				finalcalltype = SMARTCASTTOLLFREE_INBOUND;/*15*/
			}else{
				finalcalltype = SMARTCASTPSIB;/*14*/
			}
			tollfreeFlag = -1;
			break;
			/*18*/
		case OBXFER:
			if(tollfreeFlag == 1)
			{
				finalcalltype = TOLLFREE_OUTBOUND;/*24*/
			}else{
				finalcalltype = OBXFER;/*18*/
			}
			tollfreeFlag = -1;
			break;
		default:
			finalcalltype = wscrd->calltype;
			break;
	};
	wscrd->calltype = finalcalltype;
}

void calculatecost(struct wscallratedetails *wscrd, int nDuration)
{
	if(wscrd == NULL){
		LOG(L_INFO,"[calculatecost] wscrd <%p>\r\n",wscrd);
		return;
	}
	LOG(L_INFO,"[wsfifo::calculatecost]nDuration :%d wscrd->pulserate:%f wscrd->callrate:%f\r\n",
					nDuration,wscrd->pulserate,wscrd->callrate);
	if(nDuration == 0) {
		wscrd->callcost = 0.0;
		wscrd->callrate = 0.0;
		return;
	}

	if((wscrd->pulserate > 0) && (nDuration > wscrd->pulserate)){
			wscrd->callcost = ceil(nDuration/wscrd->pulserate) * wscrd->callrate;
	}else{
		wscrd->callcost = wscrd->callrate;
	}
}

void GetFinalCallCost(struct wscallratedetails *wscrd,char *fromuser, char *touser)
{
	char tempfrom[256]="";
	char tempto[256]="";
	struct ws_uri wsuri;

	/*Initial Checks..If any one failed..no use doing process..so return*/
	if (fromuser== NULL || wscrd == NULL || touser == NULL){
		LOG(L_INFO,"[wsfifo::GetFinalCallCost]wscrd <%p> owner<%p> touser <%p>\r\n",wscrd,fromuser,touser);
		return;
	}
	
	strncpy(tempfrom,fromuser,sizeof(tempfrom));
	strncpy(tempto,touser,sizeof(tempto));
	//Is WS URI or NOT...
	memset(&wsuri,0x00,sizeof(wsuri));
	if(parse_ws_fifo_uri_buffer(touser,strlen(touser),&wsuri) == 1){
		LOG(L_ERR,"[wsfifo::GetCallRate] agentid:%s wsuri.command:%s wsuri.group:%s wsuri.context:%s wsuri.type:%s\r\n",
						touser,wsuri.command,wsuri.group,wsuri.context,wsuri.type);
		bzero(tempto,sizeof(tempto));
		strncpy(tempto,wsuri.command,sizeof(tempto));
	}

	GetFinalCallType(wscrd,tempfrom,tempto);

	if(wscrd->calltype == PSTN_FORWARD || wscrd->calltype == OBXFER){
		GetCallRate(wscrd,tempfrom);
	}else{
		GetCallRate(wscrd,tempto);
	}

	/*Here we need to verify how we are billing...means Call based,pulsebased or minute based..*/
	switch(wscrd->billtype){
		case Pulse:
			wscrd->callrate = wscrd->accounttype?wscrd->saasrate:wscrd->nonsaasrate;
			calculatecost(wscrd, (wscrd->callduration));
			break;
		case Call:
			wscrd->callcost = wscrd->accounttype?wscrd->saasrate:wscrd->nonsaasrate;
			break;
		default:
			LOG(L_INFO,"[wsfifo::GetFinalCallCost]Default Case...<callrateType : %d>\r\n",wscrd->billtype);
			break;
	};
}


float GetInternationalCallCost(char *time, char *t_filed,int intl_npa)
{
	float intcost=0.00;
	char sqlcommand [1024]="";
	int nRet = -1;
	char *ptr_to = NULL;
	MYSQL_ROW row;
	MYSQL_RES *res;

	if(time == NULL || t_filed == NULL){
		LOG(L_INFO,"[wsfifo::GetInternationalCallCost] ERROR:time <%p> t_filed <%p>\r\n",time,t_filed);
		return intcost;
	}

	char *serviceidqry = ""
			"SELECT CASE WHEN weekday('%s') BETWEEN 1 AND 5 THEN "
			"(CASE WHEN hour('%s') BETWEEN 8 AND 18  THEN b.bdayprice ELSE "
			"b.beveningprice END) ELSE b.bweekendprice END AS bdayprice, b.zonecode "
			"FROM international_tariff b "
			"WHERE "
			"(SUBSTRING('%s', 1, 8) = b.zonecode "
			"OR SUBSTRING('%s', 1, 7) = b.zonecode "
			"OR SUBSTRING('%s', 1, 6) = b.zonecode "
			"OR SUBSTRING('%s', 1, 5) = b.zonecode "
			"OR SUBSTRING('%s', 1, 4) = b.zonecode "
			"OR SUBSTRING('%s', 1, 3) = b.zonecode "
			"OR SUBSTRING('%s', 1, 2) = b.zonecode) "
			"ORDER BY  b.zonecode DESC LIMIT 1";
	char *serviceidqry1 = ""
			"SELECT CASE WHEN weekday('%s') BETWEEN 1 AND 5 THEN "
			"(CASE WHEN hour('%s') BETWEEN 8 AND 18  THEN b.bdayprice ELSE "
			"b.beveningprice END) ELSE b.bweekendprice END AS bdayprice, b.zonecode "
			"FROM nanpa_tariff b "
			"WHERE "
			"(SUBSTRING('%s', 1, 8) = b.zonecode "
			"OR SUBSTRING('%s', 1, 7) = b.zonecode "
			"OR SUBSTRING('%s', 1, 6) = b.zonecode "
			"OR SUBSTRING('%s', 1, 5) = b.zonecode "
			"OR SUBSTRING('%s', 1, 4) = b.zonecode "
			"OR SUBSTRING('%s', 1, 3) = b.zonecode "
			"OR SUBSTRING('%s', 1, 2) = b.zonecode) "
			"ORDER BY  b.zonecode DESC LIMIT 1";

	bzero (sqlcommand, sizeof (sqlcommand));

	if(strlen(t_filed) > 0) {
		ptr_to = t_filed;
	}

	if(!isdigit(t_filed[0])){
		if((ptr_to = strchr(t_filed,'*')) && strlen(ptr_to)>3){
			ptr_to = ptr_to + 3;
		}else{
			LOG(L_INFO,"[wsfifo::GetInternationalCallCost] No Use to execution of this Query for <%s>\r\n",t_filed);
			return intcost;
		}
	}
	
	/* For fixing class of service issue separated nanpa_tariff from international_tariff --vravi */
	if( intl_npa == 1 ){
		snprintf (sqlcommand, sizeof (sqlcommand),serviceidqry1,time,time,ptr_to,ptr_to,ptr_to,ptr_to,ptr_to,ptr_to,ptr_to);
	}else{
		snprintf (sqlcommand, sizeof (sqlcommand),serviceidqry,time,time,ptr_to,ptr_to,ptr_to,ptr_to,ptr_to,ptr_to,ptr_to);
	}
	nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 0);
	if(nRet == -1)
	{
		LOG(L_WARN , "[wsfifo::GetInternationalCallCost]Failed qry : '%s'\n" , sqlcommand);
		return;
	}	

	res = mysql_store_result(db_wh_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0] != NULL){
			intcost = atof(row[0]);
		}
		mysql_free_result(res);
	}else{
		if(res){
			mysql_free_result(res);
		}
	}
	return intcost;
}

/*---Added to convert the sitename into lowercase if any of the characters is in uppercase --09/12/2010--Priyam--*/
void ws_set_to_lowercase(char *site)
{
	char temp[128] = "",*ptr = NULL, ch = ' ';
    if(!site){
        LOG( L_ERR, "[wsfifo::check_sitename] Sitename Not Found !!!!\n");
        return ;
    }
    ptr = site;
    while(*ptr != '\0'){
      if(isupper(*ptr)){
	      ch = tolower(*ptr);
          *ptr = ch;
      }
      ptr++;
   }
}

int check_international_npa (const char *number,int *dis_siteid)
{
	char sqlcommand [1024]="",dstphone[32]="",ph_num[32] = "";
	int db_val = 0;
	int nRet = -1;
	MYSQL_ROW row;
        MYSQL_RES *res;
//	char *format_string = "SELECT 1 FROM accountphonenumbers where phnumber='%s' UNION SELECT 2 FROM international_tariff WHERE intl_npa = 1 and zonecode = '011%s'";
	char *format_string = "SELECT 1,siteid FROM accountphonenumbers where phnumber='%s' UNION SELECT 2,0 FROM nanpa_tariff WHERE zonecode = '011%s'";/*getting siteid to know across account or within account*/

	if(number){
		if(strlen(number) > 10){
			strncpy(dstphone,number,sizeof(dstphone)-1);
		}else {
			snprintf(dstphone,sizeof(dstphone)-1,"1%s",number);
		}
		strncpy(ph_num,dstphone,sizeof(ph_num)-1);
	}
	if(dstphone[0] && strlen(dstphone) > 10){
		dstphone[4] = '\0';
	}
	memset(sqlcommand , 0x00 , sizeof(sqlcommand));
	snprintf(sqlcommand , sizeof(sqlcommand)-1,format_string,ph_num,dstphone+1);
	
	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand ,ACCACTIVE  , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::check_international_npa]Failed qry : '%s'\n",sqlcommand);
		return -1;
	}

	res = mysql_store_result(db_acc_active);
	if(res != NULL && (mysql_num_rows(res) > 0)){
		row = mysql_fetch_row(res);
		if(row[0]){
			db_val = atoi(row[0]);
		}
		if(row[1]){
			*dis_siteid = atoi(row[1]);
		}
	}
	if(res){
		mysql_free_result(res);
	}
	return db_val;
}

/* Function to get Seat Type of the caller using serviceid */
int GetWsSeatType(int nServiceid, char *cServiceType, int nLength, int *nPoolValue) {

	int nRet = 0;
	char cQuery[512] = "";

	MYSQL_ROW row;
	MYSQL_RES *res = NULL;
	
	memset(cQuery, 0, sizeof(cQuery)); 
	snprintf(cQuery, sizeof(cQuery)-1, SELECT_SERVICETYPE_QUERY, nServiceid);

	nRet = ExecuteDbQuery (&db_wh_handle , cQuery , WARE , 0);
	if(nRet == -1) {
		LOG(L_ERR , "[wsfifo::GetWsSeatType]Failed query : '%s'\n" , cQuery);
		return -1;
	}

	res = mysql_store_result(db_wh_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			memset(cServiceType, 0, sizeof(cServiceType));
			strncpy(cServiceType, row[0], nLength-1);
		}
		if(row[1]) {
			*nPoolValue = atoi(row[1]);
		}

	}

	if(res){
		mysql_free_result(res);
		res = NULL;
	}
	return 0;
}

void CallculateCallCost(struct wscallratedetails  *wscrd, int nLocalCallType, int nDuration) {
	
	int nRet = 0, nUsageRateId = 0;
	float fPulseRate = 0;
	char cQuery[512] = "";

	MYSQL_ROW row;
	MYSQL_RES *res=NULL;


	if(!wscrd){
		LOG(L_ERR,"[wsicpfifo::CallculateCallCost] wscrd is NULL",wscrd);
		return;
	}

	memset(cQuery, 0, sizeof(cQuery)); 
    snprintf(cQuery, sizeof(cQuery)-1, SELECT_USAGE_RATE_SHEET_ID_QUERY, wscrd->siteid);
    nRet = ExecuteDbQuery (&db_wspartners , cQuery , WSPARTNERS , 0);
    if(nRet == -1) {
        LOG(L_WARN , "[wsicpfifo::CallculateCallCost] Failed query : '%s'\n", cQuery);
        return;
    }
    res = mysql_store_result(db_wspartners);
    if (res && mysql_num_rows(res) > 0) {
        row = mysql_fetch_row(res);
        if(row[0]) {
            nUsageRateId = atoi(row[0]);
        }
    }
    if(res){
        mysql_free_result(res);
        res = NULL;
    }

    memset(cQuery, 0, sizeof(cQuery)); 
    snprintf(cQuery, sizeof(cQuery)-1, SELECT_CALLRATE_PULSERATE_QUERY, nLocalCallType,nUsageRateId);

	nRet = ExecuteDbQuery (&db_wh_handle , cQuery , WARE , 0);
	if(nRet == -1) {
		LOG(L_WARN , "[wsfifo::CallculateCallCost] Failed query : '%s'\n", cQuery);
		return;
	}

	res = mysql_store_result(db_wh_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			wscrd->callrate = atof(row[0]);
		}

		if(row[1]) {
			fPulseRate = atof(row[1]);
			if(fPulseRate != 0) {
				wscrd->callcost = ceil((nDuration/fPulseRate)) * (wscrd->callrate);
			}
		}
	}
	
	if(res){
		mysql_free_result(res);
		res = NULL;
	}
	return;
}

/* Getting and Updating callcost and callrate for selected calltypes and seats */
void GetCallCostAndCallRate(struct wscallratedetails  *wscrd, int nDuration, int nCallType) {
	
	char cServiceType[128]	=	"";
	int nRes = 0, nLocalCallType = 0, nFlag = 0, nPoolValue = -1;
	
	if(!wscrd){
		LOG(L_ERR,"[wsicpfifo::GetCallCostAndCallRate] wscrd is NULL",wscrd);
		return;
	}
	/*Previously we are billing for all accepted calls even if duration=0. But, Billing engine is not billing for duration=0. So we added this condition*/
	if(nDuration == 0) {
		wscrd->callcost = 0;
		wscrd->callrate = 0;
		return;
	}

	memset(cServiceType,0,sizeof(cServiceType));

	switch(nCallType){
		
		case PSTN_INBOUND: {
			nRes = GetWsSeatType(wscrd->serviceid, cServiceType, sizeof(cServiceType), &nPoolValue);
			//if nPoolValue = 2 then excluding changes for BVB seat type
			if((nRes == 0) && !strcmp(cServiceType, BVB) && nPoolValue!= 2 ) {
				nLocalCallType = Business_Voice_Basic; /*assigning calltype as 30, in order to get callrate, pulserate as per the calltype 30 */		
			}else if((nRes == 0) && !strcmp(cServiceType, BV) && nPoolValue!= 2 ) {
				nFlag = 1; 	/*Added to getting  bill only for 'BUSINESSVOICE' seat types in case of across account PSIB leg --Prashant 05-10-2019 */
			}else{
					/*	Added to fix below CEG issue  --Prashant 05-10-2019
					 * "Getting bill for PSIB call legs by dialling to other account VE (virtual extension) user DID" */
				nFlag = 1;
				wscrd->callcost = 0;
				wscrd->callrate = 0;
			}
			break;
		}

		case PSTN_OUTBOUND:
		case PSTN_FORWARD:
		case OBXFER:
 		case TOLLFREE_OUTBOUND: {
			nRes = GetWsSeatType(wscrd->serviceid, cServiceType, sizeof(cServiceType), &nPoolValue);
			if(nRes == 0) {
				 //if nPoolValue = 2 then excluding changes for BVB seat type
				if(!strcmp(cServiceType, BVB) && nPoolValue!= 2 ){
					nLocalCallType = Business_Voice_Basic;
				}else if(!strcmp(cServiceType, COMMONSEAT)){
					nLocalCallType = Commonseat;
				}else if(nCallType == PSTN_FORWARD && !strcmp(cServiceType, VIRTUALEXTENSION)){
					nLocalCallType = Virtual_Seat;
				}else{ 
					nFlag = 1;
				}
			}else{
				nFlag = 1;
			}
			break;
		}

		case TOLLFREE_INBOUND:
		case TOLLFREE_CONF: {
			nLocalCallType = nCallType;
			/* For these calltypes, noneed to check for SeatType. So, we are calculation cost directly below */
			break;
		}

		default: {
			nFlag = 1;
			break;
		}
	}

	if(nFlag == 0) {
		CallculateCallCost(wscrd, nLocalCallType, nDuration);
	}
	return;
}

void GetUserCallerId(char * cUserName, char * cCallerId, int nLength, int nSiteId){
	
	int nRet = 0;
	char cQuery[512] = "";

	MYSQL_ROW row;
	MYSQL_RES *res=NULL;
	
	if(cUserName == NULL) {
		LOG(L_ERR , "[wsfifo::GetUserCallerid] cUserName: <%p> \n",cUserName);
		return;
	}
	
	memset(cQuery, 0, sizeof(cQuery)); 
	snprintf(cQuery, sizeof(cQuery)-1, SELECT_CALLERID_QUERY, cUserName, nSiteId);

	nRet = ExecuteDbQuery (&db_acc_active , cQuery , ACCACTIVE , 0);
	if(nRet == -1) {
		LOG(L_ERR , "[wsfifo::GetUserCallerid]Failed query : '%s'\n" , cQuery);
		return;
	}

	res = mysql_store_result(db_acc_active);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			memset(cCallerId, 0, nLength);
			strncpy(cCallerId, row[0], nLength-1);
		}
	}

	if(res){
		mysql_free_result(res);
		res = NULL;
	}
	return;
}

/* Fun will accepts username/dialednumber. If it is username,it will get callerid. It will get npa for callerid/dialednumber, and will returns StateId(sid) depending on the NPA */
int GetUserSid(char *cUser, int nSiteId) {
	
	char cBuf[128] = "", cNPA[4] = "", cQuery[512] = "", cAgentId[256] = "";
	int nSid = 0, nRet = 0;

	MYSQL_ROW row;
	MYSQL_RES *res=NULL;

	if(cUser == NULL) {
		LOG(L_ERR , "[wsfifo::GetUserSid] cUser: <%p> \n",cUser);
		return -1;
	}

	memset(cAgentId,0,sizeof(cAgentId));
	memset(cBuf,0,sizeof(cBuf));

	if(isalpha(cUser[0])) {
		strncpy(cAgentId,cUser,sizeof(cAgentId)-1);
		get_agentid(cAgentId);
		GetUserCallerId(cAgentId,cBuf,sizeof(cBuf),nSiteId);
	}else {
		if( !strncmp(cUser,"*67",3) || !strncmp(cUser,"*82",3) ) {  /* Avoiding first 3 digits, if to_uri is starting with *67, *82 */
			strncpy(cBuf,cUser+3, sizeof(cBuf)-1);
		}else{
			strncpy(cBuf,cUser, sizeof(cBuf)-1);
		}
	}

	memset(cNPA,0,sizeof(cNPA));
	if(cBuf[0] == '+' && cBuf[1] == '1') {
		strncpy(cNPA, cBuf+2, 3);
	}else if(cBuf[0] == '+' || cBuf[0] == '1') {
		strncpy(cNPA, cBuf+1, 3);
	}else {
		strncpy(cNPA, cBuf, 3);
	}

	if(strlen(cNPA) <= 0 ) {
		return -1;
	}

	memset(cQuery, 0, sizeof(cQuery)); 
	snprintf(cQuery, sizeof(cQuery)-1, SELECT_SID_QUERY, cNPA);

	nRet = ExecuteDbQuery (&db_wh_handle , cQuery , WARE , 0);
	if(nRet == -1) {
		LOG(L_ERR , "[wsfifo::GetUserSid]Failed query : '%s'\n" , cQuery);
		return -1;
	}

	res = mysql_store_result(db_wh_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			nSid = atoi(row[0]);	
		}
	}

	if(res){
		mysql_free_result(res);
		res = NULL;
	}

	return nSid;
}

/* Fun will check whether caller and callee are from same state or not. If same return 1 else retun 0 */
int IsIntraCall(char *cToUser, char *cFromUser, int nSiteId) {

	int nSid1 = 0, nSid2 = 0;

	if(cToUser == NULL || cFromUser == NULL || nSiteId == 0) {
		LOG(L_ERR , "[wsfifo::IsIntraCall] cToUser: <%p> cFromUser: <%p> nSiteId = %d \n",cToUser, cFromUser, nSiteId);
		return 1;
	}
	
	nSid1 = GetUserSid(cToUser, nSiteId);
	if(nSid1 == -1 || nSid1 == 0) {
		return 1;
	}

	nSid2 = GetUserSid(cFromUser, nSiteId);
	if(nSid2 == -1 || nSid2 == 0) {
		return 1;
	}

	if(nSid1 != nSid2 ) {
		return 0;       /* shows inter state call */
	}else{
		return 1;	/*shows intra state call */
	}
}

void GetZoneCode(char *cZoneCode, char *cTo, int nLength) {

	int nRet = 0;
	char cQuery[2048] = "";

	MYSQL_ROW row;
	MYSQL_RES *res=NULL;

	if(cTo == NULL) {
		LOG(L_ERR , "[wsfifo::GetZoneCode] cTo: <%p>\n", cTo);
		return;
	}

	memset(cQuery, 0, sizeof(cQuery));
	snprintf(cQuery, sizeof(cQuery)-1, SELECT_ZONECODE_QUERY, cTo, cTo, cTo, cTo, cTo, cTo, cTo);

	nRet = ExecuteDbQuery (&db_wh_handle , cQuery , WARE , 0);
	if(nRet == -1) {
		LOG(L_ERR , "[wsfifo::GetZoneCode]Failed query : '%s'\n" , cQuery);
		return;
	}

	res = mysql_store_result(db_wh_handle);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			memset(cZoneCode,0,nLength);
			strncpy(cZoneCode,row[0],nLength-1);
		}
	}

	if(res){
		mysql_free_result(res);
		res = NULL;
	}
	return;
}

/* This Fun will update zonecode & callflag_tax in cdr(accounting tables) */ 
void SetCallTaxValuesForCallTypes(int nCalltype, char *cTo, char *cFrom, int nSiteId,  char *cZoneCode, int nLen, int *callflag_tax) {

	int is_intra_call = 1;
	
	if(cTo == NULL || cFrom == NULL) {
		LOG(L_ERR , "[wsicpfifo::SetCallTaxValuesForCallTypes] cTo: <%p>, cFrom: <%p> \n",cTo,cFrom);
		return;
	}
		
	/* Setting to default */
	memset(cZoneCode, 0, nLen);
	*callflag_tax = 1;

	if(nCalltype == INT_OUTBOUND || nCalltype == INXFER)
	{
		*callflag_tax = 3;
		GetZoneCode(cZoneCode, cTo, nLen);
	}
	else if(nCalltype == PSTN_OUTBOUND || nCalltype == PSTN_INBOUND || nCalltype == PSTN_FORWARD)
	{
		is_intra_call = IsIntraCall(cTo, cFrom, nSiteId);
		if(is_intra_call == 0) {
			*callflag_tax = 2;
		}
	}

	return;
}

/* Fun to get monthwise tabel name for inserting INTE/PSTN call records from all accounts -- 08-08-2018 */
void get_table_name_for_lcr(char *cLcrMonthWiseTableName, int nLen){

	char cBuffer[64]="";	
	time_t     present_time;
	struct tm *cLocalTime;

	present_time = time(NULL);
	cLocalTime = localtime(&present_time);
	strftime(cBuffer,sizeof(cBuffer)-1,"%m_%Y",cLocalTime);

	memset(cLcrMonthWiseTableName,0,nLen);
	snprintf(cLcrMonthWiseTableName,nLen-1,"%s%s",cPrefix_IntPstTable,cBuffer);

	return;
}

/* Fun to create monthwise new tabel for inserting INTE/PSTN call records from all accounts -- 08-08-2018 */
int creat_table(char *tblname){
	
	int nRet=1;
	char cQuery [128] = "";

	if(tblname == NULL) {
		LOG(L_WARN, "[wsfifo::creat_table] tblname : <%p>\n",tblname);
		return -1;
	}

	memset(cQuery,0,sizeof(cQuery));
	snprintf(cQuery,sizeof(cQuery)-1,CREATE_TABLE_QRY,tblname,cTemplate_Table_For_LCR);
	LOG(L_WARN, "[wsfifo::creat_table] CREATE_TABLE_QRY : %s\n",cQuery);
	nRet = ExecuteDbQuery(&db_wh_handle , cQuery , WARE , 1);
	if(nRet == -1) {
		LOG(L_WARN, "[wsfifo::creat_table] Failed qry : %s \n",cQuery);
		return nRet;
	}
	return nRet;
}

/* Fun to remove struck entries from acc_active in error response cases --08-08-2018 */
void  CheckAndDeleteLcrResponses(char *cCallid, int nResType) {

	int nRet = 0;
	char sqlcommand[256]="";

	if(!cCallid){
		LOG(L_ERR,"[wsfifo::CheckAndDeleteLcrResponses] Empty details,so return here \n",cCallid);
		return ;
	}

	if(nResType == 486) {
		snprintf (sqlcommand, sizeof (sqlcommand)-1,DELETE_LCR_ERROR_RESPONSES_486_QUERY,cCallid,LcrCallType,nXferCallType);
	} else {
		snprintf (sqlcommand, sizeof (sqlcommand)-1,DELETE_LCR_ERROR_RESPONSES_QUERY,cCallid,LcrCallType);
	}

	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG( L_INFO, "[wsfifo::CheckAndDeleteLcrResponses]Failed query : '%s'\n",sqlcommand);
		return ;
	}
	LOG( L_INFO, "[wsfifo::CheckAndDeleteLcrResponses] Delete query : '%s'\n",sqlcommand);
	return ;	
}

void GetAgentName(char *cExt, int nSiteId, int nLength){

	int nRet = -1;
	char sqlcommand[1024]="", cBuffer[64] = "";
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;

	if(cExt == NULL || nSiteId == 0){
		LOG(L_ERR,"[wsicpfifo::GetAgentName] Empty details,so return here \n");
		return ;
	}
	snprintf (sqlcommand, sizeof (sqlcommand)-1,AGENT_NAME_QUERY,nSiteId,cExt,nSiteId,cExt);
	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG( L_INFO, "[wsicpfifo::GetAgentName]Failed query : '%s'\n",sqlcommand);
		return ;
	}

	res = mysql_store_result(db_acc_active);
	if (res && mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[4] != NULL){
			memset(cBuffer,0,sizeof(cBuffer));
			strncpy(cBuffer,row[4],sizeof(cBuffer)-1); /* copying DID to Buffer. If User/Group doesn't have any DID,then row[4] is "1" */
		}
		if(row[2] != NULL && strlen(cBuffer) == 1 && !strncmp(cBuffer,"0",1) ) {   
			memset(cBuffer,0,sizeof(cBuffer));
			strncpy(cBuffer,row[2],sizeof(cBuffer)-1); /* copying agentid/group name to Buffer */
		}
	}

	if(strlen(cBuffer)>1) {
		memset(cExt,0,nLength);
		strncpy(cExt,cBuffer,nLength-1);
	}

	if(res){
		mysql_free_result(res);
		res = NULL;
	}
	return ;	
}

/*
 * Here we wrote 2 fun.s get_importedContact_details and get_AgentId_parsingDID to get the imported contact details using from:Num and TO:username and sitename and siteid, majorly focus on PSTN Inbound or Accross account calls as interal caller contact details avail. 
 * */

int get_AgentId_parsingDID(char * cToUsr, int nSiteid, char *cBuff, int nSize) 
{
	int nRet = -1 ;
	char sqlcommand[1024] = "" ;

	MYSQL_ROW row;
	MYSQL_RES *res;

	memset(sqlcommand, 0x00, sizeof (sqlcommand));
	snprintf (sqlcommand, sizeof (sqlcommand)-1, GET_AGENTID_QUERY, nSiteid, cToUsr);

	if(!db_live_handle) {
		LOG(L_WARN , "[wsfifo :: get_AgentId_parsingDID] DB handler failed here  \n");
		return -1;

	}

	nRet = ExecuteDbQuery(&db_live_handle, sqlcommand, LIVE , 0);
	if(nRet == -1){
		LOG(L_INFO,"[get_AgentId_parsingDID] Throwing error i.e failed Query: '%s'\n",sqlcommand);
		return -1;
	}

	res = mysql_store_result(db_live_handle);
	if (res &&  mysql_num_rows(res) > 0) {
		row = mysql_fetch_row(res);
		if(row[0]) {
			nRet = 0 ;
			strncpy(cBuff, row[0], nSize-1); 
		}
	}

	if(res) {
		mysql_free_result(res);
		res = NULL;
	}
	return nRet ;
}

int get_importedContact_details(char * sitename, int nSiteid, char * cContactNum, char *cToUsr, int * nImpCnctID)
{
	int nRet = -1;
	char sqlcommand[1024] = "", cBuff[64] = "",cSitename[64] = "",  *cPtr  = NULL;	

	MYSQL_ROW row;
	MYSQL_RES *res;

	*nImpCnctID = 0;

	if(sitename == NULL ||  cContactNum == NULL || cToUsr == NULL) {
		LOG(L_WARN , "[wsfifo :: get_importedContact_details] Invalid input details here i.e sitename:%p contactNum:%p ToUser:%p \n",sitename, cContactNum, cToUsr);
		return -1;
	}

	if(!db_contact_handle) {
		LOG(L_WARN , "[wsfifo :: get_importedContact_details] DB handler failed here  \n");
		return -1;

	}

	if(isdigit(cToUsr[0])) { //Get agentID based on TO DID
		memset(cBuff, 0x00, sizeof(cBuff)) ;
		get_AgentId_parsingDID(cToUsr, nSiteid, cBuff, sizeof(cBuff)) ;		
		cToUsr = cBuff ;
	}

	cPtr = strchr(cContactNum, '+');
	if(cPtr) { 
		cContactNum++; 
	}

	memset(cBuff, 0x00, sizeof(cBuff)) ;
	strncpy(cBuff, cContactNum++, sizeof(cBuff)-1) ;

	memset(cSitename, 0x00, sizeof(cSitename)) ;
        strncpy(cSitename, sitename, sizeof(cSitename)-1) ;
        /*Sitename required conversion*/
        while((cPtr = strchr(cSitename, '_'))){
                *cPtr = '.';
        }

	if(isdigit(cContactNum[0])) { // cContactNum must be a DID (digits)
		memset(sqlcommand, 0x00, sizeof(sqlcommand)) ;
		snprintf (sqlcommand, sizeof (sqlcommand), QUERY_IMPORTCONTACT_DETAILS , nSiteid, cToUsr, cSitename, cContactNum, cBuff,  cContactNum, cBuff, cContactNum, cBuff, cContactNum, cBuff);

		nRet = ExecuteDbQuery(&db_contact_handle, sqlcommand, CONTACTSDB, 5);
		if(nRet == -1) {
			LOG(L_WARN , "[wsfifo :: get_importedContact_details] Throwing error i.e failed Query:'%s'\n", sqlcommand);
			return -1;
		}

		res = mysql_store_result(db_contact_handle);
		if(res &&  mysql_num_rows(res) > 0 ){
			row = mysql_fetch_row(res);
			if(row[0]){
				*nImpCnctID = atoi(row[0]);
			}
		}

		if(res) {
			mysql_free_result(res);
			res = NULL;
		}
		return  0 ;
	} 
	return  -1;
}

/* This is a very lengthy routine, which performs all the decision making on calltype,
** properly parsing the fields, forms table names.. hell lot of stuff. Its ugly, and
** gives lots of candidates for crashing the server. Be optimistic...
**/
void racc_fifo_write_final_data (char *callid, char *fromtag, char *totag, char *branch ,unsigned nCalltoken, int nIsFwdCall)
{
	enum UriType utType,mutType;
	struct ws_uri wsuri,wsuri_mruri;
	int i,k=0, uid=0, exten,groupcode = 0,ano_calltype = 0, missedcall_cause = 0 ,need_extraleg = 0, is_pstoutbound=-1, ctcOmsFlag = 0, cFwipflag = 0,nMissedcall_cause = 0;
	char sqlcommand [4096]="", site[64]="", old_site[64]="", token [16]="",macid[64]="",group_title[64]="",callername[64]="";
	char start_time [32]="", end_time [32]="", ivr_time[32]="", t_diff [32]="",cStime[32]="";	
	char r_uri [128]="", f_field[128]="",old_uri[64]="", r_from [128]="", r_to [128]="", org_to [128]="",callerid[256]="",hold_stats[4096]="";
	char t_field[128]="",u_field[128]="",cMuri[128]="", cTemp[256] = "", cTempOne[256] = "";
	char r_contact[128],str[10]="";
	double diff=0.0;
	time_t s_time, e_time, i_time;
	struct tm s_tm, e_tm, i_tm;
	int retval = 0,intl_npa = 0,db_val = 0;
	char * pTemp = NULL,*name = NULL,ring_time[128] = "",hst[128] = "",mruri[256]="";
	char * scsiteid,intlnpa_field[32]="",is_user_call=0;
	int hold_count = 0,hold_time = 0,ring_duration = 0,hm_duration = 0;
	int nRet = -1,callrecord=0,route_type = 0,call_type = 0,Phone_Type=0,is_site_found=0,isacdcall=0,dis_siteid=0,PhoneType=0, nCalltypeflag=0;
	char  * scsql =  " SELECT REPLACE(REPLACE(sitename,'.','_'),'-','_')  FROM siteinfo_new s WHERE s.siteid = %s";
	int max_hold_time = 0, nSpam = 0, nVerified = 0, nImportedContactsID  = 0, nIscalleriddisplay = 0;
			
	char * format_string = NULL;
	MYSQL_ROW row;
	MYSQL_RES *res;
		
	/*Tulsi:Billing Engine Changes-23-07-09*/
	struct wscallratedetails wscrd;
	struct ws_uri wsuri1,wsuri2;
	char *owner = NULL;
	char *acc_tblname = NULL;   //Added for VF changes
	char *cPtr = NULL, *cPtrOne = NULL, *ptr=NULL, *cPtr1=NULL, *cPtr2=NULL, cAcc_Sitename[256]="";
	/*adding for CONTACT IP*/
	char contact_ip[64]="",DialedNumber[128]="",cToken[16]="",cFromTag[128]="",transfer_token[16]="",cAcrossUser[256]="",cTransferno[32]="";
	unsigned int CallToken=0;
	char cZoneCode[24] = "",originalcallerid[128]="",originalcnam[128]="" , cSitename[64] = "";
	int callflag_tax = 1,ispickupcall=0;
	float fCallCost_prev = 0, fCallRate_prev = 0;
	/* INTE/PSTN routing and billing changes */
	int is_direct_call = 0, is_transfer = 0, offnet_call = 0, nMoveToCdr = 1,nCallType=0;
	char cLcrTableName[32] = "", transferred_time[32] = "", tf_duration[32] = "", transfer_number[128] = "",cNewBuf[256]="", cGrpCode[16] = "",cGroupTitle[128] ="";
	char cIvr_Destination[32] = "", cIvr_Time[128] = "", cModifiedCallerId[256] = "";
   	int	nAAduration = 0;
	int rec_format = 0;

	memset(&wsuri1, 0x00,sizeof(wsuri1));
	memset(&wsuri2, 0x00,sizeof(wsuri2));
	memset(&wscrd, 0x00,sizeof(wscrd));
	memset(cSitename, 0x00, sizeof(cSitename));

	/* Basically, the following 2 queries do the same thing, but the from, to tags may 
	 * differ based on from which wsacc_call.phone the BYE was sent. (caller or callee)
	 */
	bzero (sqlcommand, sizeof (sqlcommand));
	if(nCalltoken == 0) {
		if(ws_ctccall_flag)
			snprintf(sqlcommand,sizeof(sqlcommand)-1,QUERY_ACCACTIVE_CTC_SELECT_FINAL_DATA,wsacc_call.callid,wsacc_call.fromtag,wsacc_call.totag,wsacc_call.totag,wsacc_call.fromtag,who);
		else
			snprintf(sqlcommand,sizeof(sqlcommand)-1,QUERY_ACCACTIVE_SELECT_FINAL_DATA,wsacc_call.callid,wsacc_call.fromtag,wsacc_call.totag,wsacc_call.totag,wsacc_call.fromtag,who);
	}
	else {	
		if(ws_ctccall_flag)
			snprintf(sqlcommand,sizeof(sqlcommand)-1,QUERY_ACCACTIVE_CTC_SELECT_FINAL_DATA_TOKEN,nCalltoken,nServiceid,wsacc_call.callid);
		else
			snprintf(sqlcommand,sizeof(sqlcommand)-1,QUERY_ACCACTIVE_SELECT_FINAL_DATA_TOKEN,nCalltoken,nServiceid,wsacc_call.callid);
	}

	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::racc_fifo_write_final_data]Failed qry : '%s'\n",sqlcommand);
		return;
	}

	res = mysql_store_result(db_acc_active);
	if( res == NULL || mysql_num_rows(res) <= 0 ){
			LOG(L_ERR,"[wsfifo::racc_fifo_write_final_data]no records were find '%s' '%s' '%s' '%d' Query : < %s >\n",wsacc_call.callid ,wsacc_call.fromtag, wsacc_call.totag,nServiceid , sqlcommand);
			if(res)
				mysql_free_result (res);
			racc_fifo_delete_records (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag);
			return;
	}
	row = mysql_fetch_row(res);
	if(row[0]){
		strncpy(r_uri, row[0],sizeof(r_uri));
	}
	if(row[1]){
		strncpy(r_from, row[1],sizeof(r_from));
	}
	if(row[2]){
		strncpy(r_to, row[2],sizeof(r_to));
	}
	if(row[3]){
		strncpy(start_time,row[3],sizeof(start_time));
	}
	if(row[4]){
		strncpy(end_time,row[4],sizeof(end_time));
	}
	if(row[5]){
		strncpy(token, row[5],sizeof(token));
	}
	if(row[7]){
		strncpy(old_uri,row[7],sizeof(old_uri));
	}
	if(row[8]){
    	strncpy(ivr_time,row[8],sizeof(ivr_time));
	}
	if(row[9]){
		strncpy(r_contact,row[9],sizeof(r_contact));
	}
	if( row[10]){
		strncpy(t_diff,row[10],sizeof(t_diff));	
		/* Updating min duration as '1' for all answered calls. to fix LTD-reports issue --OCT-24-2019 */
		if(nIsFwdCall == 0 && atoi(t_diff) == 0) {
			memset(t_diff, 0, sizeof(t_diff));
			strncpy(t_diff, "1", 1);
		}	
	}
	if(row[11]){
		hold_count = atoi(row[11]);
	}
	if(row[12]){
		hold_time = atoi(row[12]);
	}
	if( row[13]){
	    strncpy(ring_time,row[13],sizeof(ring_time)-1);
	}
	if(row[14]){
		ring_duration = atoi(row[14]);
	}
	if(row[15]){
		strncpy(hst,row[15],sizeof(hst)-1);
	}
	if(row[16]){
		hm_duration = atoi(row[16]);
		if((hold_count > 0) && strcmp(hst,"0000-00-00 00:00:00")){
			if(!(hm_duration)){
				hold_time += 1;
			}else{
				hold_time += hm_duration;
			}
		}
	}
	if(row[17]) {
		callrecord = atoi(row[17]);
	}
	/*CONTACT IP*/
	if(row[18]){
		memset(contact_ip,0x00,sizeof(contact_ip));
		strncpy(contact_ip,row[18],sizeof(contact_ip)-1);
	}
	if(row[19]){
        memset(macid,0x00,sizeof(macid));
        strncpy(macid,row[19],sizeof(macid)-1);
    }
	if(row[20]){
		memset(group_title,0,sizeof(group_title));
		strncpy(group_title,row[20],sizeof(group_title)-1);
	}
	if(row[21]){
		memset(callername,0,sizeof(callername));
		strncpy(callername,row[21],sizeof(callername)-1);
		if(strstr(callername, "SPAM")) {
			nSpam = 1;
		}
		if(strstr(callername, "[V]")) {
			nVerified = 1;
		}
	}
	if(row[22]){
		groupcode = atoi(row[22]);
	}
	if(row[23]){
		memset(DialedNumber,0,sizeof(DialedNumber));
		strncpy(DialedNumber,row[23],sizeof(DialedNumber)-1);
	}
	if(row[24]){
		memset(cToken,0,sizeof(cToken));
		strncpy(cToken,row[24],sizeof(cToken)-1);
		if(strstr(cToken,"wspib")){
			call_type = PSINB;
		}else{
			call_type = IPINB;
		}
	}
	if(row[25]){
		wscrd.siteid = atoi(row[25]);
	}
	if(row[26]){
		wscrd.pbxid = atoi(row[26]);
	}
	if(row[27]){
		wscrd.ownerext = atoi(row[27]);
	}
	if(row[28]){
		memset(wscrd.ownerdid,0,sizeof(wscrd.ownerdid));
		strncpy(wscrd.ownerdid,row[28],sizeof(wscrd.ownerdid)-1);
	}
	if(row[29]){
		wscrd.nagtId = atoi(row[29]);
	}
	if(row[30]){
		wscrd.serviceid = atoi(row[30]);
	}
	if(row[31]){
		memset(wscrd.agentid,0,sizeof(wscrd.agentid));
		strncpy(wscrd.agentid,row[31],sizeof(wscrd.agentid)-1);
	}
	if(ws_ctccall_flag){
		if(row[32]){       /* Added to fix voicemail issues in billing for 3pcc calls */
			memset(cMuri,0,sizeof(cMuri));
			strncpy(cMuri,row[32],sizeof(cMuri)-1);
		}
		if(row[33]){     /* Added to get missed call cause in billing for 3pcc calls */
			missedcall_cause = atoi(row[33]);
		}
	}
	if(row[32]){
		memset(transfer_token,0,sizeof(transfer_token));
		strncpy(transfer_token,row[32],sizeof(transfer_token)-1);
	}
	if(row[33]){
		memset(cAcrossUser,0,sizeof(cAcrossUser));
		strncpy(cAcrossUser,row[33],sizeof(cAcrossUser)-1);
		memset(cTransferno,0,sizeof(cTransferno));
		if((ptr = strchr(cAcrossUser,'#')) && ++ptr && strlen(ptr)>0){
			strncpy(cTransferno,ptr,sizeof(cTransferno)-1);
		}
	}
	if(row[34]){
		memset(mruri,0,sizeof(mruri));
		strncpy(mruri,row[34],sizeof(mruri)-1);
	}
	/*These changes for add hold_duartion when hold at callee end hangup at caller witout resuming at caller---sharan/04052018*/
	if(row[35] && strlen(row[35]) > 0){

		memset(hold_stats,0,sizeof(hold_stats));
		if((hold_count > 0) && (strcmp(hst,"0000-00-00 00:00:00"))){
			
			if(hm_duration == 0){
				snprintf(str,sizeof(str),":\"01\"}]");
			}else if(hm_duration <= 9){
				snprintf(str,sizeof(str),":\"0%d\"}]",hm_duration);
			}else{
				snprintf(str,sizeof(str),":\"%d\"}]",hm_duration);
			}
			snprintf(hold_stats,sizeof(hold_stats)-1,"%s%s",row[35],str);
		}else {
			snprintf(hold_stats,sizeof(hold_stats)-1,"%s",row[35]);
		}
     }	
	if(row[36]) {
		is_direct_call = atoi(row[36]);
	}
	if(row[37]){
		is_transfer = atoi(row[37]);
	}
	if(row[38]){
		strncpy(transferred_time,row[38],sizeof(transferred_time)-1);
	}
	if(row[39]) {
		strncpy(tf_duration,row[39],sizeof(tf_duration)-1);
		/* Updating min duration as '1' for all answered calls, to fix LTD-reports issue --OCT-24-2019 */
		if(nIsFwdCall == 0 && atoi(tf_duration) == 0) {
			memset(tf_duration, 0, sizeof(tf_duration));
			strncpy(tf_duration,"1",1);
		}
	}
	if(row[40]) {
		strncpy(transfer_number,row[40],sizeof(transfer_number));
	}
	if(row[41]) {
		nCallType=atoi(row[41]);
 	}
	if(row[42] && strlen(row[42]) > 0)
	{
		memset(originalcallerid,0,sizeof(originalcallerid));
		strncpy(originalcallerid,row[42],sizeof(originalcallerid)-1);
	}
	if(row[43] && strlen(row[43]) > 0)
	{
		memset(originalcnam,0,sizeof(originalcnam));
		strncpy(originalcnam,row[43],sizeof(originalcnam)-1);
	}
	if(row[44]) {
		ispickupcall = atoi(row[44]);
	}
	if(row[45]) {
             max_hold_time=atoi(row[45]);
             if((hm_duration > 0) && (max_hold_time < hm_duration)) {
                 max_hold_time = hm_duration;
             } else if((hold_count > 0) && (max_hold_time <= 0)) {
                 max_hold_time = 1;
             }
         }
    if(row[46])
    {
       nMissedcall_cause = atoi(row[46]);
    }

	memset(cIvr_Destination, 0, sizeof(cIvr_Destination));
	if(row[47] && nCallType == nRoutingPlanEXT_CallType) {
		strncpy(cIvr_Destination, row[47], sizeof(cIvr_Destination)-1);
	}
	memset(cIvr_Time, 0, sizeof(cIvr_Time));
	if(row[48] && nCallType == nRoutingPlanEXT_CallType) { 
		strncpy(cIvr_Time, row[48], sizeof(cIvr_Time)-1);	
	}
	if(row[49] && nCallType == nRoutingPlanEXT_CallType) {
		nAAduration = atoi(row[49]);
	}
    	if(row[50]) {
		nIscalleriddisplay = atoi(row[50]);
      	}

	memset(cModifiedCallerId, 0, sizeof(cModifiedCallerId));
	if(row[51]) {
		if((ptr = strstr(row[51],"sip:")) && (ptr = ptr+4)) {
			strncpy(cModifiedCallerId, ptr, sizeof(cModifiedCallerId)-1);
			if((ptr = strchr(cModifiedCallerId,'@'))) {
				*ptr = '\0';
			}
		}
	}
	if(row[52]) {
		rec_format = atoi(row[52]);
	}

	if(res)
	{
		mysql_free_result(res);
	}

	if(strlen(mruri) > 0){
		memset(&wsuri_mruri,0x00,sizeof(wsuri_mruri));
		mutType = get_fifo_uri_type(mruri,sizeof(mruri), &wsuri_mruri);
	}

	/*Added by Tulsi for billing engine -24-07-09*/
	owner = r_uri;
	// keep the to field in the org_to
	strncpy(org_to,r_to,sizeof(org_to));	
	/* Check the old_uri if it has value otherthan 0 copy that to r_to */
	if(strlen(old_uri)>1){
		strncpy(r_to,old_uri,sizeof(r_to));
	}

	strncpy(u_field,r_uri,sizeof(u_field)-1);
	strncpy(f_field,r_from,sizeof(f_field)-1);
	strncpy(t_field,r_to,sizeof(t_field)-1);
	/*strcpy(t_field,r_uri);*/
	CALLTYPE=PSTN_OUTBOUND;

	if(!strncmp(org_to,"lcr-",4)){ /* In lcr case we are using 'To' filed for getting uri type..because we are getting DID in r_uri --08-08-2018 */ 
		utType = get_fifo_uri_type(org_to,sizeof(org_to), &wsuri);
	}else{
		utType = get_fifo_uri_type(r_uri,sizeof(r_uri), &wsuri);
	}
	
	route_type = utType;
#if 0
	if(nCallType == nRoutingPlanEXT_CallType) {
		pTemp=strstr(r_from,"-");
		if (pTemp!=NULL) {
			pTemp++;
			bzero(old_site,sizeof(old_site));
			strncpy(old_site,pTemp,sizeof(old_site)-1);
			GET_OLD_SITENAME;
			nCalltypeflag=1;
		}
	}
#endif

	LOG( L_INFO, "[wsfifo::racc_fifo_write_final_data]URI type:%d sitename:%s and r_uri:%s r_from:%s r_to:%s token:%s\n", 
																		utType,wsuri.context,r_uri,r_from,r_to,token);
	/* If it is a WSURI our life is very easy copy wsuri.context to site */
	if (wsuri.context!=NULL) {
        	strncpy(site, wsuri.context, sizeof(site) - 1);
           	strncpy(cSitename, wsuri.context, sizeof(cSitename)-1);
		GET_SITENAME;
	}
	switch ( utType ) 
	{
		case XFER_Request:	
			strncpy(t_field,wsuri.command,sizeof(t_field));
			CALLTYPE = IP_FORWARD;
			break;
		case OMS_Request:
			strncpy(t_field,wsuri.command,sizeof(t_field)-1);
			LOG(L_WARN,"[OMS_Request] Group: %s\n",wsuri.group);
			if(wsuri.group && (strncmp(wsuri.group,"transfer",8) && !isdigit(wsuri.group[0]))){
					memset(f_field,0,sizeof(f_field));
					int len = strlen(wsuri.group);
					if(len > 0 && len < 3){
						snprintf(f_field,sizeof(f_field)-1,"%s",wsuri.group);
						wscrd.ownerext= atoi(f_field);
					}else{
						snprintf(f_field,sizeof(f_field)-1,"%s-%s",wsuri.group,wsuri.context);
					}
					LOG(L_WARN,"[OMS_Request] New From Uri: %s\n",f_field);
					is_user_call=1;/*Added flag to insert record for user routing plan scenarios */
			}
			CALLTYPE=PSTN_FORWARD;
			if (wsuri.command && !strncmp(wsuri.command, wsdb_international_prefix, wsdb_international_prefix_len)) {
				strncpy(intlnpa_field,wsuri.command,sizeof(intlnpa_field)-1);
				CALLTYPE=INT_OUTBOUND;
			} else if(wsuri.command){
			/* Added for the looping issue of the WorldSmart Numbers -- LOOP -- DINESH -- 13-05-2010 */
			/* Here we need to check the number for WorldSmart DID. If it is our number change the calltype to IP_FORWARD */
				if(internalbilling){		
					LOG(L_INFO , "[wsmswitch] 1 num_pointer '%s' \n" , wsuri.command);
					if(*wsuri.command == '+'){
						wsuri.command++;
						LOG(L_ERR , "[wsmswitch] 2 num_pointer '%s' \n" , wsuri.command);
					}
			
					LOG(L_INFO , "[wsmswitch] 3 number '%s' \n" , wsuri.command);
					db_val = check_international_npa(wsuri.command,&dis_siteid);
					if(db_val == 2){
						intl_npa = 1;
						CALLTYPE = INT_OUTBOUND;
						if(strlen(wsuri.command) > 10){
							snprintf(intlnpa_field,sizeof(intlnpa_field)-1,"011%s",wsuri.command+1);
						}else {
							snprintf(intlnpa_field,sizeof(intlnpa_field)-1,"011%s",wsuri.command);
                                                }
					}else if(db_val == 1){	
						CALLTYPE = IP_FORWARD;
					}
				}
			}
			owner = f_field;
			/*Setting flag to insert record for pstn user to AA group + dtmf as hunt group having disaster with another hunt group*/
			if(!strncmp(r_to,"gext-",5)){
				owner = 0;
				is_user_call=1;
			}
			break;
		case FAX_URI:
			CALLTYPE=INBOUNDFAX;
			strncpy(t_field,"Inbound FAX",sizeof(t_field)-1);
			break;
		case IVR_Number:
			CALLTYPE=PSTN_OUTBOUND;
            if(nCallType == nRoutingPlanEXT_CallType && !strncmp(r_uri, "ivr-", 4)) {
                pTemp = strstr(r_uri, "-");
                if((pTemp != NULL) && (++pTemp)) {
                    cPtr1 = strstr(pTemp, "-");
                    if(cPtr1 != NULL) {
                        memset(cGrpCode, 0x0, sizeof(cGrpCode));
                        strncpy(cGrpCode, pTemp, cPtr1 - pTemp);
                        get_group_ext(cGrpCode, site, t_field, sizeof(t_field), cGroupTitle, sizeof(cGroupTitle));
                    }
                }
            }
			break;
		case Operator_Routed:
			CALLTYPE=IP_OUTBOUND;
            if(nCallType == nRoutingPlanEXT_CallType && !strncmp(r_uri, "opr-", 4)) {
                pTemp = strstr(r_uri, "-");
                if((pTemp != NULL) && (++pTemp)) {
                    cPtr1 = strstr(pTemp, "-");
                    if((cPtr1 != NULL) && (++cPtr1)) {
                        cPtr = strstr(cPtr1, "-");
                        if(cPtr != NULL) {
                            memset(cGrpCode, 0x0, sizeof(cGrpCode));
                            strncpy(cGrpCode, cPtr1, cPtr - cPtr1);
                            get_group_ext(cGrpCode, site, t_field, sizeof(t_field),cGroupTitle, sizeof(cGroupTitle));
                        }
                    }
                }
            }
            break;
		case IVR_Command:
			CALLTYPE=IP_OUTBOUND;
			break;
		case ACD_Request:
			CALLTYPE=IP_INBOUND;
			pTemp=strstr(r_from,"-");
            if (pTemp) {
           		pTemp++;
           		strncpy(site,pTemp,sizeof(site)-1);
           		strncpy(cSitename, pTemp, sizeof(cSitename)-1);
           		GET_SITENAME;
           	}

            if(nCallType == nRoutingPlanEXT_CallType && !strncmp(r_uri,"acd",3)) {
                ptr = strchr(r_uri, '-');
                if((ptr != NULL) && (++ptr)) {
                    cPtr = strchr(ptr, '-');
                    if(cPtr != NULL) {
                        memset(&t_field, 0x0, sizeof(t_field));
                        strncpy(t_field, ptr, cPtr - ptr);
                    }
                }
            } else {
			    /*before removing this tofiled, check below condition--Tulsi-31-07-09*/
			    strncpy(t_field,"ACD Group",sizeof(t_field));
			    wscrd.assignedto = ASSGINED_TO_GROUP;
            }
			break;
		case Direct_DID:
			strncpy(t_field,r_uri,sizeof(t_field)-1);
			wscrd.assignedto = ASSIGNED_TO_EXTENSION;
			break;
		case Voice_Mail:
			CALLTYPE=VOICEMAIL_IP;
			/*comented by tulsi*/
			//strncpy(t_field,"Voice Mail",sizeof(t_field)-1);
			wscrd.assignedto = ASSIGNED_TO_EXTENSION;
			break;
		case Voice_Mail_Reading:
			//CALLTYPE=VOICEMAIL_IP;
			CALLTYPE=IP_OUTBOUND;   ///Added to change for 9999 call --  CEG bug
            if(nCallType == nRoutingPlanEXT_CallType && !strncmp(r_uri, "vmr-", 4)) {
                cPtr = strchr(r_uri, '-');
                if((cPtr != NULL) && (++cPtr)) {
                    cPtr1 = strchr(cPtr,'-');
                    if(cPtr1 != NULL) {
                        bzero(t_field, sizeof(t_field));
                        strncpy(t_field, cPtr, cPtr1-cPtr);
                    }
                }
            } else {
                strncpy(t_field,"Voice Mail Reading",sizeof(t_field)-1);
            }
			break;
		case INB_Xfer:
			CALLTYPE = IP_FORWARD;
			bzero(t_field,sizeof(t_field));
			if(wsuri.group && wsuri.context){
				snprintf(t_field , sizeof(t_field) - 1 , "%s-%s" , wsuri.group , wsuri.context); 
			}
			break;
		case Group_EXT:
		case Extension:
			CALLTYPE = IP_FORWARD;
			if(!strncmp(token,"0000000000",10)||!strncmp(token,"(null)",6)) {
				CALLTYPE = IP_OUTBOUND;
			}
            if(nCallType == nRoutingPlanEXT_CallType && !strncmp(r_uri,"gext-",5)) {
                cPtr = strchr(r_uri, '-');
                if((cPtr != NULL) && (++cPtr)) {
                    cPtr1 = strchr(cPtr, '-');
                    if(cPtr1 != NULL) {
                        bzero(t_field, sizeof(t_field));
                        strncpy(t_field, cPtr, cPtr1-cPtr);
                    }
                }
            }
   			break;
        case ACD_Agent:
        case Greeting_Manager:
        case DIR_Request:
		{
            CALLTYPE=IP_FORWARD;
            if(nCallType == nRoutingPlanEXT_CallType && (!strncmp(r_uri, "agt-", 4) || !strncmp(r_uri, "rec-", 4) || !strncmp(r_uri, "dir-", 4))) {
                cPtr = strchr(r_uri, '-');
                if((cPtr != NULL) && (++cPtr)) {
                    cPtr1 = strchr(cPtr,'-');
                    if(cPtr1 != NULL) {
                        bzero(t_field, sizeof(t_field));
                        strncpy(t_field, cPtr, cPtr1-cPtr);
                    }
                }
            }
        }
        break;
	    case International_Phnumber:
	           	CALLTYPE = INT_OUTBOUND;
			owner = r_from;
			strncpy(intlnpa_field,r_uri,sizeof(intlnpa_field)-1);
        		break;	
	    case Phone_Number:
			CALLTYPE=PSTN_FORWARD;
			db_val = check_international_npa(r_uri,&dis_siteid);
			if(db_val == 2){
				intl_npa = 1;
				CALLTYPE = INT_OUTBOUND;
				snprintf(intlnpa_field,sizeof(intlnpa_field)-1,"011%s",r_uri+1);
			}else if(db_val == 1){
				CALLTYPE = IP_FORWARD;
			}
			break;
		case WS_URI :
			CALLTYPE=IP_OUTBOUND;
			if(!strncmp(token,"0000000000",10)||!strncmp(token,"(null)",6)) {
				CALLTYPE = IP_OUTBOUND;
				if ( parse_ws_fifo_uri_buffer(r_to, strlen(r_to), &wsuri)== 1 ) {
		    		       	strncpy(token, wsuri.token,sizeof(token)-1);
				}
			}
            if(!strncmp(r_to,"obxfer",6) || strstr(r_contact,omsserver) || strstr(r_contact,omsserver1)){
				LOG(L_ERR,"[fifo_reader::WSURI] It may be warm/blind transfer for OUTBOUND / INBOUND calls. ToUri : %s \n",r_to);
                CALLTYPE = OBXFER;
                //strncpy(t_field,"Outbound Transfer",sizeof(t_field)-1);
                name=strstr(r_contact,"sip:");
                if (!name) 
					break;
                name+=4;
                pTemp=strchr(name,'@');
                if (!pTemp)
					break;
               	*pTemp='\0';
                if(name && (parse_ws_fifo_uri_buffer(name,strlen(name),&wsuri) == 1)){
					/* We are checking the NULL for the wsuri.type for the Production crash dated 05-11-09 with bug id 8609 -- SANTOSH -- 29-01-2010 */
                	if(wsuri.type && !strncmp(wsuri.type,"oms",3)){
                       	LOG(L_ERR,"1 obxfer contact:%s\n",wsuri.command);
                       	strncpy(t_field,wsuri.command,sizeof(t_field)-1);
                       	if(wsuri.command && !strncmp(wsuri.command ,wsdb_international_prefix,wsdb_international_prefix_len)){
                            CALLTYPE = INXFER;
                        }
                   	}
               	}
           	}
            if(nCallType == nRoutingPlanEXT_CallType && (!strncmp(r_uri, "nop-", 4) || !strncmp(r_uri, "wsucapp-", 8))) {
                pTemp = strstr(r_uri, "-");
                if((pTemp != NULL) && (++pTemp)) {
                    cPtr = strstr(pTemp, "-");
                    if(cPtr != NULL) {
                        if(!strncmp(r_uri, "nop-", 4)) {
                            memset(cGrpCode, 0x0, sizeof(cGrpCode));
                            strncpy(cGrpCode, pTemp, cPtr - pTemp);
                            get_group_ext(cGrpCode, site, t_field, sizeof(t_field), cGroupTitle, sizeof(cGroupTitle));
                       } else if(!strncmp(r_uri, "wsucapp-", 8)){
                           memset(t_field, 0x0, sizeof(t_field));
                           strncpy(t_field, pTemp, cPtr - pTemp);
                       }
                    }
                }
            }
			break;
		/*EXT - EXT Call Via MS --- Vijay Feb3209 */
		case Local_User:
			CALLTYPE = IP_FORWARD;
			/* Get the Sitename from user itself.  You Already have username like ramu-pandoranetworks-com*/
			if(!strncmp(token,"0000000000",10)||!strncmp(token,"(null)",6)) {
				CALLTYPE = IP_OUTBOUND;
				if ( parse_ws_fifo_uri_buffer(r_to, strlen(r_to), &wsuri)== 1 ) {
    		       	strncpy(token, wsuri.token,sizeof(token)-1);
				}
			}
			pTemp=strstr(r_uri,"-");
			if (pTemp && ++pTemp) {
				strncpy(site,pTemp,sizeof(site)-1);
           			strncpy(cSitename, pTemp, sizeof(cSitename)-1);
				GET_SITENAME;
				nCalltypeflag=0;
			}
			if (!isdigit(r_to[0])) {
				strncpy(t_field,r_uri,sizeof(t_field)-1);
			}

			exten=strlen(r_to);
			if (!strncmp (r_to, wsdb_international_prefix, wsdb_international_prefix_len)) {
				strncpy(intlnpa_field,r_to,sizeof(intlnpa_field)-1);
				CALLTYPE=INT_OUTBOUND;
				break;
			}
		    else if(exten > 9 && isdigit(r_to[0])&&strcmp(org_to,old_uri)) {
        		CALLTYPE=PSTN_FORWARD;
				break;
			}else if((exten > 4)&&(!strncmp(r_to,"iext",4) || (!strncmp(r_to,"icom",4)))){/*Ext-Ext call is marked as FWIP as it is marked as IPOB in Registrar - Vijay 23023209*/
				CALLTYPE = IP_FORWARD;
				break;
  			}else if(!strncmp(r_to, "acdxfer-", 8))
			{
				CALLTYPE=NONACD_ABONDONED_TRANSFER_OUT;
				break;	
			}		
			
			if(!strncmp(r_to,"obxfer",6) || strstr(r_contact,omsserver) || strstr(r_contact,omsserver1)){
				LOG(L_ERR,"[fifo_reader::LocalUser] It may be warm/Blind transfer for OUTBOUND / INBOUND calls. ToUri : %s \n",r_to);
				CALLTYPE = OBXFER;
				strncpy(t_field,r_uri,sizeof(t_field)-1);
				owner = f_field;
				name=strstr(r_contact,"sip:");
				if (!name) break;
				name+=4;
				pTemp=strchr(name,'@');
				if (!pTemp) break;
				*pTemp='\0';
				if(name && (parse_ws_fifo_uri_buffer(name,strlen(name),&wsuri)== 1)){
					if(wsuri.type && !strncmp(wsuri.type,"oms",3)){
						LOG(L_ERR,"1 obxfer contact:%s\n",wsuri.command);
						strncpy(t_field,wsuri.command,sizeof(t_field)-1);
						if(wsuri.command && !strncmp(wsuri.command ,wsdb_international_prefix,wsdb_international_prefix_len)){
							CALLTYPE = INXFER;
						}
					}
				}
			}
			/* Added changes for updating parking callback case billing with INTC/FWPS for International/PSTN call cases --Abhilsh/VRavi 28-11-2018 */ 
			if( !strncmp(r_to,"pcr-",4) && ((nCallType == PSTN_OUTBOUND) || (nCallType == INT_OUTBOUND))) {


				if (nCallType == PSTN_OUTBOUND){
					CALLTYPE = PSTN_FORWARD;
				} else {
					CALLTYPE = INXFER;
				}

				ptr = strchr(r_to, '-');
				if(ptr && ++ptr) {
					memset(t_field, 0, sizeof(t_field));
					strncpy(t_field, ptr, sizeof(t_field)-1);
				}

				if (!strncmp (t_field, wsdb_international_prefix, wsdb_international_prefix_len)) {
					strncpy(intlnpa_field,t_field,sizeof(intlnpa_field)-1);
				}
			}

      		break;
		case LCR:
			strncpy(t_field,wsuri.command,sizeof(t_field)-1);
			CALLTYPE = PSTN_FORWARD;
			owner = f_field;
			
			if(wsuri.command) {
				if (wsuri.command && !strncmp(wsuri.command, wsdb_international_prefix, wsdb_international_prefix_len)) {
					strncpy(intlnpa_field,wsuri.command,sizeof(intlnpa_field)-1);
					CALLTYPE = INXFER;
				}else if(internalbilling){
					LOG(L_INFO , "[wsfifo::racc_fifo_write_final_data] wsuri.command '%s' \n" , wsuri.command);		
					if(*wsuri.command == '+'){
						wsuri.command++;
					}
					
					LOG(L_INFO , "[wsfifo::racc_fifo_write_final_data] number '%s' \n" , wsuri.command);
					db_val = check_international_npa(wsuri.command,&dis_siteid);
					if(db_val == 2){
						intl_npa = 1;
						CALLTYPE = INXFER;
						if(strlen(wsuri.command) > 10){
							snprintf(intlnpa_field,sizeof(intlnpa_field)-1,"011%s",wsuri.command+1);
						}else {
							snprintf(intlnpa_field,sizeof(intlnpa_field)-1,"011%s",wsuri.command);
						}
					}
				}
			} 

		default:
			LOG( L_ERR,"[wsfifo::racc_fifo_write_final_data]ERROR, default case what is this??\n");
			break;
	};
	
	/*Removing billing for looped calls DINESH 2010-07-23*/
	if(internalbilling && (utType == OMS_Request) && (CALLTYPE == IP_FORWARD)){
		if(!is_user_call){
			LOG( L_ERR, "[wsfifo::racc_fifo_write_final_data]No billing as it is a looped call\n");
			racc_fifo_delete_records (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag);
			return;
		}
	}
	/* Make a final attempt: Not even a single record is allowed to bypass accounting */
	if (site[0] == 0) {
		char *s;
		s = strstr (r_from, "-");
		if (NULL != s) {
			s++;
			strncpy(site, s,sizeof(site)-1);
           		strncpy(cSitename, s, sizeof(cSitename)-1);
		}
		GET_SITENAME;
	}

	/*Added by Tulsi/Krishna Murthy Kandi for Billing Engine-23-07-09*/
	wscrd.calltype = CALLTYPE;
	wscrd.accounttype = 1;
	if(!wscrd.siteid){
		GetSiteId(&wscrd,site);
		is_site_found=1;
	}
	if( (strlen(DialedNumber) > 6) && (isdigit(DialedNumber[0]) || DialedNumber[0] == '+') && strncmp(DialedNumber,"011",3)){
		GetPoolDetails(&wscrd,DialedNumber,t_field,sizeof(DialedNumber));
	}

    if(nCallType == nRoutingPlanEXT_CallType) {
        pTemp = strstr(r_to, "-");
        if((pTemp != NULL) && (++pTemp)) {
            cPtr = strstr(pTemp, "-");
            if(cPtr != NULL) {
                memset(f_field, 0x0, sizeof(f_field));
                strncpy(f_field, pTemp, cPtr - pTemp);
                owner = f_field;
            }
        }
    }

	GetServiceId(&wscrd,owner);
	wscrd.callduration = atoi(t_diff);
	if(CALLTYPE == INT_OUTBOUND || CALLTYPE == INXFER){
		wscrd.callrate = GetInternationalCallCost(start_time,intlnpa_field,intl_npa);
		wscrd.pulserate = PulseRate;
		calculatecost(&wscrd,wscrd.callduration);
	}else{
        if(nCallType == nRoutingPlanEXT_CallType) {
            if(strlen(wscrd.agentid) <= 0) {
                pTemp = strstr(r_uri, "-");
                if((pTemp != NULL) && (++pTemp)) {
                    cPtr = strstr(pTemp, "-");
                    if(cPtr != NULL) {
                       memset(f_field, 0x0, sizeof(f_field));
                       strncpy(f_field, pTemp, cPtr - pTemp);
                       owner = f_field;
                    }
                }
            }
            GetServiceId(&wscrd, owner);
            memset(&f_field, 0x0, sizeof(f_field));
            strncpy(f_field, wscrd.agentid, sizeof(f_field) - 1);
            for(i=0; f_field[i]; i++) {
                if(('@' == f_field[i]) || ('.' == f_field[i])) 
                    f_field[i] = '-';
            }
           
            memset(cTemp, 0x0, sizeof(cTemp));
            memset(cTempOne, 0x0, sizeof(cTempOne));
            getProperAgent((nCallType == nRoutingPlanEXT_CallType) ? r_from : f_field, cTemp, sizeof(cTemp));
            getProperAgent(t_field, cTempOne, sizeof(cTempOne)); 
			if((strcmp(cTemp, cTempOne) == 0) || (strlen(f_field) <= 0)) {
                memset(&f_field, 0x0, sizeof(f_field));
                getProperAgent(r_from, f_field, sizeof(f_field));
			}
        }

		/*This condition for blind or warm or ivr+acd group exten cases of ACD through mediaserver*/
        if((wscrd.assignedto == ASSGINED_TO_GROUP) && !strcmp(t_field,"ACD Group")){
            char siteid[8]="";
            int groupid = 0;
            snprintf(siteid,sizeof(siteid)-1,"%d",wscrd.siteid);
            if((ptr=strstr(r_uri,siteid))){
                ptr = ptr + strlen(siteid);
                groupid = atoi(ptr);
                bzero(t_field,sizeof(t_field));
                snprintf(t_field,sizeof(t_field)-1,"%d",groupid);
                wscrd.ownerext= atoi(t_field);
                GetFinalCallCost(&wscrd,f_field,t_field);
            }
        }else{
            GetFinalCallCost(&wscrd,f_field,t_field);
        }
	}

	LOG(L_INFO,"[wsfifo::racc_fifo_write_final_data]callcost:%f pulse rate:%f",wscrd.callcost,wscrd.pulserate);
	if(wscrd.calltype == IP_FORWARD || wscrd.calltype == IP_OUTBOUND || wscrd.calltype == IP_INBOUND){
		wscrd.assignedto = ASSIGNED_TO_EXTENSION; 
	}
	if(parse_ws_fifo_uri_buffer(t_field,strlen(t_field),&wsuri1) == 1){
		LOG(L_INFO,"[wsfifo::racc_fifo_write_final_data] t_field:%s wsuri.command:%s wsuri.group:%s wsuri.context:%s wsuri.type:%s\r\n",
				t_field,wsuri1.command,wsuri1.group,wsuri1.context,wsuri1.type);
		bzero(t_field,sizeof(t_field));
		strncpy(t_field,wsuri1.command,sizeof(t_field));
	}
	memset(&wsuri1, 0x00,sizeof(wsuri1));
	if(parse_ws_fifo_uri_buffer(f_field,strlen(f_field),&wsuri1) == 1){
		LOG(L_INFO,"[wsfifo::racc_fifo_write_final_data] f_field:%s wsuri.command:%s wsuri.group:%s wsuri.context:%s wsuri.type:%s\r\n",
				f_field,wsuri1.command,wsuri1.group,wsuri1.context,wsuri1.type);
		bzero(f_field,sizeof(f_field));
		strncpy(f_field,wsuri1.command,sizeof(f_field));
	}

	ws_set_to_lowercase(site);	
	/*---- VF changes -------*/
	if(singlecdrtable == 1) {
		acc_tblname = "wscdrs";
	}else if(nCalltypeflag) {
		acc_tblname = old_site;
	}else{
		acc_tblname = site;
	}
	
	/*Added to fix the edge mark router case*/
    if( acc_tblname && strlen(acc_tblname) > 0 && (cPtr = strrchr(acc_tblname,'_')))
    {
        cPtrOne = cPtr + 1;
        while(cPtrOne && *cPtrOne)
        {
            if(isdigit(*cPtrOne))
            {
                *cPtr = '\0';
                break;
            }
            ++cPtrOne;
        }
    }
	/*------------------------*/
	/* Added changes for CTC & Acd supervisory Billing --vravi */
	if(ws_ctccall_flag){
		if(r_to && !strncmp(r_to,"wsctcb-",7) && !strncmp(r_uri,"cnf-",4)){
			if(strstr(wsacc_call.totag,CTC_TAG) || strstr(wsacc_call.fromtag,CTC_TAG)){
				memset(t_field, 0 ,sizeof(t_field));
				strncpy(t_field,DialedNumber,sizeof(t_field));
			}
		}
		if((r_to && !strncmp(r_to,CTCTYPE,4)) || (strstr(wsacc_call.totag,ACDSUP_TAG) || strstr(wsacc_call.fromtag,ACDSUP_TAG))){
			wscrd.calltype = IP_OUTBOUND;
			memset(&wsuri2, 0x00,sizeof(wsuri2));
			if(parse_ws_fifo_uri_buffer(r_to,strlen(r_to),&wsuri2) == 1){
				if(!strstr(totag,ACDSUP_TAG)){
					memset(t_field, 0 ,sizeof(t_field));
					strncpy(t_field,wsuri2.command,sizeof(t_field));
				}
				/* Added below for fixing buddy dialing to number 0 issue --srinivas*/
				if(isdigit(wsuri2.command[0]) && (atoi(wsuri2.command)==0)){
					memset(t_field, 0 ,sizeof(t_field));
					strncpy(t_field,DialedNumber,sizeof(t_field));
					if(!strncmp(t_field,"*96",3))
						wscrd.calltype = IP_FORWARD;
				}
				if((strlen(wsuri2.command)) > 5){
					if(!strncmp(wsuri2.command,"011",3)){
						memset(intlnpa_field, 0 ,sizeof(intlnpa_field));
						strncpy(intlnpa_field,wsuri2.command,sizeof(intlnpa_field)-1);
						wscrd.calltype = INT_OUTBOUND;
					}else if(GetTollFreeCodes(wsuri2.command) == 1){
						wscrd.calltype = TOLLFREE_OUTBOUND;
					}else{
						wscrd.calltype = PSTN_OUTBOUND;
					}
				}else if(!strncmp(t_field,"*67",3) || !strncmp(t_field,"*82",3)){
					wscrd.calltype = PSTN_OUTBOUND;
				}
				/*Added for 3pcc PSTN calls billing issue when EXT is in PSTN phone*/
				if(isdigit(wsuri2.group[0]) && strlen(wsuri2.group)<=5 && atoi(wsuri2.group)!=0 && (((atoi(wsuri2.command)!=0) && (strncmp(wsacc_call.fromtag,"crt-",4) || strncmp(wsacc_call.totag,"crt-",4))) || ((atoi(wsuri2.command)==0)&& (!strncmp(wsacc_call.totag,"wsctcb-",7) || !strncmp(wsacc_call.fromtag,"wsctcb-",7) || !strncmp(wsacc_call.totag,"wsrpl-",6) || !strncmp(wsacc_call.fromtag,"wsrpl-",6))) ||(!strncmp(wsacc_call.fromtag,"ptt-",4) || !strncmp(wsacc_call.totag,"ptt-",4)))){
					cFwipflag = 1;
					memset(t_field, 0 ,sizeof(t_field));
					strncpy(t_field,u_field,sizeof(t_field));
					wscrd.calltype = IP_FORWARD;
				}
			}
			if(strstr(wsacc_call.totag,ACDSUP_TAG) || strstr(wsacc_call.fromtag,ACDSUP_TAG)){
				memset(t_field, 0 ,sizeof(t_field));
				strncpy(t_field,u_field,sizeof(t_field));
			}else if((!isdigit(wsuri2.context[0])) && (!strncmp(wsacc_call.totag,"ivr-",4) || !strncmp(wsacc_call.fromtag,"ivr-",4) || !strncmp(wsacc_call.fromtag,"nop-",4) || !strncmp(wsacc_call.totag,"nop-",4) || !strncmp(wsacc_call.totag,"opr-",4) || !strncmp(wsacc_call.fromtag,"opr-",4)) && strncmp(r_uri,"cnf-",4)){
				wscrd.calltype = PSTN_OUTBOUND;
			}else if((!strncmp(wsacc_call.totag,"cnf-",4) && !isdigit(wsuri2.group[0]))|| !strncmp(r_uri,"cnf-",4) || (!strncmp(wsacc_call.fromtag,"cnf-",4) &&  !isdigit(wsuri2.group[0])) || ((!strncmp(wsacc_call.fromtag,"ptt-",4) || !strncmp(wsacc_call.totag,"ptt-",4)) && !isdigit(wsuri2.group[0]))){
				wscrd.calltype = CONFERENCING;
			}
			else if((!strncmp(wsacc_call.totag,"cnf-",4) || !strncmp(wsacc_call.fromtag,"cnf-",4)) && isdigit(wsuri2.group[0])){
				wscrd.calltype = PSTN_OUTBOUND; 
			}
		}
		if((strstr(wsacc_call.fromtag,CTC_TAG) || strstr(wsacc_call.totag,CTC_TAG)) && (strlen(cMuri)>0) && !strncmp(cMuri,"vml-",4)){ /* Added changes for fixing CTC voicemail billing issues --vravi/srinivas */
			if(!strncmp(cMuri,"vml-00",6)){// Added changes for block the Blind transfer to invalid ext billing leg
				racc_fifo_delete_records (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag);
				return;
			}
			if(wscrd.calltype != IP_FORWARD)
				ano_calltype = VOICEMAIL_IP;
		}
		/* Added to get disaster number for group calls in case of disaster is enabled for groups --Anusha*/
		if(ws_ctccall_flag && ((strstr(wsacc_call.fromtag,CTC_TAG) && !strncmp(wsacc_call.fromtag,"oms_",4) && (pTemp=strchr(wsacc_call.fromtag,'_'))) || (strstr(wsacc_call.totag,CTC_TAG) && !strncmp(wsacc_call.totag,"oms_",4) && (pTemp=strchr(wsacc_call.totag,'_'))))){
			ctcOmsFlag = 1;
			if((++pTemp) && pTemp){
				strncpy(cFromTag,pTemp,sizeof(cFromTag)-1);
				if((pTemp=strchr(cFromTag,'-'))){
					*pTemp='\0';
					if(!strncmp(r_to,CTCTYPE,4) && groupcode > 0 && !cFwipflag){
						memset(t_field, 0 ,sizeof(t_field));
						strncpy(t_field,cFromTag,sizeof(t_field)-1);
						if(strlen(t_field)>5 && !strncmp(t_field,"011",3)){
							wscrd.calltype = INT_OUTBOUND;
							memset(intlnpa_field, 0 ,sizeof(intlnpa_field));
							strncpy(intlnpa_field,t_field,sizeof(intlnpa_field)-1);
						}
						else
							wscrd.calltype = PSTN_OUTBOUND;
					}
				}
			}
		}
	}
	
	/* Added changes for getting proper service id for PSTN calls */
	if(wscrd.serviceid<=0 && strlen(wscrd.agentid)) {
		get_serviceid_pstn_calls(&wscrd);
	}

	LOG(L_ERR,"[wsfifo::racc_fifo_write_final_data] Owner Value: '%d'\n",wscrd.ownerext);
	if(utType == OMS_Request){/*Setting corresponding calltypes for routing plan cases if disaster number is within:FWIP,if across:FWPS,if international:INTC*/
		if(CALLTYPE == IP_FORWARD){/* Routingplan/user disaster to WS DID --Saidulu/Srinivas */
			Phone_Type = DISASTER_FORWARD;
		}else{ /* Routingplan/user disaster to PSTN/INTE --Saidulu/Srinivas */
			Phone_Type = PSTN_CALL;
		}
		if(wscrd.calltype == IP_FORWARD){
			if(wscrd.siteid != dis_siteid){
				wscrd.calltype = PSTN_FORWARD;
			}
		}else if(wscrd.calltype == INT_OUTBOUND){
			wscrd.calltype = INXFER;
		}
	}else if(!isdigit(t_field[0])){
		Get_PhoneType(t_field,&Phone_Type);
	}

	bzero (sqlcommand, sizeof (sqlcommand));
	if(!is_site_found){
		GetSiteId(&wscrd,acc_tblname);
	}
	if(r_to && strlen(r_to) >0 && !strncmp(r_to,"acdext-",7)){
		isacdcall=1;
	}
   
   	if(strlen(token)>0){
		CallToken = atoi(token);
	}
	
	GetCallerid(f_field,callername,callerid,256);/*Added change to Append callername and fromuri*/
	   
	/* Added Contact ip insertion field in the query */
	if(ws_ctccall_flag && (strstr(wsacc_call.fromtag,CTC_TAG) || strstr(wsacc_call.totag,CTC_TAG))){
		/*Added to get International call charge for 3pcc calls --Anusha*/
		if(wscrd.calltype == INT_OUTBOUND && utType != OMS_Request){
			wscrd.callrate = GetInternationalCallCost(start_time,intlnpa_field,intl_npa);
			wscrd.pulserate = PulseRate;
			calculatecost(&wscrd,wscrd.callduration);
		}

		if(nIscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
		    nImportedContactsID = 0;
		} else {
		    get_importedContact_details(acc_tblname, wscrd.siteid, f_field, wscrd.agentid, &nImportedContactsID);
		}
		snprintf (sqlcommand, sizeof (sqlcommand),QUERY_CTCACCOUNTNAME,acc_tblname, uid, f_field, t_field, t_diff, start_time, end_time,wscrd.calltype, CallToken, u_field,wscrd.assignedto,wscrd.ownerext,wscrd.siteid,wscrd.pbxid,wscrd.serviceid,wscrd.callcost,wscrd.callrate,hold_count,hold_time,ring_time,ring_duration,wscrd.agentid,intl_npa,callrecord,contact_ip,macid,group_title,callername,groupcode,wscrd.ownerdid,wscrd.nagtId,callid,DialedNumber,route_type,call_type,ano_calltype,missedcall_cause,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,hold_stats,max_hold_time,nImportedContactsID,rec_format);
	}else{	
			memset(cAcc_Sitename,0,sizeof(cAcc_Sitename));
			strncpy(cAcc_Sitename,acc_tblname,sizeof(cAcc_Sitename)-1);
			while((ptr = strchr(cAcc_Sitename,'_')))
				*ptr = '-';

			if(utType != LCR && strlen(cAcrossUser) >0 && !strstr(cAcrossUser,cAcc_Sitename) && !strstr(r_from,cAcc_Sitename)){
			memset(cStime,0,sizeof(cStime));
			strncpy(cStime,start_time,sizeof(cStime)-1);
			if((ptr = strchr(cStime,' '))){
				*ptr = '\0';
				if(strlen(cStime)>0){
					snprintf(sqlcommand, sizeof (sqlcommand)-1,SELECT_INTERACCOUNT_DETAIL,acc_tblname,cStime,CallToken,PSTN_INBOUND);		
					nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
					if(nRet == -1)
					{
						LOG(L_WARN , "[wsfifo::racc_fifo_write_final_data]Failed qry : '%s'\n" , sqlcommand);
						return;
					}
					res = mysql_store_result(db_wh_handle);
					if(!res || (mysql_num_rows(res) <= 0)){

						if(Phone_Type == DISASTER_FORWARD){
							PhoneType = INBOUND;
						}else{
							PhoneType = Phone_Type;
						}
						memset(sqlcommand,0,sizeof(sqlcommand));
						
						SetCallTaxValuesForCallTypes(PSTN_INBOUND, strlen(cTransferno)>0 ? cTransferno : DialedNumber, r_from, wscrd.siteid, cZoneCode, sizeof(cZoneCode), &callflag_tax);
						/*Storing collcost and callrate for further use */ 
						fCallCost_prev = wscrd.callcost;
						fCallRate_prev = wscrd.callrate;
		
						if(nIscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
					            nImportedContactsID = 0;
						} else {
						    get_importedContact_details(acc_tblname, wscrd.siteid, r_from, wscrd.agentid, &nImportedContactsID);
						}

						GetCallCostAndCallRate(&wscrd, atoi(t_diff), PSTN_INBOUND);
						snprintf (sqlcommand, sizeof (sqlcommand)-1,QUERY_ACCOUNTNAME,acc_tblname, uid, r_from, strlen(cTransferno)>0 ? cTransferno : DialedNumber, t_diff, start_time, end_time,PSTN_INBOUND, CallToken, u_field,wscrd.assignedto,wscrd.ownerext,wscrd.siteid,wscrd.pbxid,wscrd.serviceid,wscrd.callcost,wscrd.callrate,0,0,ring_time,ring_duration,wscrd.agentid,intl_npa,callrecord,contact_ip,macid,group_title,callername,groupcode,wscrd.ownerdid,wscrd.nagtId,callid,DialedNumber,route_type,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,end_time,servertz,wscrd.tdiff,PhoneType,strlen(transfer_token) >0 ? atoi(transfer_token) : 0,isacdcall,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,"",cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall,max_hold_time,(nMissedcall_cause == 15) || (nMissedcall_cause == 14) ?nMissedcall_cause:0,cIvr_Destination,cIvr_Time,nAAduration, nSpam, nVerified, nImportedContactsID, rec_format);
						LOG(L_ERR,"[wsfifo::racc_fifo_write_final_data]It's an Across account call,so we inserting one more leg into CDR '%s'\n",sqlcommand);
						/*Getting previous callcost,callrate.since we are using same wscrd struct for callcost,callrate updation into CDR*/
						wscrd.callcost = fCallCost_prev;
						wscrd.callrate = fCallRate_prev;
						
						nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
						if(nRet == -1)
						{
							LOG(L_WARN , "[wsfifo::racc_fifo_write_final_data]Failed qry : '%s'\n" , sqlcommand);
							return;
							}	
					}
					if(res)
					{
						mysql_free_result(res);
					}
				}
			}
		}

		SetCallTaxValuesForCallTypes(wscrd.calltype, t_field, f_field, wscrd.siteid, cZoneCode, sizeof(cZoneCode), &callflag_tax);
		GetCallCostAndCallRate(&wscrd, atoi(t_diff), wscrd.calltype);

		/* Inserting "lcr-"(INTE/PSTN) legs from all accounts into monthwise cdrtable.Used for monitering purpose but not for billing.This new table will contains entire PSTN/INTE calls duration  08-08-2018*/
		if((utType == LCR)) {
			memset(cLcrTableName,0,sizeof(cLcrTableName));
			get_table_name_for_lcr(cLcrTableName, sizeof(cLcrTableName));
			bzero (sqlcommand, sizeof (sqlcommand));

			snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ACCOUNTNAME,cLcrTableName, uid, (nCallType == nRoutingPlanEXT_CallType)? r_from : f_field, t_field, t_diff, start_time, end_time,wscrd.calltype, CallToken, u_field,wscrd.assignedto,wscrd.ownerext,wscrd.siteid,wscrd.pbxid,wscrd.serviceid,wscrd.callcost,wscrd.callrate,hold_count,hold_time,ring_time,ring_duration,wscrd.agentid,intl_npa,callrecord,contact_ip,macid,group_title,callername,groupcode,wscrd.ownerdid,wscrd.nagtId,callid,DialedNumber,route_type,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,end_time,servertz,wscrd.tdiff,Phone_Type,strlen(transfer_token) >0 ? atoi(transfer_token) : 0,isacdcall,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,hold_stats,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall,max_hold_time,(nMissedcall_cause == 15) || (nMissedcall_cause == 14) ?nMissedcall_cause:0,cIvr_Destination,cIvr_Time,nAAduration, nSpam, nVerified, nImportedContactsID, rec_format);
			if(strlen(sqlcommand) != 0) {
				LOG(L_INFO, "[wsfifo::racc_fifo_write_final_data] INSERT qry : '%s'\n" , sqlcommand);
				if ( strlen(site)>1&&(!isdigit(site[0])) ){
					nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
					code = mysql_errno(db_wh_handle);
					if(code == MYSQL_TABLE_NOT_FOUND_ERRCODE) {/*creating monthwise billing table for PSTN/INTE calls, if table did not exist*/
						nRet = creat_table(cLcrTableName);
						if(nRet != -1 ){
							nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
						}
					}
					if(nRet == -1){
						LOG(L_WARN , "[wsfifo::racc_fifo_write_final_data]Failed qry : '%s'\n" , sqlcommand);
					}
				}
			}
		}

		/* Inserting into accounting table */
		LOG(L_INFO , "is_direct_call = %d, is_transfer = %d \n",is_direct_call, is_transfer);
		if(is_direct_call ==  1) {
			(is_transfer == 1) ? (offnet_call = 1) : (nMoveToCdr = 0);
		}
		/* offnet_call is 1 only for 2nd leg in blindtransfers & 2,4th legs in warmtransfers. For this cases we are considering duration from transferred time to call end time  --08-08-2018*/
		bzero (sqlcommand, sizeof (sqlcommand));
		if(nMoveToCdr == 1) {
			if(offnet_call == 1) {
				calculatecost(&wscrd,atoi(tf_duration));
				if(isalpha(transfer_number[0]) && (parse_ws_fifo_uri_buffer(transfer_number,strlen(transfer_number),&wsuri1) == 1)) {
					strncpy(transfer_number,wsuri1.command,sizeof(transfer_number)-1);
				}
				if(strlen(transfer_number) > 2 &&  strlen(transfer_number) < 6) {
					GetAgentName(transfer_number,wscrd.siteid, sizeof(transfer_number));
				}
			}

			/* Added changes to fix IVR LTD missing issue --aug-1-2020*/
			if(nCallType == nRoutingPlanEXT_CallType && (strncmp(u_field,"nop-",4) == 0 || (strncmp(u_field,"ivr-",4)==0) || (strncmp(u_field,"opr-",4)==0))) {
				if(strncmp(u_field,"opr-",4) == 0) {
					memset(cIvr_Destination, 0, sizeof(cIvr_Destination));
					strncpy(cIvr_Destination,"01",sizeof(cIvr_Destination)-1);
					memset(cIvr_Time,0,sizeof(cIvr_Time));
					strncpy(cIvr_Time, start_time, sizeof(cIvr_Time)-1);
					nAAduration = atoi((offnet_call==1)?tf_duration:t_diff);
				}
				bzero (sqlcommand, sizeof (sqlcommand));
				if(nIscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
				    nImportedContactsID = 0;
				} else {
				    get_importedContact_details(acc_tblname, wscrd.siteid, (offnet_call==1)?transfer_number:r_from, wscrd.agentid, &nImportedContactsID);
				}
				snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ACCOUNTNAME,acc_tblname, uid, (offnet_call==1)?transfer_number:r_from, t_field, (offnet_call==1)?tf_duration:t_diff, (offnet_call==1)?transferred_time:start_time, end_time, AAXFER, CallToken, u_field,wscrd.assignedto,wscrd.ownerext,wscrd.siteid,wscrd.pbxid,wscrd.serviceid,wscrd.callcost,wscrd.callrate,hold_count,hold_time,ring_time,ring_duration,wscrd.agentid,intl_npa,callrecord,contact_ip,macid,cGroupTitle,callername,atoi(cGrpCode),wscrd.ownerdid,wscrd.nagtId,callid,DialedNumber,route_type,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,end_time,servertz,wscrd.tdiff,DISASTER_FORWARD,strlen(transfer_token) >0 ? atoi(transfer_token) : 0,isacdcall,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,hold_stats,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall,max_hold_time,(nMissedcall_cause == 15) || (nMissedcall_cause == 14) ?nMissedcall_cause:0,cIvr_Destination,cIvr_Time,nAAduration,nSpam, nVerified, nImportedContactsID, rec_format);
				if(strlen(sqlcommand)) {
					LOG(L_INFO, "[wsfifo::racc_fifo_write_final_data] IVR INSERT qry : '%s'\n" , sqlcommand);
		
					if ( strlen(site)>1&&(!isdigit(site[0])) ){
						nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
						if(nRet == -1)
						{
							LOG(L_WARN , "[wsfifo::racc_fifo_write_final_data]Failed qry : '%s'\n" , sqlcommand);
							return;
						}
					}
				}		
			}	

			bzero (sqlcommand, sizeof (sqlcommand));
			if(nIscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
				nImportedContactsID = 0;
			} else {
			    get_importedContact_details(acc_tblname, wscrd.siteid, ((wscrd.calltype == PSTN_FORWARD) ? t_field : ((offnet_call==1) ? transfer_number : ((nCallType == nRoutingPlanEXT_CallType)? r_from : f_field))), wscrd.agentid, &nImportedContactsID);
			}
			  snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ACCOUNTNAME,acc_tblname, uid, (offnet_call==1)?transfer_number:((nCallType == nRoutingPlanEXT_CallType)? r_from : f_field), t_field, (offnet_call==1)?tf_duration:t_diff, (offnet_call==1)?transferred_time:start_time, end_time,(nCallType == nRoutingPlanEXT_CallType)? IP_FORWARD:wscrd.calltype, CallToken, u_field,wscrd.assignedto,wscrd.ownerext,wscrd.siteid,wscrd.pbxid,wscrd.serviceid,wscrd.callcost,wscrd.callrate,hold_count,hold_time,ring_time,ring_duration,wscrd.agentid,intl_npa,callrecord,contact_ip,macid,group_title,callername,groupcode,wscrd.ownerdid,wscrd.nagtId,callid,DialedNumber,route_type,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,end_time,servertz,wscrd.tdiff,Phone_Type,strlen(transfer_token) >0 ? atoi(transfer_token) : 0,isacdcall,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,hold_stats,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall,max_hold_time,(nMissedcall_cause == 15) || (nMissedcall_cause == 14) ?nMissedcall_cause:0,cIvr_Destination,cIvr_Time,nAAduration, nSpam, nVerified, nImportedContactsID, rec_format);

		}
	}

	if(strlen(sqlcommand)) {
		LOG(L_INFO, "[wsfifo::racc_fifo_write_final_data] INSERT qry : '%s'\n" , sqlcommand);

		if ( strlen(site)>1&&(!isdigit(site[0])) ){
			nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
			if(nRet == -1)
			{
				LOG(L_WARN , "[wsfifo::racc_fifo_write_final_data]Failed qry : '%s'\n" , sqlcommand);
				return;
			}	
			if(Reporting_flag && CallToken){
				nRet = InsertReductionDetailsToDB(CallToken,acc_tblname,start_time,wscrd.siteid,nDataCenterID);
				if(nRet == -1)
				{
					LOG(L_WARN , "[wsicpfifo::racc_fifo_write_final_data]Failed query : '%s'\n" , sqlcommand);
					return;
				}	
			}		
		}
	}

	if(mutType == Voice_Mail){
		memset(sqlcommand,0,sizeof(sqlcommand));
		snprintf(sqlcommand,sizeof(sqlcommand)-1,UPDATE_VML_ENDTIME,end_time,CallToken);
		nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
		if(nRet == -1)
		{
			LOG( L_INFO, "[wsicpfifo::racc_fifo_write_final_data]Voicemail Failed query : '%s'\n",sqlcommand);
			return;
		}
	}

	/* Added for extra leg insertion for PSTN outbound and International 3pcc calls  --Srinivas*/
	if(ws_ctccall_flag && (ctcOmsFlag &&  strcmp(DialedNumber,t_field) && ((wsuri2.group && isdigit(wsuri2.group[0]) && !strncmp(r_to,CTCTYPE,4)) || strncmp(r_to,CTCTYPE,4)))){
		memset(DialedNumber, 0 ,sizeof(DialedNumber));
		strncpy(DialedNumber,cFromTag,sizeof(DialedNumber)-1);
		is_pstoutbound=check_international_npa(DialedNumber,&dis_siteid);
		if(!strncmp(DialedNumber,"011",3)){
			need_extraleg = 1;
			call_type = INT_OUTBOUND;
			memset(intlnpa_field, 0 ,sizeof(intlnpa_field));
			strncpy(intlnpa_field,DialedNumber,sizeof(intlnpa_field)-1);
			wscrd.callrate = GetInternationalCallCost(start_time,intlnpa_field,intl_npa);
			wscrd.pulserate = PulseRate;
			calculatecost(&wscrd,wscrd.callduration);
			wscrd.calltype = INT_OUTBOUND;
		}else if(GetTollFreeCodes(DialedNumber) == 1){
			need_extraleg = 1;
			wscrd.calltype = TOLLFREE_OUTBOUND;
			call_type = TOLLFREE_OUTBOUND;
		}else if((is_pstoutbound == 0) || (is_pstoutbound == 2)){
			need_extraleg = 1;
			wscrd.calltype = PSTN_FORWARD;
			call_type = PSTN_FORWARD;	
		}
		if(need_extraleg){
			GetServiceId(&wscrd,callername);
			memset(sqlcommand,0, sizeof (sqlcommand));
			if(nIscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
				nImportedContactsID = 0;
			} else {
			    get_importedContact_details(acc_tblname, wscrd.siteid, callername, wscrd.agentid, &nImportedContactsID); 
			}
			snprintf (sqlcommand, sizeof (sqlcommand),QUERY_CTCACCOUNTNAME,acc_tblname, uid,callername,DialedNumber, t_diff, start_time, end_time,wscrd.calltype, strlen(token) > 0 ? atoi(token) : 0, u_field,wscrd.assignedto,wscrd.ownerext,wscrd.siteid,wscrd.pbxid,wscrd.serviceid,wscrd.callcost,wscrd.callrate,hold_count,hold_time,ring_time,ring_duration,wscrd.agentid,intl_npa,callrecord,contact_ip,macid,group_title,callername,groupcode,wscrd.ownerdid,wscrd.nagtId,callid,DialedNumber,route_type,call_type,ano_calltype,missedcall_cause,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,hold_stats,max_hold_time, nImportedContactsID, rec_format);
			LOG(L_INFO, "[wsfifo::racc_fifo_write_final_data] INSERT qry1 : '%s'\n" , sqlcommand);
			if ( strlen(site)>1&&(!isdigit(site[0])) ){
				nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
				if(nRet == -1)
				{
					LOG(L_WARN , "[wsfifo::racc_fifo_write_final_data]Failed qry1 : '%s'\n" , sqlcommand);
					return;
				}	
			}
		}
	}
	racc_fifo_delete_records (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag);
	
	if((utType == LCR && offnet_call == 1 && CALLTYPE == INXFER) || (utType != LCR && CALLTYPE==INT_OUTBOUND))	{
		if(intllookupflag==1){
			auto_disable_international_calls(wscrd,site,f_field,t_field);
		}
	}

	return;	                                                        
}

void racc_fifo_delete_records (char *callid, char *ftag, char *ttag)
{
	char sqlcommand[512];
	int retval = 0;
	int nRet = -1;
   	bzero (sqlcommand, sizeof (sqlcommand));


	if(keepactivecallrecord){
		return;
	}
	
	snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ACCACTIVE_DELETE, callid, ftag, ttag,ftag,ttag,nServiceid);

	LOG( L_ERR, "[racc_fifo_delete_records] sqlcommand '%s'  \n" , sqlcommand );
	
	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1)
	{
		LOG(L_INFO,"[wsfifo::racc_fifo_delete_records]Failed qry : '%s'\n",sqlcommand);
		return;
	}

   	return;
}

void auto_disable_international_calls (struct wscallratedetails wscrd,char *site,char *f_field,char *t_field)
{
		char query[1024]="";
		double dbl=0.00,mbl=0.00;
		int reason = 0;
		int nRet = -1;
	
		MYSQL_ROW row;
		MYSQL_RES *res;
	
		struct timeval disabledtime;
	
		char * intlformat1="SELECT (dailylimit-dailyusage) dbl,(monthlylimit-monthlyusage) mbl from accountintlbalances where siteid = %d OR sitename='%s'";
		char * intlformat2="UPDATE accountintlbalances set dailyusage = CASE WHEN curdate() > date(eventdate) THEN %g ELSE dailyusage + %g END, monthlyusage = CASE when ( (year(curdate()) = year(eventdate) && MONTH(curdate()) > MONTH(eventdate))  or year(curdate()) > year(eventdate) ) THEN %g ELSE monthlyusage + %g END, eventdate = NOW() WHERE siteid = %d";
		char * intlformat3="UPDATE siteinfo_new set intlenabled = 0,intl_limit_exceed = 1 where siteid='%d'";
		char *intlformat4= "insert into intlcallingautodisablehistory(siteid,sitename,fromnumber,tonumber,disabledtime,reason) values(%d,'%s','%s','%s',now(),%d)";
	
		
		snprintf(query,sizeof(query)-1,intlformat2,wscrd.callcost,wscrd.callcost,wscrd.callcost,wscrd.callcost,wscrd.siteid);
		LOG( L_ERR, "[wsfifo::auto_disable_international_calls]Query 2: %s\n", query);
	
		nRet = ExecuteDbQuery(&db_live_handle , query , LIVE , 0);
		if(nRet == -1)
		{
			LOG(L_INFO,"[wsfifo::auto_disable_international_calls]Failed qry : '%s'\n",query);
			return;
		}
	
		memset(query,0x00,sizeof(query));
		snprintf (query, sizeof(query)-1,intlformat1,wscrd.siteid,site);
		LOG( L_ERR, "[wsfifo::auto_disable_international_calls]Query 1: %s\n", query);
		
		nRet = ExecuteDbQuery(&db_live_handle , query , LIVE , 0);
		if(nRet == -1)
		{
			LOG(L_INFO,"[wsfifo::auto_disable_international_calls]Failed qry1 : '%s'\n",query);
			return;
		}

	
		res = mysql_store_result(db_live_handle);
		if(res){
			if(mysql_num_rows(res) > 0 ){
				row = mysql_fetch_row(res);
					if(row[0]){
						dbl=atof(row[0]);
					}
					if(row[1]){
				  		mbl=atof(row[1]);
					}
			}
			mysql_free_result(res);
		}

		LOG(L_ERR,"dbl:%g mbl:%g\n",dbl,mbl);
	    if( (dbl <= 0.00) || (mbl <= 0.00) ){
	        if((dbl<=0.00) && (mbl<=0.00)){
				reason=3;
			}else if(dbl<=0.00){
				reason=1;
			}else{
			    reason=2;
			}
		memset(query,0x00,sizeof(query));
		snprintf(query,sizeof(query)-1,intlformat3,wscrd.siteid);
		nRet = ExecuteDbQuery(&db_live_handle , query , LIVE , 0);
		if(nRet == -1)
		{
			LOG(L_INFO,"[wsfifo::auto_disable_international_calls]Failed qry2 : '%s'\n",query);
			return;
		}

		memset(query,0x00,sizeof(query));
		snprintf(query, sizeof(query)-1,intlformat4,wscrd.siteid,site,f_field,t_field,reason);
		nRet = ExecuteDbQuery(&db_wh_handle , query , WARE , 0);
		if(nRet == -1)
		{
			LOG(L_INFO,"[wsfifo::auto_disable_international_calls]Failed qry3 : '%s'\n",query);
			return;
		}
	}
}

int InsertBillingFor611BlockedCalls(const char * hm_buf, char *token,struct wscallratedetails *wscrd,int nServiceid,struct wscallratedetails *wscrd1,int nDataCenterID,char * time_s,int lcr_calltype,struct ws_uri *wsuri,int is_ctcflag, char * f_field) {

    int nUpdateFlag =   0;
    int nRet        =   0;
    int nExtension  =   0;
    char cQuery[2094]   =   "";
    char cSitename[256] =   "",*cPtr = NULL, cTmpSitename[256]= "", cDID[256] = "";
    struct wscallratedetails wscrd3,wscrd4;
    struct ws_uri wsuri1,wsuri2;
    int nMissedCause = 0, nRejectCode = 0;

    memset(cQuery, 0, sizeof(cQuery));
    memset(cSitename, 0, sizeof(cSitename));
    memset(cTmpSitename, 0, sizeof(cTmpSitename));
    memset(cDID, 0, sizeof(cDID));
    memset(&wsuri1, 0x00,sizeof(wsuri1));
    memset(&wsuri2, 0x00,sizeof(wsuri2));
    memset(&wscrd3, 0x00,sizeof(wscrd3));
    memset(&wscrd4, 0x00,sizeof(wscrd4)); 
    if(strlen(wsacc_call.cSix11extensionsitename) > 0 ){/* if user dialed 611 in callerid block case it will come here to Get callee info from db */
       
	    if(!strncmp(wsacc_call.cSix11extensionsitename, "SPAM",4)) { /* Added for stir-shaken cdr changes */
		    parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri1);
		    if(wsuri1.command != NULL &&  wsuri1.context != NULL ) {
			    printf("%s, %s\n", wsuri1.command, wsuri1.context); 
			    nExtension = atoi(wsuri1.command);
			    strncpy(cTmpSitename, wsuri1.context,sizeof(cTmpSitename)-1);
		    }
	    }else{	
		    cPtr = strchr(wsacc_call.cSix11extensionsitename, '#');
		    if(cPtr) {
			    *cPtr = '\0';
			    cPtr++;
			    if(cPtr) {
				    strncpy(cTmpSitename, cPtr,sizeof(cTmpSitename)-1);
			    }
			    nExtension = atoi(wsacc_call.cSix11extensionsitename);
		    }
	    }
        Get611CalleeInfo(&wscrd3,"",nExtension,cTmpSitename); 
    }else  if(wsacc_call.to && strlen(wsacc_call.to) > 0){/* if user dialed aa+did option in callerid block case it will come here to Get callee info from db */
        if(!strncmp(wsacc_call.to,"pst-",4)){
            parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri1);
            memset(cDID,0,sizeof(cDID)-1);
            if(wsuri1.command != NULL){
            snprintf(cDID,sizeof(cDID)-1,"%s",wsuri1.command);
            }
        }
        if(strlen(cDID) > 0 && ((isdigit(cDID[0])) || (wsacc_call.to[0] == '+'))){
            Get611CalleeInfo(&wscrd3,cDID,-1,""); 
        }
    }

    if((strlen(wscrd3.sitename) > 0) && (wsacc_call.nMissedCallFlag == 1 || wsacc_call.nMissedCallFlag == 3)){
	    if(wsacc_call.nMissedCallFlag == 3) {
		    nMissedCause = wsacc_call.reasoncode;
		    nRejectCode = nSpamRejectCode;
	    }else{
		    nMissedCause = F_CALL_REJECTED_DUE_TO_CALLER_ID_BLOCK; 
		    nRejectCode = nCallerIdBlockRejectCode;
	    }
        snprintf (cQuery, sizeof (cQuery),QUERY_INSERT_ACCACTIVE_MISSED_CALL,
                wsacc_call.uri, wsacc_call.callid, wsacc_call.branch, wsacc_call.fromtag, (lcr_calltype == nRoutingPlanEXT_CallType) ? f_field : wsacc_call.from, wsacc_call.to, strlen(wsacc_call.rpid)>0?wsacc_call.rpid:wsacc_call.from, time_s, wsacc_call.method, strlen(wsuri->actual_token) > -1 ? wsuri->actual_token : token,wsacc_call.contact, who, wscrd3.siteid, wscrd3.pbxid > 0 ? wscrd3.pbxid : wscrd1->pbxid,strlen (token) > 0 ? atoi(token) : 0,nServiceid,wscrd3.agentid,wscrd3.group_title,wsacc_call.callername,wscrd3.groupcode,wsacc_call.DialedNumber,wscrd3.ownerext,wscrd3.ownerdid,wscrd3.nagtId,wscrd1->serviceid,is_ctcflag,nDataCenterID,wsacc_call.AccUser,nMissedCause,lcr_calltype==nRoutingPlanEXT_CallType?nRoutingPlanEXT_CallType:0,1,"",0,time_s,wscrd3.sitename, nRejectCode);

        LOG(L_WARN , "[wsfifo::InsertBillingFor611BlockedCalls] query : '%s'\n", cQuery);
        nRet = ExecuteDbQuery (&db_acc_active , cQuery, ACCACTIVE , 0);
        if(nRet == -1) {
            LOG(L_WARN , "[wsfifo::InsertBillingFor611BlockedCalls] Failed query 333 : '%s'\n", cQuery);
        }   

    }

    if(wsacc_call.nMissedCallFlag == 2){
        memset(cQuery, 0, sizeof(cQuery));
        nRet  = parse_ws_fifo_uri_buffer(wsacc_call.uri, strlen(wsacc_call.uri), &wsuri2);
        if(nRet == 1){
        Get611CalleeInfo(&wscrd4,"",atoi(wsuri2.command),wsuri2.context);
        }
        if((strlen(wscrd4.sitename) > 0)){
            snprintf (cQuery, sizeof (cQuery),QUERY_INSERT_ACCACTIVE_MISSED_CALL,
                    wsacc_call.uri, wsacc_call.callid, wsacc_call.branch, wsacc_call.fromtag, (lcr_calltype == nRoutingPlanEXT_CallType) ? f_field : wsacc_call.from, wsacc_call.to, strlen(wsacc_call.rpid)>0?wsacc_call.rpid:wsacc_call.from, time_s, wsacc_call.method, strlen(wsuri->actual_token) > -1 ? wsuri->actual_token : token,wsacc_call.contact, who, wscrd4.siteid, wscrd4.pbxid > 0 ? wscrd4.pbxid : wscrd1->pbxid,strlen (token) > 0 ? atoi(token) : 0,nServiceid,wscrd4.agentid,wscrd4.group_title,wsacc_call.callername,wscrd4.groupcode,wsacc_call.DialedNumber,wscrd4.ownerext,wscrd4.ownerdid,wscrd4.nagtId,wscrd1->serviceid,is_ctcflag,nDataCenterID,wsacc_call.AccUser,300,lcr_calltype==nRoutingPlanEXT_CallType?nRoutingPlanEXT_CallType:0,1,"",0,time_s,wscrd4.sitename, nRejectCode);

            LOG(L_WARN , "[wsfifo::InsertBillingFor611BlockedCalls] query : '%s'\n", cQuery);
            nRet = ExecuteDbQuery (&db_acc_active , cQuery, ACCACTIVE , 0);
            if(nRet == -1) {
                LOG(L_WARN , "[wsfifo::InsertBillingFor611BlockedCalls] Failed query 333 : '%s'\n", cQuery);
            }
        }
    }

    LOG(L_WARN , "[wsicpfifo::InsertBillingFor611BlockedCalls]wsacc_call.to : '%s'  wsacc_call.cSix11extensionsitename : '%s' nExtension : '%d' cTmpSitename : '%s' token : '%s' \n",wsacc_call.to,wsacc_call.cSix11extensionsitename,nExtension,cTmpSitename,token);                   
    return 0;
}

void racc_fifo_db_request(const char *hm_buf)
{
	struct tm *tm;
	time_t timep;
	char time_s [32] = "", sqlcommand[2048] = "", tempowner[64]="";
	char st_time[32]="";
	int siteid = 0,pbxid = 0;
	char *format_string;
	int retval = 0,hold_cnt = 0,hold_duration = 0,is_grp_call=0,lcr_flag = 0,lcr_calltype = 0,nRespCause=0;
	struct ws_uri wsuri;
	char *hold_buf = NULL,*ptr = NULL,*hst = NULL ,*ptr1 = NULL;
	int nRet = -1,i=0,is_ctcflag = 0, nReturn = -2;
	unsigned int call_token = 0;
	char token [15] = "" , buf[64] = "" , ext[128] = "" ,contact_ip[64]="",temp_contact_ip[256]="",agentid[64]="",agentid1[64]="",cExtn[32]="",cExtension[15]="",sitename[128],*owner=NULL,Ivr_Destination[16]="", f_field[256] = "";
	char cCosToken[32] = "";
	struct wscallratedetails wscrd,wscrd1;
	struct ws_uri  wsuri1,wsuri2;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int nCall_Type = 0;
	
	/* All required fields are here, now get the timestamp */
   	timep = wsacc_call.timestamp;
	tm = localtime (&timep);
   	strftime (time_s, 32, "%Y-%m-%d %H:%M:%S", tm);
	
	bzero 	(sqlcommand, sizeof (sqlcommand));
	bzero(token, sizeof(token));
	memset(&wscrd , 0 , sizeof(wscrd));
	memset(&wscrd1 , 0 , sizeof(wscrd1));
	memset(buf , 0 , sizeof(buf));
	memset(&wsuri, 0, sizeof(struct ws_uri));
	memset(&wsuri2, 0, sizeof(struct ws_uri));
	memset(cCosToken, 0, sizeof(cCosToken));

	switch (wsacc_call.method){
		case RACC_INVITE:
		{

			if(!strncmp(wsacc_call.to,"lcr-",4)) {
				lcr_flag = 1;
			}
		
			/*To be moved the racc module itself and try getting clean data here */
			if ( strlen(wsacc_call.fromtag)>1 && strlen(wsacc_call.totag)>1 ) {
				LOG( L_ERR,"[wsfifo::racc_fifo_db_request]ERROR RACC::re-INVITE don't insert into database \n\n\n");
				return;
			}

			/*Added to block billing for *02 3pcc call as we are billing at wsicp  --Anusha*/
			if(ws_ctccall_flag && (!strncmp(wsacc_call.to,CTCTYPE,4) && strstr(wsacc_call.fromtag,CTC_TAG) && strstr(wsacc_call.fromtag,"adh-") &&  !strncmp(wsacc_call.DialedNumber,"*02",3))){
				LOG(L_WARN," As this is the adh- uri We don't need to insert into acc_active \n");
				return;
			} 
						
			if(wsacc_call.uri && strlen(wsacc_call.uri) > 0) {
				if(lcr_flag == 1) {	
					memset(buf,0,sizeof(buf));
					strncpy(buf,wsacc_call.from,sizeof(buf)-1); 
					get_agentid(buf);
					lcr_calltype = LcrCallType;
				}
                else if(wsacc_call.nRoutingPlanExtFlag == 1) {
                    if((parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri2) == 1)) {
                        strncpy(buf, wsuri2.command, sizeof(buf) - 1);
                        owner = buf;
                        for (i=0; wsuri2.context[i]; i++) {if ('-' == wsuri2.context[i]) wsuri2.context[i] = '.';}
                            GetSiteId(&wscrd, wsuri2.context);
                            wscrd1.siteid = wscrd.siteid;
                            GetServiceId(&wscrd1, owner);
                    }
					lcr_calltype = nRoutingPlanEXT_CallType;
				}
                else{
					get_wsagent_id(wsacc_call.uri , buf , ext, sizeof(ext) - 1,sizeof(buf));
				}
				/* Added to fix group name issue in billing for pickups --Anusha*/
				if(!strncmp(wsacc_call.to,"cmd-",4)){
					if((parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri2) == 1)) {
						if(strlen(wsuri2.command)<3 || !strcmp(wsuri2.command,"001")){
							is_grp_call=1;
						}
                        if(wsacc_call.nRoutingPlanExtFlag == 1) {
                            get_group_ext_did(atoi(wsuri2.group), "", &wscrd);
                        }
					}
				}

				if(ws_ctccall_flag && (!strncmp(wsacc_call.to,CTCTYPE,4) && !strncmp(wsacc_call.fromtag,"pkp-",4)))
					memset(ext,0,sizeof(ext));
    
				if(wsacc_call.nRoutingPlanExtFlag == 1 && (!strncmp(wsacc_call.to, "gext-",5)))
				{
					get_wsagent_details(&wscrd , buf , ext, 3); 	// 3 is for HUNT group Extension
				}else {
					get_wsagent_details(&wscrd , buf , ext,is_grp_call);
				}

				memset(agentid,0x00,sizeof(agentid));
				if(strlen(buf) > 0 && strchr(buf,'@') ){
					strncpy(agentid,buf,sizeof(agentid)-1);
				}else if (strlen(ext) > 0){
					strncpy(agentid,ext,sizeof(agentid)-1);
				}
				if(!wscrd.siteid) {
					if((ptr = strchr(agentid, '@'))) {
						ptr++;
					} else {
						ptr = buf;
					}
                    if(ptr != NULL && (wsacc_call.nMissedCallFlag == 0)) {
					    GetSiteId(&wscrd,ptr);
                    }
				}
				wscrd1.siteid=wscrd.siteid;
				owner=agentid;
                if(wsacc_call.nRoutingPlanExtFlag != 1 && (wsacc_call.nMissedCallFlag == 0)) {
				    GetServiceId(&wscrd1,owner);
                }
			}

			/* Extracting timestamp for token: this identifies related calls.
			 * Length of a token is 10 chars.
			 */
			/*To be moved the racc module itself and try getting clean data here */
			if(!strncmp(wsacc_call.uri,"inv-",4)||!strncmp(wsacc_call.uri,"cos-",4)) {
				LOG( L_ERR,"[wsfifo::racc_fifo_db_request]ERROR RACC::Someone Dialed unassigned Number, We aren't charging that call\n");
				return; 
			}			
			if (wsacc_call.uri[0] == '+') {
				/* Strip off '+', creating problems when checking the account type */
				strncpy(wsacc_call.uri, &wsacc_call.uri[1],sizeof(wsacc_call.uri)-1);		
			}

			/*Added for fixing psib issues in missed calls for PSTN inbound cases --vravi/saidulu */	
			if((parse_ws_fifo_uri_buffer(wsacc_call.uri, strlen(wsacc_call.uri), &wsuri) == 1)) {
				if(wsuri.token && strlen (wsuri.token) > 10) {
					memset(token, 0, sizeof(token));
					snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
					wsuri.token = token;
				}
				else {
					strncpy(token , wsuri.token , sizeof(token)-1);
				}
			}
			
			/*Added if condition to get token in case of obxfer type and parked callback call*/
			if(strlen(token) == 0) {
				if((parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri) == 1) || (parse_ws_fifo_uri_buffer(wsacc_call.from, strlen(wsacc_call.from), &wsuri) == 1)) {
					if(wsuri.token && strlen(wsuri.token) > 10) {
						memset(token, 0, sizeof(token));
						snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
						wsuri.token = token;
					}
					else {
						strncpy(token , wsuri.token , sizeof(token)-1);
					}
				}
			}
			
			if(!strncmp(wsacc_call.to, "acdext-",7) && strlen(wscrd.group_title) <= 0){
				strncpy(wscrd.group_title,wsacc_call.QName,sizeof(wscrd.group_title)-1);
			}
			/*Added to get group title for pstn to AA group + dtmf as hunt group ext call case */
			if(!strncmp(wsacc_call.to, "gext-",5) && !strncmp(wsacc_call.uri, "oms-",4) && strlen(wscrd.group_title) <= 0){
				get_grouptitle(&wscrd,3);
			}
            /*Added to get group title for AA to PSTN */
            if(!strncmp(wsacc_call.to, "pst-",4) && strlen(wscrd.group_title) <= 0){
                get_grouptitle(&wscrd,4);
            }	

			if (0 == token[0]) {/* To be decided for Strrchr function instead of directly overlap copying*/
				if (! (strncmp (wsacc_call.to, "ext-", 4)) || ! (strncmp (wsacc_call.to, "cmd-", 4)) || !(strncmp (wsacc_call.to, "bla-", 4)) || !(strncmp (wsacc_call.to, "iext-", 5)) || !(strncmp(wsacc_call.to, "gext-", 5)) || !(strncmp(wsacc_call.to, "acdext-",7) || !(strncmp(wsacc_call.to, "obxfer-", 7)))) {
					strncpy (token, &wsacc_call.to[strlen(wsacc_call.to)-10], 10);
					if (! isdigit (token [0])) bzero (token, 15);
				}
			}
			if(ws_ctccall_flag && !strncmp(wsacc_call.to,CTCTYPE,4) && strstr(wsacc_call.fromtag,CTC_TAG)){
				is_ctcflag = 1;   /*flag to identify the acc_active entries for 3pcc calls*/
				if(!strncmp(wsacc_call.fromtag,"cos-",4) || !strncmp(wsacc_call.fromtag,"inv-",4)){
					LOG( L_ERR,"[wsfifo::racc_fifo_db_request]ERROR RACC::Someone Dialed unassigned Number, We aren't charging this CTC call\n");
					return;  //added to block the invalid ext/DID billing leg
				}
				if((!strncmp(wsacc_call.fromtag,"xfer-",5) || !strncmp(wsacc_call.fromtag,"mext-",5) || (!strncmp(wsacc_call.fromtag,"wsctcb-",7) && !strncmp(wsacc_call.to,"ctc-0-",6))) && !strncmp(wsacc_call.uri,"oms-",4) && strstr(wsacc_call.uri,"-transfer-")){
					LOG( L_ERR,"[wsfifo::racc_fifo_db_request] we are blocking for DID-pstn outbound PSTN call uri : %s fromtag : %s\n",wsacc_call.uri,wsacc_call.fromtag);
					return; //blocking the extra leg for DID-pstn outbound  PSTN call
				}
				if(parse_ws_fifo_uri_buffer(wsacc_call.to,strlen(wsacc_call.to),&wsuri2) == 1){
					/* Added for removing 3rd leg in case of (DID-international) and (conf-International) PSTN dialing*/
					if(!strncmp(wsuri2.group,"011",3) && isdigit(wsuri2.command[0])){
						if ((strlen(wsuri2.command) > 5) || ((strlen(wsuri2.command) <= 5) && !strncmp(wsacc_call.fromtag,"cnf-",4))){
							LOG( L_ERR,"[wsfifo::racc_fifo_db_request] we are blocking for (DID-international) and (conf-International) PSTN dialing  wsuri2.command : %s fromtag : %s\n",wsuri2.command,wsacc_call.fromtag);
							return;
						}
					}
				}
			}
			if(strlen(wsuri.actual_token) > 0 && strstr(wsuri.actual_token,"wspib") && strlen(token)>0 ){
				snprintf(sqlcommand,sizeof(sqlcommand)-1,UPDATE_CALLERNAME,wsacc_call.callername,atoi(token),nServiceid,PSTN_INBOUND);/*updating intial callername in wsicp entry with corresponding callername*/
				if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
					LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
					return;
				}
			}

            if(lcr_calltype == nRoutingPlanEXT_CallType) {
                memset(f_field, 0x0, sizeof(f_field));
                strncpy(f_field, wsacc_call.from, sizeof(f_field) - 1);
                if(!strncmp(wsacc_call.uri, "xfer-", 5) || !strncmp(wsacc_call.uri, "agt-", 4) || !strncmp(wsacc_call.uri, "rec-", 4) || !strncmp(wsacc_call.uri, "vmr-", 4) || !strncmp(wsacc_call.uri, "wsprov-", 7) || !strncmp(wsacc_call.uri, "inv-", 4) || !strncmp(wsacc_call.uri, "9994-", 5) || !strncmp(wsacc_call.uri, "dir-", 4) || !strncmp(wsacc_call.uri, "oms-", 4) || !strncmp(wsacc_call.uri, "gext-", 5) || !strncmp(wsacc_call.uri, "wsucapp-", 8) || !strncmp(wsacc_call.uri, "acd-", 4) || !strncmp(wsacc_call.uri, "acdext-", 7) || !strncmp(wsacc_call.uri, "acdcnam-", 8) || !strncmp(wsacc_call.uri, "ivr-", 4) || !strncmp(wsacc_call.uri, "nop-", 4) || !strncmp(wsacc_call.uri, "opr-", 4)) {
                    strncpy(f_field, wsacc_call.cAgentId, sizeof(f_field) - 1);
                }
                if((strlen(wscrd.group_title) <= 0) && strlen(wsacc_call.cGrpName) > 0 && strchr(wsacc_call.cGrpName, '-') == NULL 
                                                                                       && strchr(wsacc_call.cGrpName, '@') == NULL) {
                    strncpy(wscrd.group_title, wsacc_call.cGrpName, sizeof(wscrd.group_title) - 1);
                }
            }

			memset(sqlcommand,0,sizeof(sqlcommand));
            if (wsacc_call.nMissedCallFlag > 0){
                memset(sqlcommand, 0, sizeof(sqlcommand));
                InsertBillingFor611BlockedCalls(hm_buf, token, &wscrd, nServiceid, &wscrd1, nDataCenterID, time_s,lcr_calltype,&wsuri,is_ctcflag, f_field);
                break;
            }else if ( isdigit(wsacc_call.uri[0]) ) {
				snprintf (sqlcommand,sizeof(sqlcommand)-1,QUERY_INSERT_ACCACTIVE_DIGIT, wsacc_call.uri, wsacc_call.callid, wsacc_call.branch, wsacc_call.fromtag, wsacc_call.from,
				wsacc_call.to, strlen(wsacc_call.rpid)>0?wsacc_call.rpid:wsacc_call.from, time_s, wsacc_call.method, strlen(wsuri.actual_token) > 0 ? wsuri.actual_token : token,
				wsacc_call.uri, wsacc_call.contact, who, wscrd.siteid ,  wscrd.pbxid > 0 ? wscrd.pbxid : wscrd1.pbxid,strlen (token) > 0 ? atoi(token) : 0,nServiceid ,wscrd1.agentid, wscrd.group_title,wsacc_call.callername,wscrd.groupcode,wsacc_call.DialedNumber,wscrd1.ownerext,wscrd1.ownerdid,wscrd1.nagtId,wscrd1.serviceid,is_ctcflag,nDataCenterID,wsacc_call.AccUser,wsacc_call.is_direct_call,lcr_calltype);
			}
			else {
				/* Added code changes to insert call_type as 51 in case of transfer to pstn to avoid stucks in acc_active --Prashant \Aug-2020/ */
				if( !strncmp(wsacc_call.uri, "oms-",4) && strstr(wsacc_call.uri,"-transfer-")) {
					nCall_Type = nXferCallType;
				}

				snprintf (sqlcommand, sizeof (sqlcommand),QUERY_INSERT_ACCACTIVE,
				wsacc_call.uri, wsacc_call.callid, wsacc_call.branch, wsacc_call.fromtag, (lcr_calltype == nRoutingPlanEXT_CallType) ? f_field : wsacc_call.from, wsacc_call.to, strlen(wsacc_call.rpid)>0?wsacc_call.rpid:wsacc_call.from, time_s, wsacc_call.method, strlen(wsuri.actual_token) > 0 ? wsuri.actual_token : token,wsacc_call.contact, who, wscrd.siteid, wscrd.pbxid > 0 ? wscrd.pbxid : wscrd1.pbxid,strlen (token) > 0 ? atoi(token) : 0,nServiceid,wscrd1.agentid,wscrd.group_title,wsacc_call.callername,wscrd.groupcode,wsacc_call.DialedNumber,wscrd1.ownerext,wscrd1.ownerdid,wscrd1.nagtId,wscrd1.serviceid,is_ctcflag,nDataCenterID,wsacc_call.AccUser,0,lcr_calltype==nRoutingPlanEXT_CallType?nRoutingPlanEXT_CallType:nCall_Type, wsacc_call.nWSacdaacallerid);

			}
			LOG( L_INFO, "INVITE: %s\n\n", sqlcommand);
			break;
		}

		/* 180 Ringing, We dont handle this. Coded for brevity */
		case RACC_RINGING:
			if((parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri)== -1) || !strncmp(wsuri.type,"ptt" , 3)) {
				LOG(L_WARN,"[wsfifo::racc_fifo_db_request] Parse URI Failed : %s\n", wsacc_call.to);
				snprintf (sqlcommand, sizeof(sqlcommand)-1,QUERY_RINGING_UPDATE_ACCACTIVE, wsacc_call.method,time_s,wsacc_call.callid, wsacc_call.fromtag,wsacc_call.branch,who);
			}else {
				if(wsuri.token && strlen (wsuri.token) > 10) {
					memset(token,0,sizeof(token));
					snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
					wsuri.token = token;
				}
				snprintf (sqlcommand, sizeof(sqlcommand)-1,QUERY_RINGING_ACCACTIVE_URI, wsacc_call.method,time_s, strlen (wsuri.token) > 0? atoi(wsuri.token): 0, nServiceid,wsacc_call.branch);
			}
            break;
		/* Coded for ACK, Has never reached here so far... */		
		case RACC_ESTABLISHED:
			bzero(sqlcommand,sizeof(sqlcommand));
			if(parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri)== -1 ) {
				LOG(L_WARN,"[wsfifo::racc_fifo_db_request] Parse URI Failed : %s\n", wsacc_call.to);
	        		snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ESTABLISHED_ACCACTIVE,wsacc_call.method, time_s, wsacc_call.totag, wsacc_call.branch, wsacc_call.callid, wsacc_call.fromtag, who);
			}
			else {
				if(wsuri.token && strlen (wsuri.token) > 10) {
					memset(token,0,sizeof(token));
					snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
					wsuri.token = token;
				}
				snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ESTABLISHED_ACCACTIVE_URI, wsacc_call.method, time_s, wsacc_call.totag, wsacc_call.branch,strlen(wsuri.token) > 0 ? atoi(wsuri.token) : 0 , nServiceid);
			}
			
			LOG( L_WARN, "ACK: %s\n", sqlcommand);
			break;
		/* 	Received OK: INVITE successful.
		 *	Note that we again insert branch to make life easy.
		 */

			
		/* VMIP Billing issue fix - Vijay 07052009 */		
		case RACC_TEMPMOVED:
			if ( parse_ws_fifo_uri_buffer(wsacc_call.uri, strlen(wsacc_call.uri), &wsuri)== -1 ) {
					LOG(L_WARN,"[racc_fifo_db_request] Parse URI Failed : %s\n", wsacc_call.uri);
					return;
			}

			/* Added for class of service patch  --Aug-5-2020*/			
			if(wsuri.token && strlen(wsuri.token)>0) {
				strncpy(cCosToken, wsuri.token, sizeof(cCosToken));
			}

			if(wsuri.token && strlen (wsuri.token) > 10) {
				memset(token,0,sizeof(token));
				snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
				wsuri.token = token;
			}

			/* Added for class of service patch  --Aug-5-2020*/			
			if(wsuri.type && !strncmp(wsuri.type,"vml",3) && wsacc_call.resp_cause == F_INV ) {
				wsuri.token = cCosToken;	
			}

			if(wsuri.type && !strncmp(wsuri.type,"cnf",3)) {
					 bzero(sqlcommand,sizeof(sqlcommand));
					 snprintf (sqlcommand, sizeof (sqlcommand),QUERY_TMPMOVED_ACCACTIVE_CNF,wsacc_call.method,wsacc_call.uri,time_s,strlen (wsuri.token) > 0 ? atoi(wsuri.token): 0);
			}else if(wsuri.type && !strncmp(wsuri.type,"vml",3)){

				/* Added below for missedcall changes for timeout and rejected cases --vravi/saidulu*/		
				#if 0
				bzero(sqlcommand,sizeof(sqlcommand));
				if(!strncmp(wsacc_call.to, "acdxfer-", 8))
				{
					snprintf (sqlcommand, sizeof (sqlcommand),QUERY_TMPMOVED_MISSED_CALLERSIDETRANSFER_UPDATE, time_s,strlen (wsuri.token) > 0 ? atoi(wsuri.token): 0, nServiceid);	
				}
				else
				{
				#endif
					if(wsuri.command && strlen(wsuri.command) > 0 && strcmp(wsuri.command,"00") && wsuri.context && strlen(wsuri.context)>0 && wsuri.token && strlen(wsuri.token)>0){
						memset(sqlcommand,0,sizeof(sqlcommand));
						snprintf (sqlcommand, sizeof (sqlcommand),QUERY_GET_ACTIVEDETAILS, wsacc_call.callid,nServiceid);	
						if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
							LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
							return;
						}
						res = mysql_store_result(db_acc_active);
						if(!res || (mysql_num_rows(res) <= 0)){
							memset(sqlcommand,0,sizeof(sqlcommand));
							get_wsagent_details(&wscrd , wsuri.context ,wsuri.command ,2);
							GetServiceId(&wscrd,wsuri.command);

							if(wsacc_call.resp_cause == F_INV) {
								nRespCause = F_INV;	
							}else{
								nRespCause = F_NOREGSITER;	
							}
							snprintf (sqlcommand, sizeof (sqlcommand),QUERY_INSERT_ACCACTIVE,
							wsacc_call.uri, wsacc_call.callid, "", "", wsacc_call.from, wsacc_call.to, strlen(wsacc_call.rpid)>0?wsacc_call.rpid:wsacc_call.from, time_s, wsacc_call.method, strlen(wsuri.actual_token) > 0 ? wsuri.actual_token : token,"", who, wscrd.siteid,wscrd.pbxid,strlen (wsuri.token) > 0 ? atoi(wsuri.token) : 0,nServiceid,wscrd.agentid,wscrd.group_title,wsacc_call.callername,wscrd.groupcode,wsacc_call.DialedNumber,wscrd.ownerext,wscrd.ownerdid,wscrd.nagtId,wscrd.serviceid,is_ctcflag,nDataCenterID,wsacc_call.AccUser,wsacc_call.nVoicmailCalleridBlock == 1?15:nRespCause);
							if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
								LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
								return;
							}
						}
						if(res)
						{
							mysql_free_result(res);
						}

						if(strlen(wsuri.actual_token) > 0 && strstr(wsuri.actual_token,"wspib") ){
							memset(sqlcommand,0,sizeof(sqlcommand));
							snprintf(sqlcommand,sizeof(sqlcommand)-1,UPDATE_CALLERNAME,wsacc_call.callername,atoi(wsuri.token),nServiceid);
							if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
								LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
								return;
							}
						}
					}

					if(wsuri.token && strlen(wsuri.token) >0){
						memset(sqlcommand,0,sizeof(sqlcommand));
						LOG(L_WARN , "[wsfifo::racc_fifo_db_request] ############## wsacc_call.resp_cause : '%d' \n", wsacc_call.resp_cause);
						if(wsacc_call.resp_cause == F_INV) {
							snprintf (sqlcommand, sizeof (sqlcommand),QUERY_TMPMOVED_MISSED_UPDATE_COS, wsacc_call.uri,time_s,F_INV,wsacc_call.callid,nServiceid);
						}else{
							snprintf (sqlcommand, sizeof (sqlcommand),QUERY_TMPMOVED_MISSED_UPDATE, wsacc_call.uri,time_s,wsacc_call.callid,nServiceid);	
						}

						if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
							LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
							return;
						}
						LOG( L_ERR, "[wsfifo::temp_moved] Query :  %s \n", sqlcommand);
						memset(sqlcommand,0,sizeof(sqlcommand));
						snprintf (sqlcommand, sizeof (sqlcommand),QUERY_VML_URI_UPDATE,  wsacc_call.uri,atoi(wsuri.token));	
					}
				//}		
				LOG( L_ERR, "[racc_fifo_db_request] RACC_TEMPMOVED sqlcommand '%s'  \n" , sqlcommand );
			}else{
				LOG( L_ERR, "[wsfifo::temp_moved] Not a cnf call or vml call ,wsacc_call.callid : %s wsacc_call.to : < %s > \n",wsacc_call.callid , wsacc_call.to);
				if ( parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri)== -1 ) {
					LOG(L_WARN,"[racc_fifo_db_request] Parse URI Failed : %s\n", wsacc_call.uri);
					return;
				}
				usleep(fwd_delay_time);/*Added delay for to get the db result(request_uri from acc_active) in wsobproxy for rejected and forwarded scenario's before deleting from acc_active table*/
				/* Added changes for inserting phone forward leg in CDR's  --Saidulu*/
   				bzero(sqlcommand,sizeof(sqlcommand));
				snprintf(sqlcommand , sizeof(sqlcommand)-1 , UPDATE_PHONE_FORWARD , RACC_TERMINATING , time_s , time_s , atoi(wsuri.token) , wsacc_call.callid , nServiceid , wsacc_call.branch); 
	
				if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
					LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
					return;
				}

				racc_fifo_write_final_data (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag, wsacc_call.branch , (wsuri.token && strlen (wsuri.token) > 0 )? atoi(wsuri.token): 0, 1);

   				bzero(sqlcommand,sizeof(sqlcommand));
			}
			LOG( L_ERR, "[wsfifo::temp_moved] Query :  %s \n", sqlcommand);
			break;

		case RACC_ACCEPTED:
			/* Getting  Agentid,contact ip:port from contact of 200 OK */
			if(strlen(wsacc_call.contact) > 0){
				memset(agentid1,0x00,sizeof(agentid1));
				strncpy(agentid1,wsacc_call.contact,sizeof(agentid)-1);
		        if((ptr=strchr(agentid1,':')) && ++ptr){
					memset(agentid,0x00,sizeof(agentid));
					strncpy(agentid,ptr,sizeof(agentid)-1);
				    if(ptr=strchr(agentid,'@')){
						*ptr = '\0';
						 if(++ptr){
							if((ptr1 = strchr(ptr,';')) || (ptr1 = strchr(ptr,'>'))){
								*ptr1 = '\0';
								memset(contact_ip,0,sizeof(contact_ip));
								strncpy(contact_ip,ptr,sizeof(contact_ip)-1);
								ptr = ptr1 = NULL;
							}
						}
						if(strlen(agentid) > 0){
							/* Getting agentid id from wsuri of OMS type */
							if((strstr(agentid,"oms-")) && (parse_ws_fifo_uri_buffer(agentid, strlen(agentid), &wsuri) == 1) ){
									memset(agentid,0,sizeof(agentid));
									snprintf(agentid,sizeof(agentid)-1,"%s@%s",wsuri.group,wsuri.context);
									while((ptr1 = strchr(agentid,'-')))
	    							{
										*ptr1='.';
									}
							}else{
								get_agentid(agentid);
							}
						}
					}
				}
			}
			call_token = 0;
			memset(&wsuri, 0, sizeof(struct ws_uri));
			if(parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri) == 1) {
				if(wsuri.token && strlen(wsuri.token) > 10) {
					memset(token, 0, sizeof(token));
					snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
					wsuri.token = token;
				}
				if(wsuri.token){
					call_token = atoi(wsuri.token);
				}
			}
			snprintf (sqlcommand,sizeof(sqlcommand)-1,QUERY_RACC_ACCPTED,wsacc_call.method,wsacc_call.totag, time_s, contact_ip,wsacc_call.macid, time_s, call_token,wsacc_call.callid, nServiceid,wsacc_call.branch);
           	if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
                LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
       	        return;
       		}

			if(nEnableSleepPatch == 1) {
				/* Invite & Answer came to MSICP at the same time and processing by it from different processes.
 				* Here, in race-condition Answer update may execuite before the record insertion. To avoid this added '1'sec sleep -- Nov-30-2019*/	
				nReturn =  mysql_affected_rows(db_acc_active);
				if(nReturn == 0 || nReturn == -1) {
					sleep(1);	
					LOG(L_WARN , "[wsfifo::racc_fifo_db_request] nReturn : '%d',sqlcommand : %s \n" ,nReturn, sqlcommand);
					if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
						LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
						return;
					}
				}
			}
		
			/*Added changes for not to insert missed calls in CDR's for other users in group when one user in group is answered the call --Saidulu*/
			memset(sqlcommand,0x00,sizeof(sqlcommand));
			snprintf (sqlcommand, sizeof (sqlcommand),QUERY_ACCEPT_MISSED_DELETE,wsacc_call.callid,wsacc_call.branch);
			break;
		/* BYE Request */
		case RACC_TERMINATING:
			if(((parse_ws_fifo_uri_buffer(wsacc_call.to, strlen(wsacc_call.to), &wsuri)== -1 ) && (parse_ws_fifo_uri_buffer(wsacc_call.from, strlen(wsacc_call.from), &wsuri)== -1)) || !strncmp(wsuri.type,"ptt" , 3)) {
				snprintf (sqlcommand, sizeof(sqlcommand)-1,QUERY_TERMINATING_UPDATE_ACCACTIVE, wsacc_call.method, time_s,wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag, wsacc_call.totag, wsacc_call.fromtag, who);
			}
			else 
			{
				if(wsuri.token && strlen (wsuri.token) > 10) {
					memset(token, 0, sizeof(token));
					snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
					wsuri.token = token;
				}
				snprintf (sqlcommand, sizeof(sqlcommand)-1,QUERY_TERMINATING_UPDATE_ACCACTIVE_URI, wsacc_call.method, time_s, strlen (wsuri.token) > 0? atoi(wsuri.token): 0, nServiceid,wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag, wsacc_call.totag, wsacc_call.fromtag, who);
			}
			break;
	
		/* 408 Request Timed Out:
		 * We will not delete direct extension calls now, wait for them to go to voicemail
		 */
		case RACC_TIMEDOUT:
			LOG(L_ERR,"[RACC_TIMEDOUT] wsacc_call.restype : %d , wsacc_call.resp_cause: %d , wsacc_call.to : %s \n",wsacc_call.restype,wsacc_call.resp_cause,wsacc_call.to);
			memset(sqlcommand,0x00,sizeof(sqlcommand));
			CheckAndDeleteLcrResponses(wsacc_call.callid,0);
			if(strstr(wsacc_call.to,"acdext-")){
				snprintf (sqlcommand, sizeof (sqlcommand),QUERY_REJECTED_DELETE,wsacc_call.callid ,wsacc_call.branch,nServiceid);
			}else if(wsacc_call.resp_cause == F_TIMEOUT && (strstr(wsacc_call.to,"pst-") || strstr(wsacc_call.to,"did-"))){
				if(wsacc_call.restype == RESPONSE_IN){
                                	snprintf (sqlcommand, sizeof (sqlcommand),LCR_UPDATE_QUERY,F_TIMEOUT,0,wsacc_call.callid);
				}
			}else{
				snprintf (sqlcommand, sizeof (sqlcommand),QUERY_MISSEDCALL_UPDATE,wsacc_call.resp_cause,wsacc_call.callid ,nServiceid,wsacc_call.branch);
			}
			LOG( L_ERR, "TIMEOUT: %s\n\n", sqlcommand);
			break;

		case RACC_REJECTED:
		case RACC_DEFAULT:

				LOG(L_ERR,"[RACC_REJECTED] wsacc_call.restype : %d, %d, wsacc_call.resp_cause: %d \n",wsacc_call.restype, wsacc_call.nLcrReject, wsacc_call.resp_cause);
				memset(sqlcommand,0x00,sizeof(sqlcommand));
				CheckAndDeleteLcrResponses(wsacc_call.callid, wsacc_call.nResCode);
				if(wsacc_call.nLcrReject != 2 && wsacc_call.nLcrReject != 5 && (wsacc_call.restype == RESPONSE_IN) && (wsacc_call.resp_cause != F_DEFAULT && wsacc_call.resp_cause != F_SERVICE_UNAVILABLE) && !strstr(wsacc_call.to,"acdext-")){
					snprintf (sqlcommand, sizeof (sqlcommand),QUERY_MISSEDCALL_UPDATE,wsacc_call.resp_cause,wsacc_call.callid ,nServiceid,wsacc_call.branch);
				}else if((wsacc_call.restype == RESPONSE_OUT || wsacc_call.restype == RESPONSE_IN) && (strstr(wsacc_call.to,"acdext-") || (ws_ctccall_flag && !strncmp(wsacc_call.to,CTCTYPE,4)))){/*Added RESPONSE_IN condition to delete acc_active entries for acd calls if morethan 1 phonetype is there*/
					/* Deleting missedcalls if the flag is not enabled --vravi/saidulu */
					snprintf (sqlcommand, sizeof (sqlcommand),QUERY_REJECTED_DELETE,wsacc_call.callid ,wsacc_call.branch,nServiceid);
				}else if(wsacc_call.restype == RESPONSE_OUT && wsacc_call.resp_cause != F_SERVICE_UNAVILABLE) {
   					bzero(sqlcommand,sizeof(sqlcommand));
					if ( wsacc_call.nLcrReject == 1) {
						snprintf(sqlcommand, sizeof(sqlcommand)-1, LCR_UPDATE_QUERY, F_BLACKLISTED, CALL_DECLINED, wsacc_call.callid);
					} else if ( wsacc_call.nLcrReject == 3) {
						snprintf(sqlcommand, sizeof(sqlcommand)-1, LCR_UPDATE_QUERY, F_BLOCK_BY_FAR_END_SERVICE_PROVIDER, CALL_REJECTED, wsacc_call.callid);
					} else if ( wsacc_call.nLcrReject == 4) {
						snprintf(sqlcommand, sizeof(sqlcommand)-1, LCR_UPDATE_QUERY, F_BLOCK_BY_FAR_END_CUSTOMER, CALL_UNWANTED, wsacc_call.callid);
					}
				} else if ( wsacc_call.nLcrReject == 5 && (wsacc_call.restype == RESPONSE_IN) && (wsacc_call.resp_cause != F_DEFAULT) && !strstr(wsacc_call.to,"acdext-")) {
						snprintf(sqlcommand, sizeof(sqlcommand)-1, LCR_UPDATE_QUERY,F_INV, CALL_DECLINED, wsacc_call.callid);
				} else if ((wsacc_call.resp_cause == F_SERVICE_UNAVILABLE) && (wsacc_call.restype == RESPONSE_IN)) {
						snprintf(sqlcommand, sizeof(sqlcommand)-1, LCR_UPDATE_QUERY, F_SERVICE_UNAVILABLE, CALL_DECLINED, wsacc_call.callid);
				}
				LOG(L_ERR,"[RACC_REJECTED] QUERY:< %s %d, %d >\n",sqlcommand, wsacc_call.method, wsacc_call.nLcrReject);
				break;
		case RACC_CANCELLED:
				/* Deleting struck ACD entries from acc_active in case all rejects --vravi/saidulu */
				memset(sqlcommand,0x00,sizeof(sqlcommand));
				if(strstr(wsacc_call.to,"acdext-")){
					snprintf (sqlcommand, sizeof (sqlcommand),QUERY_REJECTED_DELETE,wsacc_call.callid ,wsacc_call.branch,nServiceid);
				}else{
					/* Added below for missed call in case of caller canceling --vravi/saidulu */
					if(wsacc_call.restype == REQUEST_FWDED){
					snprintf (sqlcommand, sizeof (sqlcommand),QUERY_CANCELLED_MISSED_UPDATE,time_s,F_DEFAULT,F_REJECT,wsacc_call.resp_cause,wsacc_call.callid ,nServiceid);
					}else if((wsacc_call.restype == REQUEST_FWDED_LOCAL)){
						snprintf (sqlcommand, sizeof (sqlcommand),QUERY_MISSEDCALL_UPDATE,wsacc_call.resp_cause,wsacc_call.callid ,nServiceid,wsacc_call.branch); 
					}
				}
				break;

		case RACC_HOLD:
			break; /*Hold metric details are updating from ums so breaking here*/
			if(parse_ws_fifo_uri_buffer(wsacc_call.from, strlen(wsacc_call.from), &wsuri)== -1 ) {
				LOG(L_WARN,"[ocpacc_fifo_db_request] Parse URI Failed : %s\n", wsacc_call.from);
			}
			
			if(hm_buf && ((hold_buf = strstr(hm_buf,"chm=")) || (hold_buf = strstr(hm_buf,"crm=")))){
				hold_buf = hold_buf+4;
				if(hold_buf) {
					if((ptr = strchr(hold_buf,'-'))){
						*ptr = '\0';
						hold_cnt = atoi(hold_buf);
						ptr++;
						if(ptr){                  
							hold_duration = atoi(ptr);
						}
						LOG(L_WARN,"HOLD_COUNT = %d HOLD_TIME = %d\n",hold_cnt,hold_duration);
					}
				}
			}
			if(hold_cnt > 0 && hm_buf){
				if(strstr(hm_buf,"crm=")){
					hst = "0000-00-00 00:00:00";
				}else{
					hst = time_s;
				}
				if(!wsuri.token ) {	
					snprintf (sqlcommand,sizeof(sqlcommand)-1 ,QUERY_HOLD_UPDATE, hold_cnt,hold_duration,hst,wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag, wsacc_call.totag, wsacc_call.fromtag, who);
				}else {
					snprintf (sqlcommand, sizeof(sqlcommand)-1 ,QUERY_HOLD_UPDATE_TOKEN, hold_cnt,hold_duration,hst,strlen (wsuri.token) > 0? atoi(wsuri.token): 0, nServiceid);
				}
			}
			break;
		case RACC_302_INVITE:
			get_grouptitle(&wscrd,2);
			if((parse_ws_fifo_uri_buffer(wsacc_call.uri, strlen(wsacc_call.uri), &wsuri) == 1)) {
			 	snprintf (sqlcommand, sizeof (sqlcommand),QUERY_TMPMOVED_DELETE_ACCACTIVE, wsacc_call.callid);
				LOG( L_ERR, "[racc_fifo_db_request][302 received from the far end / user ] RACC_302INVITE sqlcommand '%s'  \n" , sqlcommand );	
				if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
					LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
					return;
				}
				memset(token, 0, sizeof(token));
				if(wsuri.token && strlen (wsuri.token) > 10) {
					snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
					wsuri.token = token;
				}
				else {
					strncpy(token , wsuri.token , sizeof(token)-1);
				}
				memset(Ivr_Destination,0,sizeof(Ivr_Destination));
				if(wsuri.type  && strlen(wsuri.type)>0 && !strncmp(wsuri.type,"opr",3)){
					strncpy(Ivr_Destination,"01",sizeof(Ivr_Destination)-1);
				}
				if(wsuri.context && strlen(wsuri.context) > 0){
					memset(sitename,0,sizeof(sitename));
					strncpy(sitename,wsuri.context,sizeof(sitename)-1);
					while((ptr = strchr(sitename,'-')))
						*ptr = '_';
					GetSiteId(&wscrd1,sitename);
					memset(sqlcommand,0,sizeof(sqlcommand));
					snprintf (sqlcommand,sizeof(sqlcommand)-1,QUERY_302ACCOUNTNAME,sitename,wsacc_call.from,time_s,AAXFER,strlen (token) > 0 ? atoi(token) : 0,wsacc_call.callid,Ivr_Destination,wscrd.group_title,wscrd.groupcode,wsacc_call.callername,wscrd1.siteid,time_s,wsacc_call.from,wscrd.groupcode);				
                }
				LOG( L_ERR, "302_INVITE: %s\n\n", sqlcommand);
			}
			break;
		 default:
			/* Do Nothing */
			goto err;
			break;
	}
		
	if(strlen(sqlcommand) > 0){		
		if(wsacc_call.method == RACC_302_INVITE){
			nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
		}else{
			nRet = ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0);
		}
		if(nRet == -1)
		{
			LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
			return;
		}

		if(nEnableSleepPatch == 1 && wsacc_call.method == RACC_TERMINATING ) {
			for(i = 0; i < 2 ; i++) {
				nReturn =  mysql_affected_rows(db_acc_active);
				if(nReturn == 0 || nReturn == -1) {
				
					// Here above condition is matching means call hang up fields updation is failed.
					// Might be Answer and hangup came to MSICP at the same time and processing by it from different processes
					// To Fix the billing issue we are waiting for 1 second and re-firing the query again. 
					// Alog with these changes we are adding primary key with AUTO_INCREMENT in acc_active table to improve the query performence and to resolve the deadlock issue. -- these DB primary key changes are suggested by raghu.
					sleep(1);	
					LOG(L_WARN , "[wsfifo::racc_fifo_db_request] nReturn : '%d',sqlcommand : %s \n" ,nReturn, sqlcommand);
					nRet = ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0);
					if(nRet == -1) {
						LOG(L_WARN , "[wsfifo::racc_fifo_db_request]Failed query : '%s'\n" , sqlcommand);
						return;
					}
				}else{
					break;
				}
			}
		}
	}	
	
	switch (wsacc_call.method)	{
		case RACC_TERMINATING:
			/* Not inserting to CDRs if it is at a  time answer to avoid 1062 errors */
			if(strstr(wsacc_call.user_agent,user_agent)){
				racc_fifo_delete_records (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag);
				break;
			}
			if(wsuri.token && strlen (wsuri.token) > 10) {
				memset(token, 0, sizeof(token));
				snprintf(token, sizeof(token)-1, "0%s", &wsuri.token[strlen(wsuri.token)-9]);
				wsuri.token = token;
			}
			racc_fifo_write_final_data (wsacc_call.callid, wsacc_call.fromtag, wsacc_call.totag, wsacc_call.branch , (wsuri.token && strlen (wsuri.token) > 0 )? atoi(wsuri.token): 0, 0);
		break;
			
		default:
			break;
	}		
	return;

err:
	LOG(L_ERR, "[wsfifo::racc_fifo_db_request]ERROR (wsfifo): Huh? Method Undefined?? \n");
	return;
}

int ExecuteDbQuery(MYSQL **myDBhandler , char *cQuery , int nDbType , int nDBLogFlag)
{
	char cQueryBuf[4096] = "";
	int i=0;

	if(!myDBhandler){
		if(dbconnection(nDbType) == -1){
			LOG( L_INFO, "[wsfifo::ExecuteDbQuery]'trying reconnect both servers down'\n");
			return -1;
		}
	}

	memset(cQueryBuf , 0 , sizeof(cQueryBuf));
	strncpy(cQueryBuf , cQuery , sizeof(cQueryBuf)-1);

	if(mysql_query(*myDBhandler , cQueryBuf)){
		code = mysql_errno(*myDBhandler);
		LOG( L_INFO, "[wsfifo::ExecuteDbQuery] Mysql Error No:%d  QUERY : ' %s '\n",code,cQueryBuf);

		/* Retrying the query execution for retry errors --vravi */
		for(erno=0;erno<retrymaxnoerr;erno++){
			if((code == retryerrbuff[erno])){
				while(i < queryretrycount){
					if(mysql_query(*myDBhandler , cQueryBuf)){
						i++;
				 	}else{
						LOG( L_INFO, "[wsfifo::ExecuteDbQuery] Query succeeded after requery : %s \n",cQueryBuf);
						return 0;
				 	}
				}
				if(i == queryretrycount){
					LOG( L_INFO, "[wsfifo::ExecuteDbQuery] Query failed all the times in retry errors : Query : %s \n",cQueryBuf);
					return -1;
				}
			}
		}
	
		/* Getting new connection and retrying for reconnection errors --vravi */	
		for(erno=0;erno<maxnoerr;erno++){
			if((code == errbuff[erno])){
				if(dbconnection(nDbType) == -1){
					LOG( L_INFO, "[wsfifo::ExecuteDbQuery]'trying reconnect both servers down'\n");
					if(nDBLogFlag == 1)
					{
						strcat(cQueryBuf , ";");
						write( outfd, cQueryBuf , strlen(cQueryBuf) );
					}

					return -1;
				}
				mysql_query(*myDBhandler , cQueryBuf);
				break;
			}
		}
	}

	return 0;
}

/* Function for processing Missed call details from acc_active table --vravi/saidulu */

int missedcall_details(int childno)
{
	char sqlcommand[2048]="";
	int nRet,missed_call_status,n,wscalltype = 0,is_voicemail = 0,groupcode=0,is_groupcall=0 , temp_owner = 0 ;
	MYSQL_RES *res=NULL,*res1=NULL;
	MYSQL_ROW row;
	enum UriType utType;
	struct ws_uri wsuri,wsuri1;
	struct wscallratedetails wscrd;
	char cUtctime[128]="",c1Utctime[128]="";
    char f_field[256] = "", cTemp[256] = "", cTempOne[256] = "", cTempTwo[256] = "";	
	char callid[256]="",from[256]="",to[256]="",time_s[32]="",time1_s[32]="",request_uri[128]="",site[64]="",token [16]="", ring_time[128] = "",wsagentid[128]="",sip_to[256]="",calltoken[16]="",VmStart_Time[32]="",VmEnd_Time[32]="",cTransferno[32]="" , temp_wsagentid[128]="" , temp_to[256]="" , temp_ownerdid[32]="", rpid[256] = "";
	int missedcall_cause=1,j=0,pbxid = 0,siteid = 0,i=0,ring_duration = 0,owner=0,agent_id=0,call_type = 0 , acduser_missedcall_cause = 0, nCallType = 0;
	char *ptr = NULL,*s =NULL,cStime[32]="";
	char *acc_tblname = NULL;
	char start_time[32]="",end_time[32]="",grouptitle[64]="",callername[64]="",ownerdid[32]="",callerid[256]="";
	char DialedNumber[64]="",pickup_token[16]="";
	unsigned int CallToken=0;
	static int is_ws_grp=0, is_ivr_usercall=0;
	int nGrpCount =0, nFound = 0, nIvrHuntCount=0, from_ivr =0,another_calltype=0,dis_siteid=0,db_val=0,is_disaster=0,Phone_Type=0;
	char grp_callid[100][256];
	char ivr_usr_hunt_token[100][256],cAcrossUser[256]="",Across_from[256]="",cAcc_Sitename[256]="";
	int callflag_tax = 1, ispickupcall = 0, nImportedContactsID  = 0 ;;
	char cZoneCode[24] = "",originalcallerid[128]="",originalcnam[128]="",cSitename[256]="", cModifiedCallerId[256] = "";
	int is_direct_call = 0, is_transfer = 0, offnet_call = 0, nMoveToCdr = 1, nSpam = 0, nVerified = 0, nRejectCode = 0, iscalleriddisplay = 0;
	int rec_format = 0;

	/*connecting to database because it is separate process*/
	if(dbconnection(ACCACTIVE)==-1)
	{
		LOG(L_ERR,"[wsfifo::missedcall_details] Active transactions  Database connection is failed \n");
		return -1;
	}
	if(dbconnection(WARE) == -1)
	{
		LOG(L_ERR,"[wsfifo::missedcall_details] Warehouse transactions  Database connection is failed \n");
		return -1;
	}
	if(dbconnection(CONTACTSDB) == -1)
	{
		LOG(L_ERR,"[wsfifo::missedcall_details] CONTACTSDB transactions  Database connection is failed \n");
		return -1;
	}
	if(dbconnection(LIVE) == -1)
	{
		LOG(L_ERR,"[wsfifo::missedcall_details] LIVE  transactions  Database connection is failed \n");
		return -1;
	}
	if(connection_flag == 2){
		pthread_t t;
		pthread_attr_t attr;
 
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (pthread_create(&t, &attr, start_presence_connection_pinger, (void *)&pt_timer)){
			LOG(L_ERR, "[wsicpfiforeader::readfifo] Failed to create a thread\n");
			return -1;
		}
	}
	
	while(1)
	{
		/* Iteration duration */	
		sleep(missedcall_sleep);
		/* Getting missed call details from acc_active table */	
		memset(sqlcommand,0x00,sizeof(sqlcommand));        
        snprintf(sqlcommand,sizeof(sqlcommand),QUERY_MISSEDCALLS_SELECT,servertz,servertz,nServiceid,missedcall_querytime);
		nRet = ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0);

     	if(nRet == -1)
        {
			LOG(L_ERR , "[wsfifo::missedcall_details] SELECT query Failed : '%s'\n" , sqlcommand);
			continue;
        }
		res = mysql_store_result(db_acc_active);
		if((!res) || ((n=mysql_num_rows(res)) <= 0)){
			if(res){
			      mysql_free_result(res);
				  res=NULL;
			}
			continue;
		}

		j = 0;	
		memset(start_time,0x00,sizeof(start_time));
		memset(end_time,0x00,sizeof(end_time));
		memset(grp_callid, 0x00, sizeof(grp_callid));
		memset(ivr_usr_hunt_token, 0x00, sizeof(ivr_usr_hunt_token));
		nGrpCount=0;
		nIvrHuntCount=0;
		nFound=0;
		while ((row = mysql_fetch_row(res)) != NULL)
		{
			memset(VmStart_Time,0,sizeof(VmStart_Time));
			memset(VmEnd_Time,0,sizeof(VmEnd_Time));
			strncpy(VmStart_Time,INTIAL_TIME,sizeof(VmStart_Time)-1);
			strncpy(VmEnd_Time,INTIAL_TIME,sizeof(VmEnd_Time)-1);
			memset(cTransferno,0,sizeof(cTransferno));
			memset(temp_to,0,sizeof(temp_to));
			from_ivr=0;
			temp_owner = 0;
			nMoveToCdr = 1; /* By default we are moving all records into cdrs */
			offnet_call = 0;
			/* Resetting to default values */
			callflag_tax = 1;
			ispickupcall = 0;
			memset(cZoneCode,0,sizeof(cZoneCode));
			memset(originalcnam,0,sizeof(originalcnam));
			memset(originalcallerid,0,sizeof(originalcallerid));
            memset(cSitename,0,sizeof(cSitename));
			missedcall_cause = 1,pbxid = 0,siteid = 0,ring_duration = 0,is_voicemail = 0,groupcode = 0,owner = 0,agent_id = 0,is_direct_call = 0, is_transfer = 0;
			nSpam = 0, nVerified = 0, nRejectCode = 0;

			/*fetching start time and end time for deletion in case of acdext- and moc- */
			if(row[6])
			{
				memset(time1_s,0x00,sizeof(time1_s));
				strncpy(time1_s,row[6],sizeof(time1_s)-1);
				 if(j == 0){
					memset(start_time,0x00,sizeof(start_time));
					strncpy(start_time,row[6],sizeof(start_time)-1);
				 }
				 if(j == (n-1)){
					memset(end_time,0x00,sizeof(end_time));
					strncpy(end_time,row[6],sizeof(end_time)-1);
				 }
				 j++;
			}
			if(row[4])
			{	
				memset(sip_to,0x00,sizeof(sip_to));
				strncpy(sip_to,row[4],sizeof(sip_to)-1);
				/* Excluding missed call insertion for acd or MOC calls --vravi/saidulu */	
				if((strstr(sip_to,"acdext-")) || (strstr(sip_to,"moc-")))
				{
					continue;
				}
				if(strstr(sip_to,"cmd-")){
					from_ivr=1;
				}					
				memset(to,0x00,sizeof(to));
				strncpy(to,row[4],sizeof(to)-1);
			}
			if(row[7])
			{
				missedcall_cause=atoi(row[7]);
				/* Not inserting missedcall entry for those with missedcall_cause is 0 but deleting that entry */
				if(!missedcall_cause){
					continue;
				}
			}
			if(row[0])
			{
				memset(callid,0x00,sizeof(callid));
				strncpy(callid,row[0],sizeof(callid)-1);
			}
			if(row[1])
			{
				pbxid = atoi(row[1]);	
			}
			if(row[2])
			{
				memset(request_uri,0x00,sizeof(request_uri));
				memset(wsagentid,0x00,sizeof(wsagentid));
				strncpy(request_uri,row[2],sizeof(request_uri)-1);
				strncpy(wsagentid,row[2],sizeof(wsagentid)-1);
			}
			if(row[3])
			{
				memset(from,0x00,sizeof(from));
				strncpy(from,row[3],sizeof(from)-1);
				memset(Across_from,0,sizeof(Across_from));
				strncpy(Across_from,from,sizeof(Across_from)-1);

			}
				
			if(row[5])
			{
				memset(time_s,0x00,sizeof(time_s));
				strncpy(time_s,row[5],sizeof(time_s)-1);
			}
			
			if(row[8])
			{
				siteid=atoi(row[8]);
			}
			if(row[9]){
				memset(token,0x00,sizeof(token));
				strncpy(token, row[9],sizeof(token)-1);
				if(strstr(token,"wspib")){
					call_type = PSINB;
				}else{
					call_type = IPINB;
				}
			}
			if(row[10]){
				memset(ring_time,0x00,sizeof(ring_time));
				strncpy(ring_time,row[10],sizeof(ring_time)-1);
			}
			if(row[11]){
				ring_duration = atoi(row[11]);
			}
			if(row[12]){
				memset(calltoken,0x00,sizeof(calltoken));
				strncpy(calltoken,row[12],sizeof(calltoken)-1);
			}
			if(row[13]){
				is_voicemail = atoi(row[13]);
			}
			if(row[14]){
				memset(wsagentid,0x00,sizeof(wsagentid));
				strncpy(wsagentid,row[14],sizeof(wsagentid)-1);
			}
			if(row[15]){
				memset(grouptitle,0,sizeof(grouptitle));
				strncpy(grouptitle,row[15],sizeof(grouptitle)-1);
			}
			if(row[16]){
				memset(callername,0,sizeof(callername));
				strncpy(callername,row[16],sizeof(callername)-1);
				if(strstr(callername, "SPAM")) {
					nSpam = 1;
				}
				if(strstr(callername, "[V]")) {
					nVerified = 1;
				}
			}
			if(row[17]){
				groupcode=atoi(row[17]);
			}
			if(row[18]){/*Dialed Number insertion into CDR's*/
				memset(DialedNumber,0,sizeof(DialedNumber));
				strncpy(DialedNumber,row[18],sizeof(DialedNumber)-1);
			}
			if(row[19]){
				owner = atoi(row[19]);
			}
			if(row[20]){
				memset(ownerdid,0,sizeof(ownerdid));
				strncpy(ownerdid,row[20],sizeof(ownerdid)-1);
			}
			if(row[21]){
				agent_id = atoi(row[21]);
			}
			if(row[22]){
				memset(pickup_token,0,sizeof(pickup_token));
				strncpy(pickup_token,row[22],sizeof(pickup_token)-1);
			}
			if(row[23]){
				memset(cUtctime,0,sizeof(cUtctime));
				strncpy(cUtctime,row[23],sizeof(cUtctime)-1);
			}
			if(row[24]){
				memset(c1Utctime,0,sizeof(c1Utctime));
				strncpy(c1Utctime,row[24],sizeof(c1Utctime)-1);
			}
			if(row[25]){
				memset(cAcrossUser,0,sizeof(cAcrossUser));
				strncpy(cAcrossUser,row[25],sizeof(cAcrossUser)-1);
				if((ptr = strchr(cAcrossUser,'#')) && ++ptr && strlen(ptr)>0){
					strncpy(cTransferno,ptr,sizeof(cTransferno)-1);
				}
			}
 			if(row[27]){
 				is_direct_call = atoi(row[27]);
 			}
			if(row[28]){
				is_transfer = atoi(row[28]);
			}
            if(row[29]){
                nCallType = atoi(row[29]);
            }
			memset(cModifiedCallerId, 0, sizeof(cModifiedCallerId));
			if(row[30]) {
				memset(rpid, 0x0, sizeof(rpid));
                		strncpy(rpid, row[30], sizeof(rpid) - 1);
				if((ptr = strstr(row[30],"sip:")) && (ptr = ptr+4)) {
					strncpy(cModifiedCallerId, ptr , sizeof(cModifiedCallerId)-1);
					if(ptr = strchr(cModifiedCallerId, '@')) 
						*ptr = '\0';
					}
            		}

			if(row[31] && strlen(row[31])>0)
			{
				strncpy(originalcallerid,row[31],sizeof(originalcallerid)-1);
			}
			if(row[32] && strlen(row[32])>0)
			{
				strncpy(originalcnam,row[32],sizeof(originalcnam)-1);
			}
			if(row[33]){
				ispickupcall = atoi(row[33]);
			}
            if(row[34] && strlen(row[34])>0 ){ 
               strncpy(cSitename,row[34],sizeof(cSitename)-1);
            }
	    if(row[35] && strlen(row[35])>0) {
		    nRejectCode = atoi(row[35]);
	    }
	    if(row[36]){
		iscalleriddisplay = atoi(row[36]);
	    }
	    if(row[37]) {
		    rec_format = atoi(row[37]);
	    }
			if(missedcall_cause != F_CANCEL && missedcall_cause != F_ROUTINGRINGTIMEOUT && missedcall_cause != F_CANCELFORWARD && missedcall_cause != F_CALL_COMPLETED_ELSEWHERE && is_voicemail && row[26] && strlen(row[26])>0){
				memset(time_s,0x00,sizeof(time_s));
				strncpy(time_s,time1_s,sizeof(time_s)-1);
				strncpy(VmStart_Time,time1_s,sizeof(VmStart_Time)-1);
				memset(time1_s,0,sizeof(time1_s));
				strncpy(time1_s,row[26],sizeof(time1_s)-1);
				strncpy(VmEnd_Time,time1_s,sizeof(VmEnd_Time)-1);
			}

			is_ws_grp=0;
			is_ivr_usercall=0;
			another_calltype=0;
			memset(&wsuri1, 0, sizeof(struct ws_uri));
			if(parse_ws_fifo_uri_buffer(sip_to, strlen(sip_to), &wsuri1)== -1){
				LOG(L_WARN,"[wsfifo::missedcall_details] Parse URI Failed: %s\n", sip_to);
                continue;
			}
			if( wsuri1.group && (wsuri1.type && !strncmp(wsuri1.type,"cmd",3)) || (((!strncmp(wsuri1.type,"gext",4))) && wsuri1.command && !strstr(wsuri1.command,DialedNumber)) || (wsuri1.type && !strncmp(wsuri1.type,"ext",3)))
			{

				if(!strncmp(wsuri1.type,"ext",3))			 /* IVR to user to HUNT group ---Selvan */
				{			
					for(i = 0; i < nIvrHuntCount; i++) {	/* Missed call cases */
						if(strlen(ivr_usr_hunt_token[i]) && !strncmp(ivr_usr_hunt_token[i],wsuri1.token,sizeof(ivr_usr_hunt_token[i]))){
							is_ws_grp=1;						
							break;	
						}								
					}
					if(from_ivr && is_voicemail){ 	/* Voicemail cases */
						is_ws_grp=1;
						memset(ivr_usr_hunt_token, 0x00, sizeof(ivr_usr_hunt_token));
						strncpy(ivr_usr_hunt_token[nIvrHuntCount],wsuri1.token,strlen(wsuri1.token));
						nIvrHuntCount++;
					}						
				}					
				else
				{
					if(groupcode && groupcode!=atoi(wsuri1.group))     /* IVR to group call ---Selvan */
					{
						is_ws_grp	= 1;
					}
					else{
						is_ivr_usercall = 1;
						memset(ivr_usr_hunt_token, 0x00, sizeof(ivr_usr_hunt_token));
						strncpy(ivr_usr_hunt_token[nIvrHuntCount],wsuri1.token,strlen(wsuri1.token));
						nIvrHuntCount++;
					}
				}					
			}  
			/* Starting with 1st ivr+hunt grp user's grpcallid and also for groupcall voicemail */
			if(is_ws_grp || groupcode){
				for(i = 0; i < nGrpCount; i++) {
					if(strlen(callid) && !strncmp(grp_callid[i],callid,strlen(callid))){	
						is_ws_grp =0;
						nFound = 1;
					}
				}
				if(nFound ==0){
					strncpy(grp_callid[nGrpCount],callid,sizeof(grp_callid[nGrpCount])-1);
					nGrpCount++;
					is_ws_grp =1;/*Setting this value if call routed to group voicemail*/
				}
			}					

			nFound=0;			
			is_groupcall=0;
			is_disaster = 0;
			Phone_Type = 0;
			db_val = 0;
			dis_siteid = 0;

			memset(sqlcommand,0x00,sizeof(sqlcommand));
			if(is_voicemail){
/*				is_groupcall=1; //We are inserting voicemail detail in cdr
				snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_UPDATE_ACCACTIVE,F_DEFAULT,missedcall_cause,atoi(calltoken),nServiceid);
				if(ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0)){
					LOG(L_WARN , "[wsfifo::missedcall_details]Failed qry : '%s'\n" , sqlcommand);
				}*/
				another_calltype=VOICEMAIL_IP;
			}
			if(((missedcall_cause==F_CANCEL||missedcall_cause==F_RINGTIMEOUT||missedcall_cause== F_REJECT||missedcall_cause==F_OTHER || missedcall_cause == F_PICKUP || missedcall_cause == F_ROUTINGRINGTIMEOUT || missedcall_cause == F_CANCELFORWARD || missedcall_cause == F_CALL_COMPLETED_ELSEWHERE) && groupcode && (strlen(grouptitle) > 0))){
				if(is_ws_grp){
					is_groupcall=0;
				}else if(is_ivr_usercall){
					is_groupcall=0;
				}else{
					is_groupcall=1;
				}
			}

			if(is_ws_grp){
				if (!strncmp(to, "acdxfer-", 8)){
					temp_owner = owner;
					memset(temp_ownerdid,0,sizeof(temp_ownerdid));
					strncpy(temp_ownerdid,ownerdid,sizeof(temp_ownerdid)-1);
					memset(temp_wsagentid , 0 , sizeof(temp_wsagentid));
					strncpy(temp_wsagentid , wsagentid , sizeof(temp_wsagentid)-1);
					strncpy(temp_to , to , sizeof(temp_to)-1);
					memset(to , 0 , sizeof(to));
					strncpy(to , grouptitle , sizeof(to)-1);
				}

				wscrd.siteid=siteid;
				get_group_ext_did(groupcode,grouptitle,&wscrd);
				if(!is_ivr_usercall)
					owner=wscrd.ownerext;
				memset(ownerdid,0x00,sizeof(ownerdid));
				if(strlen(wscrd.ownerdid)<0){
					snprintf(ownerdid,sizeof(ownerdid)-1,"%d",owner);
				}							
				else{
					strncpy(ownerdid,wscrd.ownerdid,sizeof(ownerdid)-1);
				}						
			}	

			if(missedcall_cause != F_NOREGSITER){
				memset(cXMLBuf , 0 , sizeof(cXMLBuf));
				snprintf(cXMLBuf , sizeof(cXMLBuf) , XML_CALLDETAILS , cUtctime , c1Utctime , from , wsagentid , callid , callername, iscalleriddisplay ? 1: 0, "\0");
				LOG(L_ERR , "[wsfifo::missedcall_details] : %s\n" , cXMLBuf);
				send_to_server(cXMLBuf , strlen(cXMLBuf) + 1);
			}

			if(!is_groupcall || is_ws_grp){
				utType = get_fifo_uri_type(request_uri,sizeof(request_uri), &wsuri);
				/*Added changes to insert roting plan or disaster changes*/
				if(utType == OMS_Request){
					if(wsuri.group && (strncmp(wsuri.group,"transfer",8) && !isdigit(wsuri.group[0]))){
						is_disaster = 1;
						memset(to,0,sizeof(to));
						strncpy(to,wsuri.command,sizeof(to)-1);
						memset(from,0,sizeof(from));
						snprintf(from,sizeof(from)-1,"%s-%s",wsuri.group,wsuri.context);
						db_val = check_international_npa(wsuri.command,&dis_siteid);
						if(db_val == 1){
							wscalltype  = IP_FORWARD;
						}else{
							wscalltype = PSTN_FORWARD;
						}
					}
				}
				/*End of roting plan or disaster changes*/

				/* Getting site from WSURI if exists */
				memset(site,0,sizeof(site));
				if (wsuri.context!=NULL && utType != Local_User) {
					strncpy(site,wsuri.context,sizeof(site)-1);
					GET_SITENAME;
				}
				/* Checking for sitename existence once again */
				if(site[0] == 0){
					if(utType == Local_User){
						s = strstr (request_uri, "-");
					}else{
						s = strstr (from, "-");
					}
					if (NULL != s) {
						s++;
						if(s){
							memset(site,0,sizeof(site));
							strncpy(site, s,sizeof(site)-1);
						}
					}
					GET_SITENAME;
				}
                if ( missedcall_cause == 15 ){
                    if(strlen(cSitename) > 0){
                        memset(site,0,sizeof(site));
                        strncpy(site,cSitename,sizeof(site)-1 ) ;  
                    }else if (strlen(site) > 0){
                        memset(cSitename,0,sizeof(cSitename));
                        strncpy(cSitename,site,sizeof(cSitename)-1) ;
                    }
                }
				if(strlen(site) > 0){
                        		strncpy(cSitename, site, sizeof(cSitename)-1 ) ;
                    ws_set_to_lowercase(site);
					acc_tblname = site;
				}else{
					LOG(L_INFO, "[wsfifo::missedcall_details]sitename is not found : UTYPE: %d  Request uri : %s \n", utType,request_uri);
					continue;
				}
				memset(&wscrd, 0x00,sizeof(wscrd));
				if( (strlen(DialedNumber) > 6) && (isdigit(DialedNumber[0]) || DialedNumber[0] == '+') && strncmp(DialedNumber,"011",3)){
					wscrd.siteid = siteid;
					GetPoolDetails(&wscrd,DialedNumber,request_uri,sizeof(DialedNumber));
				}
				if(!is_disaster || !wscalltype){
					wscalltype = IP_FORWARD;
				}
				/*For setting PSIB or FWDIP based on call source */
				if(strstr(token,"wspib")){
					memset(token,0x00,sizeof(token));
					strncpy(token,calltoken,sizeof(token)-1);
				}
				if(strlen(token) >0){
					CallToken = atoi(token);
				}
				if(is_ws_grp && (!is_ivr_usercall)){
					memset(wsagentid,0,sizeof(wsagentid));
					if(to && (!strncmp(to,"pst-",4)) && (wsuri1.command != NULL) ){
						snprintf(wsagentid,sizeof(wsagentid)-1,"%s",wsuri1.command);
					}else{	
						snprintf(wsagentid,sizeof(wsagentid)-1,"%s",grouptitle);
					}
					owner=0;
					memset(ownerdid,0,sizeof(ownerdid));
				}
				if (strlen(temp_to) > 0 && !strncmp(temp_to, "acdxfer-", 8))
				{
					wscalltype=NONACD_ABONDONED_TRANSFER_OUT;	
					another_calltype = 0;
					acduser_missedcall_cause = missedcall_cause;
					missedcall_cause=0;
				}		
				/*Inserting to missedcall details table */
				if(singlecdrtable == 1) {
					memset(acc_tblname,0,sizeof(acc_tblname));
					strncpy(acc_tblname,"wscdrs",sizeof(acc_tblname)-1);
				}
				GetSiteId(&wscrd,acc_tblname);

				if(!strcmp(VmEnd_Time,INTIAL_TIME) || !strcmp(VmStart_Time,INTIAL_TIME)){/*if any time is zero hv to copy below elemnts to zro,otherwise entries are not inserting into CDR's*/
					strncpy(VmStart_Time,INTIAL_TIME,sizeof(VmStart_Time)-1);
					strncpy(VmEnd_Time,INTIAL_TIME,sizeof(VmEnd_Time)-1);
				}
				GetCallerid(from,callername,callerid,256);
					
				memset(cAcc_Sitename,0,sizeof(cAcc_Sitename));
				strncpy(cAcc_Sitename,acc_tblname,sizeof(cAcc_Sitename)-1);
				while((ptr = strchr(cAcc_Sitename,'_')))
					*ptr = '-';
				if(strlen(cAcrossUser) >0 && !strstr(cAcrossUser,cAcc_Sitename) && !strstr(Across_from,cAcc_Sitename)){
					memset(cStime,0,sizeof(cStime));
					strncpy(cStime,start_time,sizeof(cStime)-1);
					if((ptr = strchr(cStime,' '))){
						*ptr = '\0';
						if(strlen(cStime)>0){
							snprintf(sqlcommand, sizeof (sqlcommand)-1,SELECT_INTERACCOUNT_DETAIL,acc_tblname,cStime,CallToken);		
							nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
							if(nRet == -1)
							{
								LOG(L_WARN , "[wsfifo::missedcall_details]Failed qry : '%s'\n" , sqlcommand);
								if(res){
									mysql_free_result(res);
									res=NULL;
								}
								return;
							}
							res1 = mysql_store_result(db_wh_handle);
							if(!res1 || (mysql_num_rows(res1) <= 0)){
								memset(sqlcommand,0,sizeof(sqlcommand));
                                if(acduser_missedcall_cause == 15 || missedcall_cause == 15 ){
					if(iscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
						nImportedContactsID = 0;
					} else {
					    get_importedContact_details(cSitename, wscrd.siteid, Across_from, strlen(cTransferno)>0 ? cTransferno : DialedNumber, &nImportedContactsID); 
					}
                                 snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_CDRS,cSitename,callid,pbxid,request_uri,Across_from,strlen(cTransferno)>0 ? cTransferno : DialedNumber,time_s,time1_s,0,wscrd.siteid,nServiceid,CallToken,ring_time,PSTN_INBOUND,wsagentid,ring_duration,grouptitle,callername,groupcode,owner,ownerdid,agent_id,DialedNumber,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,time1_s,servertz,wscrd.tdiff,strlen(pickup_token) >0 ? atoi(pickup_token) : 0,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,VmEnd_Time,VmStart_Time,VmEnd_Time,VmStart_Time,0,Phone_Type,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, nSpam, nVerified, nRejectCode, nImportedContactsID, rec_format); 
                                }else{
								if(iscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
									nImportedContactsID = 0;
								} else {
								    get_importedContact_details(acc_tblname, wscrd.siteid, Across_from, strlen(cTransferno)>0 ? cTransferno : DialedNumber, &nImportedContactsID); 
								}
								snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_CDRS,acc_tblname,callid,pbxid,request_uri,Across_from,strlen(cTransferno)>0 ? cTransferno : DialedNumber,time_s,time1_s,0,wscrd.siteid,nServiceid,CallToken,ring_time,PSTN_INBOUND,wsagentid,ring_duration,grouptitle,callername,groupcode,owner,ownerdid,agent_id,DialedNumber,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,time1_s,servertz,wscrd.tdiff,strlen(pickup_token) >0 ? atoi(pickup_token) : 0,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,VmEnd_Time,VmStart_Time,VmEnd_Time,VmStart_Time,0,Phone_Type,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, nSpam, nVerified, nRejectCode, nImportedContactsID, rec_format);
                                }
								LOG(L_ERR,"[wsfifo::missedcall_details]It's an Across account call,so we inserting one more leg into CDR '%s'\n",sqlcommand);
		
								nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
								if(nRet == -1)
								{
									LOG(L_WARN , "[wsfifo::missedcall_details]Failed qry : '%s'\n" , sqlcommand);
									if(res){
										mysql_free_result(res);
										res=NULL;
									}
									return;
								}
							}
							if(res1)
							{
								mysql_free_result(res1);
							}
						}
					}
				}
				memset(sqlcommand,0,sizeof(sqlcommand));
				if(is_disaster){
					if(wscalltype == PSTN_FORWARD) { /*Added for Pstn forward -karan*/
						Phone_Type = PSTN_CALL;
					}
					if(wscalltype == IP_FORWARD){
						Phone_Type = DISASTER_FORWARD;
						missedcall_cause = 0;
						if(wscrd.siteid != dis_siteid){
							wscrd.calltype = PSTN_FORWARD;
						}
					}else if(is_voicemail){
						wscalltype = IP_FORWARD;
					}
				}
				if(missedcall_cause == F_PICKUP){/*Added changes update missedcall_cause as zero for *53,*54 pickup scenarios*/
					missedcall_cause =0;
					memset(to,0,sizeof(to));
					strncpy(to,wsagentid,sizeof(to)-1);
				}
                /*Added "xfer" check for routing plan ext voicemail case.*/
				if((!strncmp(request_uri,"oms-",4) && !strncmp(to,"gext-",5)) || ((((wsuri.type != NULL) && !strncmp(wsuri.type,"xfer", 4)) 
                                                                              && (!strncmp(to,"ext-", 4) || !strncmp(to,"iext-", 5))))) {
					memset(to,0,sizeof(to));
					strncpy(to,wsuri.command,sizeof(to)-1);
				}
                
				/*Added changes update missedcall_cause as zero for pstn to AA group and dtmf as hunt group */
                if(!strncmp(request_uri,"oms-",4) && !strncmp(to,"gext-",5)) {
					missedcall_cause =0;
                }

                if(nCallType == nRoutingPlanEXT_CallType) {
                    memset(&f_field, 0x0, sizeof(f_field));
                    strncpy(f_field, from, sizeof(f_field) - 1);
                    memset(cTemp, 0x0, sizeof(cTemp));
                    memset(cTempOne, 0x0, sizeof(cTempOne));
                    memset(cTempTwo, 0x0, sizeof(cTempTwo));
                    getProperAgent(from, cTemp, sizeof(cTemp));
                    getProperAgent(to, cTempOne, sizeof(cTempOne));
                    getProperAgent(wsagentid, cTempTwo, sizeof(cTempTwo));
                    if((strcmp(cTemp, cTempOne) == 0) || (strcmp(cTemp, cTempTwo) == 0)) {
                        memset(&f_field, 0x0, sizeof(f_field));
                        strncpy(f_field, rpid, sizeof(f_field) - 1);
                    }
                }

				SetCallTaxValuesForCallTypes(wscalltype, to, from, wscrd.siteid, cZoneCode, sizeof(cZoneCode), &callflag_tax);

				LOG(L_INFO , "is_direct_call = %d, is_transfer = %d,%d,%s \n",is_direct_call, is_transfer, missedcall_cause,callid);
				/* Inserting into accounting table */
				if(is_direct_call) {
					(is_transfer == 1) ? (offnet_call = 1) : (nMoveToCdr = 0);
				}
				memset(sqlcommand, 0, sizeof(sqlcommand));
                if(missedcall_cause == 300){
                    memset(time1_s,0,sizeof(time1_s));
                    snprintf(time1_s,sizeof(time1_s)-1,"%s",time_s);
                }

		/*Added below changes to update proper missedcall_cause and another_calltype in acc_sitename table to fix HUNT group report issue fix --Prashant/Abhilash*/
		if(missedcall_cause == F_CANCELFORWARD) {
			another_calltype = 0;
			missedcall_cause = 0;
			memset(to, 0, sizeof(to));
			strncpy(to, request_uri, sizeof(to)-1);
		}else if(missedcall_cause == F_ROUTINGRINGTIMEOUT || missedcall_cause == F_CALL_COMPLETED_ELSEWHERE) {
			another_calltype = nCancelAnotherCallType;
			if(missedcall_cause == F_ROUTINGRINGTIMEOUT) {
				missedcall_cause = F_RINGTIMEOUT;
			}
		}	

		if(nRejectCode == 603 && missedcall_cause == 21) {
			if((ptr = strstr(request_uri,"oms-")) && ptr+4) {
				memset(to, 0, sizeof(to));
				strncpy(to, ptr+4, sizeof(to)-1);
				if((ptr = strchr(to, '-'))) {
					*ptr = '\0';
				}
				wscalltype = PSTN_FORWARD;			
			}
		}
                if(missedcall_cause == 15){
		if(iscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
			nImportedContactsID = 0;
		} else {
		    get_importedContact_details(cSitename, wscrd.siteid, (nCallType == nRoutingPlanEXT_CallType) ? f_field : from, wsagentid, &nImportedContactsID); 
		}
                snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_CDRS,cSitename,callid,pbxid,request_uri,(nCallType == nRoutingPlanEXT_CallType) ? f_field : from,to,time_s,time1_s,missedcall_cause,wscrd.siteid,nServiceid,CallToken,ring_time,PSTN_INBOUND,wsagentid,ring_duration,grouptitle,callername,groupcode,owner,ownerdid,agent_id,DialedNumber,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,time1_s,servertz,wscrd.tdiff,strlen(pickup_token) >0 ? atoi(pickup_token) : 0,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,VmEnd_Time,VmStart_Time,VmEnd_Time,VmStart_Time,another_calltype!=VOICEMAIL_IP?another_calltype:0,Phone_Type,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, nSpam, nVerified, nRejectCode, nImportedContactsID, rec_format);
                }else if(nMoveToCdr == 1 ) {
					if(iscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
						nImportedContactsID = 0;
					} else {
					    get_importedContact_details(acc_tblname, wscrd.siteid, (nCallType == nRoutingPlanEXT_CallType) ? f_field : from, wsagentid, &nImportedContactsID); 
					}
					snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_CDRS,acc_tblname,callid,pbxid,request_uri,(nCallType == nRoutingPlanEXT_CallType) ? f_field : from,to,time_s,time1_s,missedcall_cause == 300?15:missedcall_cause,wscrd.siteid,nServiceid,CallToken,ring_time,wscalltype,wsagentid,ring_duration,grouptitle,callername,groupcode,owner,ownerdid,agent_id,DialedNumber,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,time1_s,servertz,wscrd.tdiff,strlen(pickup_token) >0 ? atoi(pickup_token) : 0,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,VmEnd_Time,VmStart_Time,VmEnd_Time,VmStart_Time,missedcall_cause == 300?VOICEMAIL_IP:another_calltype,Phone_Type,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, nSpam, nVerified, nRejectCode, nImportedContactsID, rec_format);
				}

				if(strlen(sqlcommand)) {
					nRet = ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1);
					if(nRet == -1){
						LOG(L_WARN , "[wsfifo::missedcall_details]Failed qry : '%s'\n" , sqlcommand);
					}
				}
				/* Added changes for inserting extra leg into CDR's ACD known extention transfer --vravi/Saidulu*/
				if (strlen(temp_to) > 0 && !strncmp(temp_to, "acdxfer-", 8)){
					another_calltype=VOICEMAIL_IP;
					wscalltype = IP_FORWARD;
					memset(cZoneCode,0,sizeof(cZoneCode));
					callflag_tax = 1;
					offnet_call = 0;
					memset(sqlcommand,0,sizeof(sqlcommand));
                    if(acduser_missedcall_cause == 15 || missedcall_cause ==15 ){
				if(iscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
					nImportedContactsID = 0;
				} else {
				    get_importedContact_details(cSitename, wscrd.siteid, from, temp_wsagentid,  &nImportedContactsID); 
				}
                     snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_CDRS,cSitename,callid,pbxid,request_uri,from,temp_to,time_s,time1_s,acduser_missedcall_cause,wscrd.siteid,nServiceid,CallToken,ring_time,PSTN_INBOUND,temp_wsagentid,ring_duration,"",callername,0,temp_owner,temp_ownerdid,agent_id,DialedNumber,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,time1_s,servertz,wscrd.tdiff,strlen(pickup_token) >0 ? atoi(pickup_token) : 0,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,VmEnd_Time,VmStart_Time,VmEnd_Time,VmStart_Time,another_calltype,Phone_Type,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, nSpam, nVerified, nRejectCode, nImportedContactsID, rec_format);  
                    }else {
					if(iscalleriddisplay == 1) {
		/* Added code changes to set 'imported_contactid' 0 in acc_sitename table in case of AA/ACD group set 'CNAM' to fix the bug 'PTICNDNAWO-103' in import contact project --Prashant/Surekha -Jun-2021 */	
						nImportedContactsID = 0;
					} else {
					    get_importedContact_details(acc_tblname, wscrd.siteid, from, temp_wsagentid,  &nImportedContactsID); 
					}
					snprintf(sqlcommand,sizeof (sqlcommand),QUERY_MISSEDCALLS_CDRS,acc_tblname,callid,pbxid,request_uri,from,temp_to,time_s,time1_s,acduser_missedcall_cause,wscrd.siteid,nServiceid,CallToken,ring_time,wscalltype,temp_wsagentid,ring_duration,"",callername,0,temp_owner,temp_ownerdid,agent_id,DialedNumber,call_type,wscrd.orgname,wscrd.poolname,wscrd.aliasdid,time1_s,servertz,wscrd.tdiff,strlen(pickup_token) >0 ? atoi(pickup_token) : 0,call_type,IN_BOUND,strlen(cModifiedCallerId)>0?cModifiedCallerId:callerid,VmEnd_Time,VmStart_Time,VmEnd_Time,VmStart_Time,another_calltype,Phone_Type,cZoneCode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, nSpam, nVerified, nRejectCode, nImportedContactsID, rec_format);
                    }
					LOG(L_WARN , "[wsfifo::missedcall_details] ACCOUNTING EXTRA INSERT  : '%s' (acdxfer-) \n" , sqlcommand);
					if(ExecuteDbQuery(&db_wh_handle , sqlcommand , WARE , 1)){
							LOG(L_WARN , "[wsfifo::missedcall_details]Failed qry : '%s'\n" , sqlcommand);
					}
				}

				if(Reporting_flag && CallToken){
					nRet = InsertReductionDetailsToDB(CallToken,(missedcall_cause ==15)|| (acduser_missedcall_cause ==15) ? cSitename:acc_tblname,time_s,siteid,nDataCenterID);
					if(nRet == -1)
					{
						LOG(L_WARN , "[wsfifo::missedcall_details]Failed query : '%s'\n" , sqlcommand);
						if(res){
							mysql_free_result(res);
							res=NULL;
						}
						return;
					}	
				}		
			}
		}
		if(res){
			mysql_free_result(res);
			res=NULL;
		}
		/* Deleting the missed call processed entries from acc_active table */
		if(strlen(start_time) > 0 && strlen(end_time) > 0){	
			memset(sqlcommand,0x00,sizeof(sqlcommand));	
			snprintf (sqlcommand, sizeof (sqlcommand),QUERY_MISSEDCALLS_DELETE_ACCACTIVE, start_time,end_time,nServiceid);
			if(ExecuteDbQuery (&db_acc_active , sqlcommand , ACCACTIVE , 0)){
					LOG(L_WARN,"[wsfifo::missedcall_details] DELETE query Failed :'%s'\n",sqlcommand);
			}
		}
	}	
	
	if(connection_flag > 0)
		close(server_sockfd);
	mysql_close(db_acc_active);
	mysql_close(db_wh_handle);
	mysql_close(db_contact_handle);
	mysql_close(db_live_handle);

	return 0;
}


int get_group_ext_did(int code, char *title, struct wscallratedetails *wscrd)
{
	int nRet;
	MYSQL_ROW row;
	MYSQL_RES *res=NULL;
	char sqlcommand[1024]="";

	if(!code) {
		LOG(L_ERR,"[get_group_ext_did]Above values is NULL,so return here\n");
		return 0;
	}
	memset(sqlcommand,0,sizeof(sqlcommand));
    if(title != NULL && strlen(title) > 0) {
	    snprintf(sqlcommand,sizeof(sqlcommand),QUERY_GET_GRPEXT_DID,code,title,wscrd->siteid);
    } else {
        snprintf(sqlcommand,sizeof(sqlcommand),QUERY_GET_GRPEXT_DID_ONE,code,wscrd->siteid);
    }
	nRet = ExecuteDbQuery(&db_acc_active , sqlcommand , ACCACTIVE , 0);
	if(nRet == -1){
		LOG(L_INFO,"[get_group_ext_did] Failed qry : '%s'\n",sqlcommand);
		return 0;
	}
	res = mysql_store_result(db_acc_active);
	if (!res || mysql_num_rows(res) <=0) {
		if(res) {
			mysql_free_result(res);
			res = NULL;
		}
		LOG(L_INFO,"[get_group_ext_did]Result is not found : '%s'\n",sqlcommand);
		return 0;
	}
	if(row = mysql_fetch_row(res))	{				
		if(row[0] && strlen(row[0]) > 0) {
			wscrd->ownerext=atoi(row[0]);		
		}
		if(row[1] && strlen(row[1]) > 0) {
			memset(wscrd->ownerdid, 0x00, sizeof(wscrd->ownerdid));
			strncpy(wscrd->ownerdid, row[1],sizeof(wscrd->ownerdid)-1);
		}		
        if((!title || strlen(title) <= 0) && row[2] && strlen(row[2]) > 0) {
            memset(wscrd->group_title, 0x00, sizeof(wscrd->group_title));
            strncpy(wscrd->group_title, row[2],sizeof(wscrd->group_title)-1);
        }					
	}
	if(res) {
		mysql_free_result(res);
		res = NULL;
	}
	return 1;
}	

int get_agentid(char *agentid)
{
	char *cPtr = NULL,cTmp[256]="";
    int nPhonetype = 0;

	if(!(cPtr = strchr(agentid , '@'))) {
	    if((cPtr = strchr(agentid,'-')))
    	{
        	*cPtr = '@';
	        strncpy(cTmp ,cPtr,sizeof(cTmp)-1);
    	    if(cPtr && isdigit(cPtr[-2])) {
        	    cPtr = cPtr -2;
	        }
    	    else if(cPtr && isdigit(cPtr[-1])) {
        	    cPtr--;
	        }

    	    if(cPtr)
        	    nPhonetype = atoi(cPtr);

	        if(nPhonetype > 0 && nPhonetype < 32) {
    	        *cPtr = '\0';
        	    strcat(agentid,cTmp);
	        }
		}
	}
	while((cPtr = strchr(agentid,'-'))){
		*cPtr = '.';
	}
    return 0;
}

int get_operator_plan(struct ws_uri *wsuri_to,char *cGcode,int nLen){
	char sqlcommand[1024]= "";
	int nRet=0;
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;

	if(!wsuri_to || !cGcode){
		LOG(L_ERR,"[get_operator_plan]wsuri_to or wscrd is null,so returning...\n");
		return 0;
	}
	memset(sqlcommand,0, sizeof (sqlcommand));
	snprintf(sqlcommand , sizeof(sqlcommand)-1 ,IVR_OPR_QUERY ,wsuri_to->group,wsuri_to->context);
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1){
		LOG(L_INFO,"[get_operator_plan]Failed qry : '%s'\n",sqlcommand);
		return 0;
	}
	res = mysql_store_result(db_live_handle);
	if (!res || mysql_num_rows(res) <=0) {
		if(res) {
			mysql_free_result(res);
			res = NULL;
		}
		LOG(L_INFO,"[get_operator_plan]Result is not found : '%s'\n",sqlcommand);
		return 0;
	}
	if(row = mysql_fetch_row(res))
	{
		if(row[0] && strlen(row[0]) > 0) {
			memset(cGcode,0,nLen);
			strncpy(cGcode,row[0],nLen-1);
		}
	}
	
	if(res) {
		mysql_free_result(res);
		res = NULL;
	}
	return 1;
}

int get_cmd_plan(struct ws_uri *wsuri_to,struct wscallratedetails *wscrd,int *nFlag){
	char sqlcommand[1024]= "";
	int nRet=0;
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;

	if(!wsuri_to || !wscrd || !nFlag){
		LOG(L_ERR,"[get_cmd_plan]wsuri_to or cGcode is null,so returning...\n");
		return 0;
	}
	memset(sqlcommand,0, sizeof (sqlcommand));
	snprintf(sqlcommand , sizeof(sqlcommand)-1 ,IVR_CMD_QUERY ,wsuri_to->context,wsuri_to->group,wsuri_to->command,wsuri_to->context,wsuri_to->group,wsuri_to->command,wsuri_to->context,wsuri_to->group,wsuri_to->command);
	nRet = ExecuteDbQuery(&db_live_handle , sqlcommand , LIVE , 0);
	if(nRet == -1){
		LOG(L_INFO,"[get_cmd_plan]Failed qry : '%s'\n",sqlcommand);
		return 0;
	}
	res = mysql_store_result(db_live_handle);
	if (!res || mysql_num_rows(res) <=0) {
		if(res) {
			mysql_free_result(res);
			res = NULL;
		}
		if(wsuri_to->command && atoi(wsuri_to->command) == 0){
			LOG(L_ERR,"[get_cmd_plan] 0th detsination details are not found,so call is routed to operator\n");
			*nFlag=2;
			return 1;
		}
		LOG(L_ERR,"[get_cmd_plan] Result is empty,so return here\n");
		return 0;
	}
	if(row = mysql_fetch_row(res))
	{
		if(row[0] && strlen(row[0])>0) {
			memset(wscrd->group_title,0,sizeof(wscrd->group_title)-1);
			strncpy(wscrd->group_title,row[0],sizeof(wscrd->group_title)-1);
		}
		if(row[1] && strlen(row[1])>0){
			wscrd->groupcode=atoi(row[1]);
		}
		if(row[2]){
			*nFlag=atoi(row[2]);
		}
	}
	if(res) {
		mysql_free_result(res);
		res = NULL;
	}
	return 1;
}

int get_serviceid_pstn_calls(struct wscallratedetails *wscrd){

	char sqlcommand[512]="";
	int nRet=-1;
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;

	if(!wscrd){
		LOG(L_ERR,"[wsicpfifo::get_serviceid_pstn_calls] wscrd is NULL",wscrd);
		return -1;
	}

	memset(sqlcommand,0,sizeof(sqlcommand));
	snprintf(sqlcommand,sizeof(sqlcommand),SERVICEID_QUERY,wscrd->agentid,wscrd->siteid);
	
	nRet=ExecuteDbQuery(&db_live_handle, sqlcommand, LIVE, 0);
	if(nRet == -1){
		LOG(L_ERR , "[get_serviceid_pstn_calls] Serviceid Query execution failed : < %s >\n",sqlcommand);
		return -1;
	}

	res=mysql_store_result(db_live_handle);
	if(res && mysql_num_rows(res)>0){
		row=mysql_fetch_row(res);
		if(row[0])
			wscrd->serviceid=atoi(row[0]);
	}
	if(res){
		mysql_free_result(res);
		res=NULL;
	}
	return 0;
}

int  Get_PhoneType(char *t_field,int *cPhone_Type){

        struct ws_uri wsuri;
        char *ptr =NULL;
        if(!t_field || !cPhone_Type){
            LOG(L_ERR,"[Get_PhoneType]Any of the above is null,so return here\n");
            return 0;
        }
    if(t_field && strlen(t_field) >0 ){
        if(parse_ws_fifo_uri_buffer(t_field,strlen(t_field),&wsuri) != 1){
            if(ptr = strchr(t_field,'-')){
                if(ptr && ptr[-1] && isdigit(ptr[-1])) {
                    ptr--;
                    if(ptr){
                        *cPhone_Type = atoi(ptr);
                    }
                }else{
                    *cPhone_Type = SOFTPHONE;
                }
            }
        }
    }
    return 1;
}
int InsertReductionDetailsToDB(int nToken, char *cpTablename, char *cpStartTime, int nSiteid, int nDCID)
{
	int nRet = 0;
	char sqlcommand[2048]="";
	
	memset(sqlcommand, 0, sizeof(sqlcommand));
	snprintf(sqlcommand, sizeof(sqlcommand)-1, QUERY_REDUCTION, nToken, cpTablename, cpStartTime, nSiteid, nDCID);
	nRet = ExecuteDbQuery (&db_wh_handle , sqlcommand , WARE , 1);
	if(nRet == -1)
	{
		LOG(L_WARN , "[wsicpfifo::InsertReductionDetailsToDB]Failed query : '%s'\n" , sqlcommand);
		nRet = -1;
	}
	return nRet;
}

void GetCallerid(char *from,char *callername,char *callerid,int nLen){

	char cCallerName[64]="",cFrom[128]="",cName[128]="";	

	if(!from || !callername || !callerid){
		LOG(L_ERR,"[GetCallerid]Any of the above is NULL,so return here");
		return;
	}

	memset(cCallerName,0,sizeof(cCallerName));
	memset(cFrom,0,sizeof(cFrom));
	memset(cName,0,sizeof(cName));
	
	if(callername[0] == '+'){
		strncpy(cName,&callername[1],sizeof(cName)-1);
	}else{
		strncpy(cName,callername,sizeof(cName)-1);
	}

	if(isdigit(cName[0]) && strlen(cName) == 10){
		snprintf(cCallerName,sizeof(cCallerName)-1,"1%s",cName);
	}else{
		strncpy(cCallerName,cName,sizeof(cCallerName)-1);
	}

	memset(cName,0,sizeof(cName));
	if(from[0] == '+'){
		strncpy(cName,&from[1],sizeof(cName)-1);
	}else{
		strncpy(cName,from,sizeof(cName)-1);
	}

	if(isdigit(cName[0]) && strlen(cName) == 10){
		snprintf(cFrom,sizeof(cFrom)-1,"1%s",cName);
	}else{
		strncpy(cFrom,cName,sizeof(cFrom)-1);
	}

	if( (strlen(cCallerName) <=0 ) || (strlen(cFrom) == strlen(cCallerName)) && !strcmp(cFrom,cCallerName)){
		snprintf(callerid,nLen-1,"%s",cFrom);
	}else{
		snprintf(callerid,nLen-1,"%s (%s)",cFrom,cCallerName);
	}
}

/*  Added to get group extension to insert into voice CDR table for user's routing plan to auto attendent group case --Prashant/Surekha */
int get_group_ext(char *cGroupCode, char *cSitename, char *cGroupExt, int nLen, char *cGroupTitle, int nTitleLen) 
{
	char cQuery[2048] = "", orgsite[512] = "";
	int nRet = -1, i = 0;
	MYSQL_ROW row;
	MYSQL_RES *res = NULL;

	if(cGroupCode == NULL || strlen(cGroupCode) <= 0 || cSitename == NULL || strlen(cSitename) <= 0) {
		LOG(L_ERR,"[get_group_ext] *** either groucode or sitename is empty...so returning ");
		return -1;
	}

	memset(cQuery,  0x0, sizeof(cQuery));
	memset(orgsite, 0x0, sizeof(orgsite));
	memset(cGroupTitle, 0, nTitleLen);
	
	strncpy(orgsite, cSitename, sizeof(orgsite) - 1);
	GET_ORGSITENAME;
	
	snprintf(cQuery, sizeof(cQuery) - 1, SELECT_GRP_EXT_QUERY, cGroupCode, orgsite);

	nRet = ExecuteDbQuery(&db_live_handle, cQuery, LIVE, 0);
	if(nRet == -1) {
		LOG(L_ERR , "[get_group_ext] GRP_EXT Query execution failed : < %s >\n", cQuery);
		return -1;
	}

	res = mysql_store_result(db_live_handle);
	if(res == NULL) {
		LOG(L_INFO,"[get_group_ext] Result found NULL : '%s'\n",cQuery);
		return -1;
	}

    if(mysql_num_rows(res) <= 0) {
        mysql_free_result(res);
        LOG(L_INFO,"[get_group_ext] No rows found  : '%s'\n",cQuery);
        return -1;
    }

    row = mysql_fetch_row(res);
	if(row != NULL && row[0] != NULL && strlen(row[0]) > 0) {
		memset(cGroupExt, 0x0, nLen);
		strncpy(cGroupExt, row[0], nLen - 1);
	}
	if(row != NULL && row[1] != NULL && strlen(row[1]) > 0) {
		strncpy(cGroupTitle, row[1], nTitleLen-1);
	}

	mysql_free_result(res);

	return 0;
}

void getProperAgent(char *cName, char *cBuf, int nLen) 
{
    if(cName == NULL || strlen(cName) <= 0) {
        LOG(L_INFO,"[getProperAgent] cName found NULL \n");
        return;
    }
    int nI = 0;
    char cTemp[256] = "";
    char *cPtr = NULL;

    memset(cTemp, 0x0, sizeof(cTemp));
    strncpy(cTemp, cName, sizeof(cTemp) - 1);

    if((cPtr = strchr(cTemp, '1')) || (cPtr = strchr(cTemp, '2'))){
        *cPtr='\0';
        cPtr++;
        if(cPtr != NULL) {
            strcat(cTemp, cPtr);
        }
    }

    for(nI = 0; cTemp[nI]; nI++) {
        if(('@' == cTemp[nI]) || ('.' == cTemp[nI])) { 
            cTemp[nI] = '-';
        }
    }

    strncpy(cBuf, cTemp, nLen - 1);
}

/*Added to insert end point deatils in table --rajesh jun302021*/
void stir_racc_fifo_db_request() {

		char cStirCdrTable[128] = "", cSqlCommand[10240] = "";
		int nCode = 0, nRet = 0;
		struct ws_uri wsuri;

		char cTemp[256]="",cContactIp[128]="",clContactIp[128]="",*transport_type="UDP";
		char *ptr = NULL;

		GetMonthWiseTableName(cStirCdrTable, sizeof(cStirCdrTable));
		if(parse_ws_fifo_uri_buffer(wsacc_stir.req_uri,strlen(wsacc_stir.req_uri),&wsuri) != 1){
				LOG(L_WARN , "[wsicpfifo::stir_racc_fifo_db_request] parsing req_uri : %s is failed \n",wsacc_stir.req_uri);
				return;
		}
		if((ptr = strchr(wsuri.token, '@'))) {
				*ptr = '\0';
		}

		strncpy(cTemp,wsacc_stir.contact_ip,sizeof(cTemp)-1);
		ptr=strchr(cTemp,'@');
		if(ptr){
				strncpy(cContactIp, ptr + 1, sizeof(cContactIp)-1);
				ptr=strstr(cContactIp,";transport=");
				if(ptr) {
						if(!strncasecmp(ptr,";transport=tls", 14)) {
								transport_type = "TLS";
						}
						else if(!strncasecmp(ptr,";transport=tcp", 14)) {
								transport_type = "TCP";
						}
						*ptr = '\0';
				}

				ptr=strchr(cContactIp,';');
				if(ptr) {
						*ptr = '\0';
				}
				ptr=strchr(cContactIp,'>');
				if(ptr) {
						*ptr = '\0';
				}
		}

		bzero(cTemp, sizeof(cTemp));
		bzero(clContactIp, sizeof(clContactIp));
		strncpy(cTemp, wsacc_stir.lcontact_ip, sizeof(cTemp)-1);
		ptr=strchr(cTemp,'@');
		if(ptr){
				strncpy(clContactIp, ptr + 1, sizeof(clContactIp)-1);
				ptr=strstr(clContactIp,";");
				if(ptr) {
						*ptr = '\0';
				}
				ptr=strchr(clContactIp,'>');
				if(ptr) {
						*ptr = '\0';
				}
		}

		if(wsacc_stir.via_ips && strlen (wsacc_stir.via_ips) && strstr(cContactIp,":8070")){
				bzero(clContactIp, sizeof(clContactIp));
				bzero(cContactIp, sizeof(cContactIp));
				strncpy(clContactIp, wsacc_stir.via_ips, sizeof(clContactIp)-1);
				strncpy(cContactIp, wsacc_stir.via_ips, sizeof(cContactIp)-1);
		}

		if(strstr(cContactIp, ":6090") == NULL) { // Do not insert UMS contact ip list into endpoint details bug STIR-830
			snprintf(cSqlCommand, sizeof(cSqlCommand)-1, INSERT_INTO_ENDPOINT_CDR, cStirCdrTable, wsacc_stir.callid, wsuri.token,cContactIp,clContactIp,transport_type,who, nDataCenterID );
		}	

		if(strlen(cSqlCommand)) {
				nRet = ExecuteDbQuery(&db_wh_handle , cSqlCommand , WARE , 1);
				nCode = mysql_errno(db_wh_handle);
				if(nCode == MYSQL_TABLE_NOT_FOUND_ERRCODE) {
						nRet = CreateTableAndExecuteFailedInsert(cStirCdrTable,cSqlCommand, cEndPonitCdrTemplate);
				}
				if(nRet == -1){
						LOG(L_WARN , "[wsicpfifo::stir_racc_fifo_db_request]Failed qry : '%s'\n" , cSqlCommand);
				}

		}
}


/* Fun to get Specific monthwise tabel name for inserting  call records from all accounts */
void GetMonthWiseTableName(char *cMonthWiseTableName, int nLen){

		char cBuffer[64]="";
		time_t     present_time;
		struct tm *cLocalTime;

		present_time = time(NULL);
		cLocalTime = localtime(&present_time);
		strftime(cBuffer,sizeof(cBuffer)-1,"%m_%Y",cLocalTime);

		memset(cMonthWiseTableName,0,nLen);

		/* Copying table-name for inserting PENDING-USER records from all account */
		snprintf(cMonthWiseTableName,nLen-1,"%s%s",cEndPonitCdrPrefix,cBuffer);

		return;
}

/* Fun to create monthwise new tabel and will try to re-insert the 'Insertion failed Record (1146 - MYSQL ERROR)' for all accounts -- 26-12-2018 */
int CreateTableAndExecuteFailedInsert(char *cTableName, char *cInsertQuery, char *cTemplateName){

        int nRet=1;
        char cQuery [128] = "";

        if(cTableName == NULL || cInsertQuery == NULL || cTemplateName == NULL) {
                LOG(L_WARN, "[wsicpfifo::CreateTableAndExecuteFailedInsert] cTableName : <%p> cInsertQuery <%p>\n",cTableName,cInsertQuery);
                return -1;
        }

        memset(cQuery,0,sizeof(cQuery));
        snprintf(cQuery,sizeof(cQuery)-1,CREATE_TABLE_QRY,cTableName,cTemplateName);
        LOG(L_WARN, "[wsicpfifo::CreateTableAndExecuteFailedInsert] CREATE_TABLE_QRY : %s\n",cQuery);
        nRet = ExecuteDbQuery(&db_wh_handle , cQuery , WARE , 1);
        if(nRet == -1) {
                LOG(L_WARN, "[wsicpfifo::CreateTableAndExecuteFailedInsert] Failed qry : %s \n",cQuery);
                return nRet;
        }

        /* Re-Inserting the Insertion failed record */
        if(strlen(cInsertQuery)>0) {
                nRet = ExecuteDbQuery(&db_wh_handle , cInsertQuery , WARE , 1);
        }

        return nRet;
}

