#ifndef __FUN_H__
#define __FUN_H__
#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "../../msg_translator.h"
#include "../../data_lump.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_uri.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/contact/contact.h"
#include "../mysql/dbase.h"
#include "../../db/db.h"
#include "../../sr_module.h"
#include"../tm/tm_load.h"

#define WS_USE_RTP_PROXY 1
#define WS_DONT_USE_RTP_PROXY 2

/*This defines are used for the operator type checking*/
#define WS_OPRTYPE_GROUP 1
#define WS_OPRTYPE_USER 2

#define WS_SEPERATOR '-'

#define  SHM_OFFSET 24

/* Added for 3pcc calls*/
#define CTC_TAG "-3pcc"

#define UNSASSIGNED					0
#define ASSGINED_TO_GROUP			1
#define ASSIGNED_TO_EXTENSION		2
#define ASSIGNED_TO_CONFERENCE    	3
#define ASSIGNED_TO_X11PROFILE		4
#define ASSIGNED_TO_FAX				5
#define ASSIGNED_TO_SMARTCONNECT    6
#define ASSIGNED_TO_VOICEMAIL           7
#define ASSIGNED_TO_BLA             8
#define ASSIGNED_TO_NORMAL_GROUP_DID	9
#define ASSIGNED_TO_UCAPP           12

#define USA	1
#define UK	2

#define URI_VALID_CHARS	".-_+"

/* For Shared Memory */
#define PRIMARY 1
#define SECONDARY 2
#define DISABLED 	0 
#define ENABLED		1
#define SHM_KEY ((key_t)  11111)
#define SHMWSICP_KEY ((key_t)  11222)

#define MINPSTN_DIGITS  10
#define MAXPSTN_DIGITS  15

#define ASSOCIATIVE_ACC_CHECK 1
#define WS_USER_EXTEN  1
#define WS_GROUP_EXTEN 2
#define WS_STREAMAPP_EXTEN 3

/* Added for fixing duplicate call token */
#define PROCESS_BITS_LEN  5
#define SERVICEID_RANGE 8
#define PROCESS_COUNT 32
#define PROCESS_TOKEN_RANGE 0xFFFFFF
#define PROCESS_RANGE_BITS_LEN 24

#define QUERYSELREGCONTACT "SELECT registrar,contact,public_registrar,dcid FROM location WHERE username='%s' AND macid='%s'"

#define ACCOUNTDIDQUERY "SELECT STRAIGHT_JOIN a.siteid, extensions, assignedto, REPLACE(REPLACE(sitename, '.', '-'), '_', '-'),COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE siteid = s.siteid AND parameter = 2003),0),0,a.dcid,s.dcid FROM accountphonenumbers a, siteinfo_new s WHERE phnumber = '%s' AND a.siteid = s.siteid AND s.acctype != 2 AND a.assignedto < 100"

#define ACCOUNTDIDQUERY_TRANSFER "SELECT STRAIGHT_JOIN a.siteid, extensions, assignedto, REPLACE(REPLACE(sitename, '.', '-'), '_', '-'),COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE siteid = s.siteid AND parameter = 2003),0),COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE REPLACE(sitename, '.', '-') = '%s' AND parameter = 2003),0),a.dcid,s.dcid FROM accountphonenumbers a, siteinfo_new s WHERE phnumber = '%s' AND a.siteid = s.siteid AND s.acctype != 2 AND a.assignedto < 100"

#define MOBILECALL_QUERY "SELECT value,coalesce((SELECT pvalue FROM aclevel_pbxpreferences WHERE siteid = m.siteid AND parameter = 2003),0) FROM mobilecallpreferences m WHERE m.wsuserid='%s' AND m.priority=%d"

#define OLD_NATIONAL_QUERY 		"SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber s, pbxusers p, agent a, classofrules c WHERE c.outboundlocalldzoneid = 0 AND a.siteid = c.siteid AND c.id = a.classid AND s.username = '%s' AND s.agentid = p.agentid AND p.agentid = a.agentid UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber sub, pbxusers p, agent a, classofrules c, ldzonecodes ld, state_codes s WHERE sub.username = '%s' AND a.agentid = sub.agentid AND sub.agentid = p.agentid AND a.classid = c.id AND c.siteid = a.siteid AND code = %s AND s.scid = ld.scid AND ld.zoneid = c.outboundlocalldzoneid"

#define NATIONAL_QUERY "SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber s, pbxusers p, agent a, classofrules c WHERE c.outboundlocalldzoneid = 0 AND a.siteid = c.siteid AND c.id = a.classid AND s.agentid = '%s' AND s.agentid = p.agentid AND p.agentid = a.agentid  UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber sub, pbxusers p, agent a, classofrules c, ldzonecodes ld, state_codes s WHERE sub.agentid = '%s' AND a.agentid = sub.agentid AND sub.agentid = p.agentid AND a.classid = c.id AND c.siteid = a.siteid AND code = %s AND s.scid = ld.scid AND ld.zoneid = c.outboundlocalldzoneid UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM services sr, pbxusers p, agent a, classofrules c WHERE c.outboundlocalldzoneid = 0 AND a.siteid = c.siteid AND c.id = a.classid  AND a.agentid = '%s' AND a.agentid = p.agentid AND a.serviceid = sr.serviceid AND sr.sid in ('VIRTUALEXTENSION', 'VIRTUALEXTENSIONUNLIMITED', 'COLLABORATOR') UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM services sr, pbxusers p, agent a, classofrules c, ldzonecodes ld, state_codes s WHERE a.agentid = '%s' AND a.agentid = p.agentid AND a.classid = c.id AND c.siteid = a.siteid AND code = %s AND s.scid = ld.scid AND ld.zoneid = c.outboundlocalldzoneid AND a.serviceid = sr.serviceid AND sr.sid in ('VIRTUALEXTENSION', 'VIRTUALEXTENSIONUNLIMITED', 'COLLABORATOR')"

#define NANPAQUERY  "SELECT STRAIGHT_JOIN callerid, p.calleridflag FROM subscriber s, pbxusers p, agent a, classofrules c,siteinfo_new si WHERE c.outboundlocalldzoneid = 0 AND c.outboundnanpazoneid = 0 AND a.siteid = c.siteid  AND c.siteid = si.siteid AND si.siteid = a.siteid AND  s.siteid=c.siteid AND s.siteid=si.siteid AND p.siteid=c.siteid AND p.siteid=si.siteid AND c.id = a.classid  AND s.agentid = '%s' AND s.agentid = p.agentid AND p.agentid = a.agentid AND si.acctype != 2 UNION  SELECT STRAIGHT_JOIN callerid, p.calleridflag FROM  subscriber sub, pbxusers p, agent a, classofrules c, ldzonecodes ld, state_codes s,siteinfo_new si WHERE sub.agentid = '%s' AND a.agentid = sub.agentid AND sub.agentid = p.agentid AND a.classid = c.id AND c.siteid = a.siteid AND c.siteid = si.siteid AND si.siteid = a.siteid AND si.acctype != 2  AND code = '%s' AND s.scid = ld.scid  AND ld.zoneid = c.outboundlocalldzoneid UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM services sr, pbxusers p, agent a, classofrules c,siteinfo_new si WHERE c.outboundlocalldzoneid = 0 AND c.outboundnanpazoneid = 0 AND a.siteid = c.siteid  AND c.siteid = si.siteid AND si.siteid = a.siteid AND p.siteid=c.siteid AND p.siteid=si.siteid AND c.id = a.classid AND a.agentid = '%s' AND p.agentid = a.agentid AND si.acctype != 2 AND a.serviceid = sr.serviceid AND sr.sid in ('VIRTUALEXTENSION', 'VIRTUALEXTENSIONUNLIMITED','COLLABORATOR') UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM services sr,pbxusers p, agent a, classofrules c, ldzonecodes ld, state_codes s,siteinfo_new si WHERE a.agentid = '%s' AND p.agentid = a.agentid AND a.classid = c.id AND c.siteid = a.siteid AND c.siteid = si.siteid AND si.siteid = a.siteid AND si.acctype != 2  AND code = '%s' AND s.scid = ld.scid  AND ld.zoneid = c.outboundlocalldzoneid AND a.serviceid = sr.serviceid AND sr.sid in ('VIRTUALEXTENSION', 'VIRTUALEXTENSIONUNLIMITED','COLLABORATOR')"

#define INTERNATIONAL_QUERY  "SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber s, pbxusers p, agent a , classofrules c, siteinfo_new si WHERE c.outboundintezoneid = 0 AND si.siteid = a.siteid AND a.siteid = c.siteid AND si.intlenabled = 1 AND c.id = a.classid AND a.agentid = p.agentid AND '%s' = s.agentid AND s.agentid = p.agentid UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber s, pbxusers p, agent a, classofrules c, siteinfo_new si, countries co, intzonecodes i WHERE si.siteid = p.siteid AND p.siteid = a.siteid AND a.siteid = c.siteid AND si.intlenabled = 1 AND p.agentid = a.agentid AND '%s' = s.agentid AND s.agentid = p.agentid AND a.classid = c.id AND c.outboundintezoneid = i.zoneid AND co.ccode = '%s' AND i.contrycode = co.cid  UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM services sr, pbxusers p, agent a , classofrules c, siteinfo_new si WHERE c.outboundintezoneid = 0 AND si.siteid = a.siteid AND a.siteid = c.siteid AND p.siteid = a.siteid AND si.intlenabled = 1 AND c.id = a.classid AND a.agentid = '%s' AND a.agentid = p.agentid AND a.serviceid = sr.serviceid AND sr.sid in ('VIRTUALEXTENSION', 'VIRTUALEXTENSIONUNLIMITED','COLLABORATOR') UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM services sr, pbxusers p, agent a, classofrules c, siteinfo_new si, countries co, intzonecodes i WHERE si.siteid = p.siteid AND p.siteid = a.siteid AND a.siteid = c.siteid AND si.intlenabled = 1 AND a.agentid = '%s' AND p.agentid = a.agentid AND a.classid = c.id AND c.outboundintezoneid = i.zoneid AND co.ccode = '%s' AND i.contrycode = co.cid  AND a.serviceid = sr.serviceid AND sr.sid in ('VIRTUALEXTENSION', 'VIRTUALEXTENSIONUNLIMITED','COLLABORATOR')"

#define OLD_INTERNATIONAL_QUERY "SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber s, pbxusers p, agent a , classofrules c, siteinfo_new si WHERE c.outboundintezoneid = 0 AND si.siteid = a.siteid AND a.siteid = c.siteid AND si.intlenabled = 1 AND c.id = a.classid AND a.agentid = p.agentid AND '%s' = s.username AND s.agentid = p.agentid UNION SELECT STRAIGHT_JOIN callerid,p.calleridflag FROM subscriber s, pbxusers p, agent a, classofrules c, siteinfo_new si, countries co, intzonecodes i WHERE si.siteid = p.siteid AND p.siteid = a.siteid AND a.siteid = c.siteid AND si.intlenabled = 1 AND p.agentid = a.agentid AND '%s' = s.username AND s.agentid = p.agentid AND a.classid = c.id AND c.outboundintezoneid = i.zoneid AND co.ccode = '%s' AND i.contrycode = co.cid"

#define GROUP_EXT_QUERY "SELECT 1,s.siteid ,0,COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE siteid = s.siteid AND parameter = 2003),0),s.dcid FROM pbxusers p, siteinfo_new s WHERE p.siteid = s.siteid AND p.extensions = '%s' AND s.sitename=REPLACE('%s','-','.')  AND s.acctype != 2 UNION SELECT 2,s1.siteid , s1.code,COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE siteid = s2.siteid AND parameter = 2003),0),s2.dcid FROM special s1, siteinfo_new s2 WHERE s1.siteid = s2.siteid AND s1.extensions = '%s' AND s2.sitename=REPLACE('%s','-','.') AND s2.acctype != 2 UNION SELECT 3 , u.siteid , u.extension , COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE siteid = s3.siteid AND parameter = 2003),0),s3.dcid FROM uc_apps u, siteinfo_new s3 WHERE u.siteid=s3.siteid AND u.extension = '%s' AND s3.sitename=REPLACE('%s','-','.') AND s3.acctype != 2"

#define GROUP_EXTN_QUERY "SELECT 1,COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE parameter = 2003 AND siteid=s.siteid),0),s.stir_route,p.stir_enable,p.stir_route FROM pbxusers p, siteinfo_new s WHERE p.siteid = s.siteid AND  p.extensions = '%s' AND REPLACE(s.sitename,'.','-')='%s' UNION SELECT 2,COALESCE((SELECT pvalue FROM aclevel_pbxpreferences WHERE parameter = 2003 AND siteid=s2.siteid),0),s2.stir_route,0,0 FROM special s1, siteinfo_new s2 WHERE s1.siteid = s2.siteid AND s1.extensions = '%s' AND REPLACE(s2.sitename,'.','-')='%s'"

#define MSG_NOTIFY_QUERY "SELECT username,registrar,public_registrar,dcid,macid FROM location WHERE agentid='%s'"

#define ACCOUNT_BLOCK_QUERY "SELECT routeto,exten FROM aclevel_blocked_phonenumbers WHERE siteid=%d AND phonenumber='%s'"

#define GET_CONTACT_QUERY "SELECT distinct  registrar,public_registrar,dcid from location where username='%s'"

#define DCEROUTEQUERY "SELECT location_ip FROM ws_dc_route_locations WHERE dcid = %d AND state = 1"

#define GET_DCID_QUERY "SELECT DISTINCT dcid FROM acc_active WHERE sip_callid ='%s'"

#define GET_IVR_CMD_PLAN "SELECT extensions,(case when destination_type=0 AND is_user=1 then 1 when is_user=0 and  destination_type=2 then 4 end )flag,'' FROM pbxusers p,extendedpbxivrcommand e, siteinfo_new s WHERE e.siteid = s.siteid AND p.siteid = s.siteid AND p.siteid = e.siteid AND s.sitename = REPLACE('%s','-','.') AND e.extensiongroupcode = p.id AND e.groupcode = %s AND command = %s and acctype!=2 and acctype!=4 and destination_type in(0,2) UNION SELECT extensions,2,code FROM special sp,extendedpbxivrcommand e, siteinfo_new s WHERE sp.siteid = e.siteid AND e.siteid = s.siteid AND s.sitename = REPLACE('%s','-','.') AND e.groupcode = %s AND sp.code = e.extensiongroupcode AND command = %s AND is_user=0 and  destination_type=0 and acctype!=2 and acctype!=4 UNION select '',3,'' from extendedpbxivrcommand e , siteinfo_new si where e.siteid = si.siteid and si.sitename = REPLACE('%s','-','.') and groupcode = %s and command = %s and isoperator = 1 and acctype!=2 and acctype!=4 UNION SELECT extensiongroupcode , 5, '' FROM extendedpbxivrcommand e, siteinfo_new s WHERE e.siteid=s.siteid AND s.sitename= REPLACE('%s','-','.')  AND e.groupcode = %s AND command = %s AND is_user=0  and  destination_type=3  and acctype!=2 and acctype!=4  UNION SELECT extensiongroupcode ,6,u.id FROM extendedpbxivrcommand e, siteinfo_new s,uc_apps u WHERE e.siteid=s.siteid AND u.siteid=s.siteid AND extensiongroupcode=u.extension AND s.sitename= REPLACE('%s','-','.')  AND e.groupcode = %s  AND command = %s AND is_user=0  and  destination_type=4  and acctype!=2 and acctype!=4 "

#define IVR_OPR_QUERY "select extensions from special s,siteinfo_new si where code = %d and s.siteid = si.siteid AND si.sitename = REPLACE(REPLACE('%s','-','.'),'_','.')"

#define EXT911_QUERY "SELECT siteid,classid FROM agent WHERE agentid='%s'"

#define MWI_SUBSCRIBE_QUERY "SELECT DISTINCT 1 FROM pp_subscription WHERE macid='%s' AND (tousername='%s'  OR tousername='%s'  OR tousername='%s') AND event='message-summary'"

#define UCAPPID_QUERY "SELECT id FROM uc_apps WHERE extension = '%d' and siteid = '%d'"

#define SELECT_IVR_DETAILS "SELECT code, acdenabled, extendedgroup, enableoperator, s.siteid, grouptype,s.stir_route FROM special sp, siteinfo_new s WHERE sp.siteid = s.siteid AND sitename=REPLACE('%s', '-', '.') AND (sp.extensions = '%s' or sp.code = '%s') AND acctype!=2 AND acctype!=4"

/*Added below queries(EXT611_QUERY,GET_USER_EXTN_QUERY) for 611 calls*/
#define GET611_AGENTID_QUERY "SELECT psc.value,so.dealer_id FROM pbxshortcodes psc, siteowners so, agent a WHERE so.siteid = (select siteid from siteinfo_new where sitename=replace('%s','-','.') AND acctype not in (2,4)) AND (so.dealer_id = psc.dealer_id OR psc.dealer_id = (SELECT refererid FROM sitemasters WHERE dealercode = so.dealer_id) ) AND psc.shortcode = 611 AND psc.wholesaleflag = 1  AND a.agentid=psc.value"

#define GET_611USER_EXTN_QUERY "SELECT a.extensions FROM agent a, siteinfo_new s WHERE a.agentid = '%s' AND a.siteid = s.siteid"

#define GET_SITEID_QUERY "SELECT siteid FROM siteinfo_new WHERE sitename = REPLACE('%s','-','.')"

#define LOCAL_CALLERID_QUERY "SELECT p.callerid, p.calleridflag, p.isautomatic_callerid_enabled,p.extensions,s.is_150_rate_limit_enabled,s.defaultcallerid FROM pbxusers p,siteinfo_new s WHERE p.siteid=s.siteid AND p.agentid='%s'"

#define LOCAL_CALLERID_QUERY_611 "SELECT callerid,si.siteid FROM pbxusers p,siteinfo_new si  WHERE p.agentid='%s' and si.sitename='%s'"
#define SITENAME_PARSE for (i=0; cSitename[i]; i++) {if ('-' == cSitename[i]) cSitename[i] = '.';}

#define FAX_CALLERID_QUERY "select c.phonenumber, si.defaultcallerid  from  conference c,siteinfo_new si where c.extension = '%s' and  REPLACE(si.sitename, '.', '-')  = '%s' and si.siteid = c.siteid"

#define WSDID_USER_DETAILS "SELECT ac.siteid , pb.agentid FROM accountphonenumbers ac , pbxusers pb WHERE ac.siteid=pb.siteid AND ac.extensions=pb.extensions AND ac.phnumber='%s'"

#define END_USER_IP_DETAILS "SELECT endpointip FROM %s WHERE sip_callid='%s'"

#define ISFRAUDTRIGGERED "SELECT 1 FROM fraud_trigger_call_block_by_account WHERE siteid= %d UNION SELECT 1 FROM fraud_trigger_call_block_by_user WHERE siteid= %d and agentid='%s'"

#define DIAL_10DIGIT_QUERY "SELECT 1 from npa_dialing_exclude_codes WHERE code = '%s' limit 1"


enum call_interruption_types{
	BLOCK_OB = 1,
	BLOCK_IB,
	BLOCK_BOTH,
};

enum MediaFeature {
    SmartConnect = 9994,
	AutoProvision = 9995,	
	AcdAgent = 9997,
	GreetingAdmin = 9998,
	VoicemailAdmin
};

enum {
	VML_FLAG = 1,
	BUSY_FLAG,
};

enum Ivr_Type {
	Invalid_Ivr = -1,
	Ivr_User = 1,
	Ivr_Group,
	Ivr_Operator,
    Ivr_Vml,
    Ivr_PSTN,
    Ivr_Streamlet 
};
enum GroupType {
	InvalidGroupType,
	RegularGroup,
	ExtendedGroup,
	ACDGroup,
	OperatorRouted,
	DisasterACDGroup,
	BLAGroup
};

struct ws_uri {
	char parsed[100];
	char * type;
	char * command;
	char * group;
	char * context;
	char * token;
};

struct PstnPhone {
	char * phone;
	int nSiteid;
	int nAssignedTo;
	int nAssignedID;
	char sitename[50];
};

enum CallSource {
	MediaServer,
	ACDServer,
    Registrars,
	MySelf,
	OtherSource
};

/*EXT - EXT Call Via MS --- Vijay Feb2009 */
enum UriType {
	Invalid_Uri = -2,
	Extension = 0,
	IVR_Command,
	Xfer4DID,
	CNF_Request,
	ACD_Agent_Extension,
	PSTN_Transfer,
	Pickup_Request,
	ACD_Request,
	DIR_Request,
	INV_Request,
	ACD_Voicemail,
	ACD_Voicemail_Transfer,
	ACD_Agent,
	ACD_DID,
	MWI_Request,
	Push_To_Talk,
	Disaster_Request,		
	OB_Transfer,
	Acccode_Type,
	MOBILECALL,
	VMD_Command,
	VM_OPRCALL,
	IEXTCALL,
	ICOMCALL,
	Bla_PhoneNumber=24,
	GEXT,
	ACEFORWARD,
	NINE11,
	ACDXFER,
	CTC_Request,
	WSCTCBCALL,
	WSPCR,
	FGRP,
	SIX11,
	LCR_Request,
	Phone_Number,
	International_Phnumber,
	Feature_Request,
	WS_URI,
	Local_User
};

enum server_types{
    MS=1,
    ACDMS,
    OMS,
	OTHER,
};

enum notify_types{
	NOTIFY_REBOOT = 1,
	NOTIFY_MWI
};

enum stir_route{
	STIR_REJECT = 1,
	STIR_REJECT_WITH_PROMPT,
	STIR_ROUTE_WITH_SPAM
};

typedef struct shm_ids{
	int shmid;
	int insert_time;
	int in_use;
} SHMIDS;

/* Exported parameters */
extern char *db_url;
extern char *voicemail_server;
extern char *proxy_domain;
extern char *emergency_server;
extern char *conference_server;
extern char *fax_server;
extern char *smartconnect_server;                                                                                                                             
extern char *national_prefix;
extern char *international_prefix;
extern int  prefix_length;
extern char *umslb_proxy;
extern char *umslb_proxy1;
extern char *registrar_domain;
extern char *mediaserver_ips;
extern char *servertz;
extern int country_code;
extern int max_bla_lines;
extern char  * ws_dummy_site; 
extern int global_callblocking;
extern int datacenter_id;
extern char  * Contact_invalid_Chars;
extern char *wsicp_primary;
extern char *wsicp_secondary;
extern int mwi_subscribe_flag;

typedef char ip_list[48][24];
extern ip_list wsregips;
extern ip_list wsicpips;
extern ip_list mediaips;

extern struct tm_binds _tmb;//added for CNAM --Nikhil

extern ip_list msuris;
extern ip_list omsuris;
extern ip_list acduris;

extern cmd_function t_relay;
extern cmd_function t_on_failure;
extern cmd_function append_hf;
extern cmd_function sl_send_reply;

enum UriType get_uri_type(struct sip_msg *msg, struct ws_uri * wsuri, char *,char *token,char *prio1,int uri_len,int token_len);
int set_contact_username (struct sip_msg *, char * );
int parse_ws_uri(struct sip_msg *, struct ws_uri *, char *,char *token);
int parse_ws_uri_buffer(char * buffer, int nLength, struct ws_uri *);
int worksmart_process_acd_f (struct sip_msg * msg, db_con_t *, db_func_t);
int get_greeting(int nGroup, char * context);

int route_to_pstn (struct sip_msg *msg, char * phone) ;
int route_to_outboundserver1( struct  sip_msg * msg,char *phone,struct ws_uri *wsuri,char *);
int route_to_outboundserver( struct  sip_msg * msg  ,  char *phone);
int route_to_media_server(struct sip_msg *msg, char * phone,char *cCalleridBlock) ;
int route_to_server(struct sip_msg *msg, char * phone, char * server,int sflag); 
int set_uri2 (struct sip_msg *msg, char * phone);
int normalize( char * phone, char * buffer, int nBufferLen,int phone_len);
int call_t_relay(struct sip_msg * msg);
int send_reply(struct sip_msg * msg, int nCode, char * pMessage);
int ws_set_rpid( struct sip_msg *msg, char *target_server,const char*,char *cExtn ,int siteid,int *is_blocked_number, int nLen, int nGcbflag, int nAddRpid, char *cSiteName);
int get_db_int(db_val_t value);
int isInList(ip_list list, char *  source );
int send_publish_message(char *cid,int sid,char *server,char *ownip);//Added for CNAM --Nikhil
int ws_ext_call_process (struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag,int extn_callid_pass,int associate_acc_chk,int ob_ext_txfer, int rpid_flag);
int ws_geturi_contact(struct sip_msg *msg, str aor, int *append);
int GetRemoteDataCenterIP(int nDcid, char *cLocationIP, int nLen);
int is_header_exists(struct sip_msg *msg,char *header_name,int header_name_len,char *header_value,int header_value_len);
int add_header(struct sip_msg *msg,char *header,int header_len);
int get_wsucapp_id(int did,int nSiteid);
int Validate_Contact(struct sip_msg *msg);

#endif

