/*
 *	File	: 	WORKSMART.C
 *	Notes	: 	This is the CONFIDENTIAL property of PANDORA NETWORKS (IND) PVT. LTD.
 *
 *	LOG		:	memset's are replaced with bzero (22nd Dec 2004)
 */
#define TRUE 1
#define FALSE 0
//#define SMARTDBG
#define USE_RTP_PROXY
//#define USE_MEDIA_PROXY
#define DIDSIZE 25
#define MINEXTLEN 3

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

#include "../../db/db.h"
#include "../mysql/dbase.h"
#include "../../route_struct.h"
#include "../registrar/lookup.h"
#include "../../data_lump.h"
#include "../../dset.h"
#include "../../mem/mem.h"
#include "../../msg_translator.h"
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"
#include "../tm/tm_load.h" /*Project:CNAM , Date:29-10-2009*/

#include "../../parser/msg_parser.h"
#include "../../parser/parse_uri.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/contact/contact.h"
#include "../../parser/parse_from.h"
#include <regex.h>
#include <mysql/mysql.h>
#include <pthread.h>

/*!The  following header are added for Worksmart*/
/*We need to an bind_usrloc_t  variable here for just db */

#include "../usrloc/usrloc.h"

#include "fun.h"

#define REPLACE_DOT(string) for (i=0; string[i]; i++) {\
	if ('.' == string[i]) string[i] = '-';\
}

#define SHMSEGMENTSIZE_PREV (SHM_OFFSET + ( noofshmsegments_prev * sizeof(SHMIDS)))
#define SHMSEGMENTSIZE (SHM_OFFSET + ( noofshmsegments * sizeof(SHMIDS)))

MODULE_VERSION

static int mod_init (void);
static void mod_destroy (void);
static int connect_db (void);
static int init_shm (int _rank);
static int child_init(int _rank );

static int get_resp_code(struct sip_msg *);
int use_secondary_server(struct sip_msg *, char *, char *);
int use_secondary_wsicp_server(struct sip_msg *, char *, char *);


int get_uri_contact(struct sip_msg * msg, char *phoneno, int *append,int (*fptr)(int *), int *);
int get_direct_extension (char *, int );
int get_extended_plan(struct sip_msg * msg, char * prio, struct ws_uri * wsuri);
int ws_set_iattr(struct sip_msg *, int);
int process_dir_request (struct sip_msg *, struct ws_uri *);
int process_3pcc_call (struct sip_msg *, char *);
int process_3pccsup_call (struct sip_msg *, char *,int);
int process_acd_agent_call2 (struct sip_msg *, char *, struct ws_uri *);
int process_conf_call2 (struct sip_msg *, struct ws_uri *, int );
int process_ob_ext_call2 (struct sip_msg *msg, char *priority, struct ws_uri *wsuri);
int process_ext_call2 (struct sip_msg *, char *, struct ws_uri *, int nFlag, int extcall,int rpid_flag); /*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
int process_ivr_command2 (struct sip_msg *msg, char *priority, struct ws_uri *wsuri,int nFlag);

/*Added by tulsi*/
int process_vmopr_call (struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag );
int process_bla_call (struct sip_msg *msg, char *priority ,struct ws_uri *wsuri, int nFlag);

int process_x11_returnedcall(struct sip_msg *, char *);
int worksmart_get_dialplan2 (struct sip_msg *, char *, char *);
int worksmart_route_to_operator (struct sip_msg *, struct PstnPhone);
int worksmart_route_to_operator2 (struct sip_msg *, int,int , char *,char *token);
int worksmart_route_to_ivr2 (struct sip_msg *msg,int nFlag) ;

int ws_set_transfer_uri (struct sip_msg *, char *, char *);
int ws_set_contact_uri (struct sip_msg *, char *, char *);
int ws_process_subscribe (struct sip_msg *, char *, char *);
static int ws_process_mwi_notify (struct sip_msg *, char *, char *);
static int ws_process_reboot_notify (struct sip_msg *, char *, char *);
int ws_process_911 (struct sip_msg *, char *, char *);
int ws_process_302(struct sip_msg * , char * , char *);
int ws_process_notify (struct sip_msg *msg, char *phone, char *p2);
int ws_process_response(struct sip_msg *msg);

int create_branches (struct sip_msg *, db_res_t *, int, struct ws_uri *);
int disconnect_db();

/*Call forked to unlimited users in Hunt group */
int shm_create(db_res_t * result, char *cToken,int nRows);
int process_nested_forking(struct sip_msg *msg,db_res_t * result,char *token,char *cSitename,int nRows);
int generate_forkgroup(int nStart, int nEnd, char * uri,struct sip_msg * msg,char *cSitename,char *cToken, int nPrevSize,int *nAppend, int nVal,int nRows,int uri_len);
int UpdateShmCounter(char *cToken,char * cUri);
int create_fork_branches ( struct sip_msg * msg , struct ws_uri * wsuri);

int set_rpid_from_user (struct sip_msg *msg, char * dstphone, char *cTransferred_Agentd, char *cToken );
int ws_set_group_name(struct sip_msg *, char *);
int is_permited (char * context, char * extension, char * dstphone ) ;

/*! This Reset the Disaster State and revert the INVITE to Media Server*/
int ws_systemwide_disaster (struct sip_msg  * msg,char *token);
static int isinlist_f(struct sip_msg *msg, char * i, char * j);
static int f_clear_forkgroup_shared_memory(struct sip_msg *msg , char *cToken, char *cBuffer);

/*One Touch Call pickup / Attended transfers - Invite with Replace Header*/
int ws_process_invite_replaces(struct sip_msg * msg,char *prio1);

int get_group_cmd_plan ( struct sip_msg *msg, char * prio, struct ws_uri * wsuri, int nFlag) ;
int get_opr_grp_plan ( struct sip_msg *msg, char * prio, struct ws_uri * wsuri, int nExten,int nFlag) ;

/*For Group ext call project*/
int process_group_ext_call(struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag, int extcall, int rpid_flag);
int ws_grp_ext_call_process( struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag, int extcall, int rpid_flag,int nStreamletivr);
static int GetCallLocation(struct sip_msg *msg, char *cCallID, char *cUri);
int route_to_fwdout_server(struct  sip_msg * msg,char *prio1,struct ws_uri * wsuri);

int shmid_clear_onstartup ();
int update_shmid(int shmid,int nCount);
int insert_shmid_shm (int shm_id);
int select_shmid (int shm_id);
/*To get the agent id in correct format like abc@pbxtesting*/
void get_wsagent_id(const char *inbuffer,char *outbuffer,int outbuf_len, int *value);
/*! 
 * 	Project:CNAM
 *  Date:18-11-2009
 *  Usage: fuctions and varialbles  
 *  */
int ws_cnam_request(struct sip_msg *msg, int siteid);
int isphnum(char *string, int len);
/* Added to allow extension in PSTN phone field from users routing plan		--Prashant/Swaroopa */
int frame_uri(struct sip_msg *msg, struct ws_uri *wsuri, char *cNewuri, int nLen, char *cAgentId, char *cGrpName, int *nDcid);
int check_conf_exten (struct sip_msg *msg,  struct ws_uri *wsuri, int nSiteid, char * cNewuri, int nLen);
int frame_group_uri (struct sip_msg * msg, struct ws_uri *wsuri, int nGroupId, int nSiteId, int gType, char *disasterphonenumber, char *cNewuri, int nLen);
int frame_associate_acc_uri(struct sip_msg *msg, char *priority, struct ws_uri *wsuri, char *cNewuri, int nLen);
int frame_feature_codes_uri (struct sip_msg *msg, int nFeature, char *cToken, char *cSitename, char *cNewuri, int nLen);
int frame_611_call_uri(struct sip_msg *msg, struct ws_uri *wsuri, char *cNewuri, int nLen);
int GetSiteId(char *nSiteId);
int validate_calleid_611(struct sip_msg * msg, char *cToken, char * cSix11extensionsitename);
/*Added to get to and from user from to and from headers --rajesh */
// STIR SHAKEN NEW FUNCTIONS START
static void get_fromuser(struct sip_msg *msg,char *fromuser,int);
int route_to_balancer_on_did_checks(struct  sip_msg * msg, char * realdid , char * token ,char *didsitename );
int IsSpamCall(struct sip_msg *msg);
int prepare_contact_spam_route_fail(struct sip_msg *msg, int nRouteType);
int preparePAIheader(struct  sip_msg * msg);
void GetMonthWiseTableName(char *cMonthWiseTableName, int nLen);
// STIR SHAKEN NEW FUNCTIONS END

load_tm_f loader_tm;
struct tm_binds _tmb;
/*-----------*/
/*TO know phone Number is DID or PSTN --vishal*/
int CheckPSTNorDID(const char *cPhoneNumber);

int get_isext911_plan(struct sip_msg * msg,int callee_siteid);
/*Process 611 Transfer Calls --Jagan*/
int process_611_calls(struct sip_msg *,struct ws_uri * ,char *);
void generate_unique_process_range(int Rank);
unsigned generate_unique_token(void);
int modify_from_to_for_buddydialing(struct sip_msg *msg, char *touser, char *token);
int get_from_displayname(struct sip_msg *msg, char *display, int len);
int GetAgentAssignedDID(char *agentid, char *phnumber, int len);
int GetAgentCallerId(char *agentid, char *callerid, int len);

int seconds_range = 0x1FFFF;
int cps_bits_len = 7;
int max_cps = 0x7F;

SHMIDS * shmids_ptr = NULL;
sem_t *sem_key = NULL;

static db_con_t *h = NULL;
db_func_t ws_dbf;
int _nOrigSeq = 0;

int _nNoRelay = 0;
key_t shm_key = 0;

/*Added by tulsi for ip checking*/
ip_list wsregips;
ip_list wsicpips;

ip_list msuris;
ip_list omsuris;
ip_list acduris;

ip_list specialnumbers;

enum CallSource  Source;

/*Added for Virtual Phone seat-Tulsi-21-07-09*/
char phonelist[MAX_BRANCHES][256];
/*Added by tulsi for media pool attended transfer*/
int _nSupSeq = 0;
int media_failover = 1;
ip_list mediaips;
int warmflag=0;
char group_name[256] = "";	///Added for callerid changes
#define WS_ENTRYSEPERATOR    '|'

typedef struct {
    int nDisasterMode;
    int nMediaServerType;
} WSServerState;

typedef struct {
    int nDisasterMode;
    int nICPServerType;
} WSicpServerState;

int shmid = 0;
int shm_clear_id = 0;
int shmforkgrpid = 0;

int shmwsicpid = 0;
WSServerState * WSState = NULL;
WSicpServerState * WSicpState = NULL;

/* Exported parameters */
char *db_url = NULL;
char *voicemail_server = NULL;
char *proxy_domain = NULL;
char *emergency_server = NULL;
char *conference_server = NULL;
char *fax_server = NULL;
char *smartconnect_server = NULL;
char *national_prefix = NULL;
char *international_prefix = NULL;
int  prefix_length = 0;
char *umslb_proxy = NULL;
char *umslb_proxy1 = NULL;
char *registrar_domain = NULL;
char *mediaserver_ips = NULL;
char *servertz = NULL;
int country_code = 0;
int max_bla_lines =0;
char  * ws_dummy_site = NULL;
int global_callblocking = 0;
int datacenter_id = 0;
int nOtherDataCenterId = 0;
int nRouteAcdAndConfCallsToHomeDc = 0;
int enable_dc_transfer = 0;
int nMaxExtLen = 0;       // Added to check max extension len
/*611 support values reading from conf*/
char  * ws_support_site     =   NULL;
char  * ws_support_exten    =   NULL;
int engage_mediaanchor = 0; //Added for Media Anchor

char *Contact_invalid_Chars =   NULL;
int Contact_Regcheck	=	0;

char *wsicp_primary = NULL;
char *wsicp_secondary = NULL;
char *primary_fwdout_server = NULL;
char *secondary_fwdout_server = NULL;
char *cEndPonitCdrPrefix = NULL;

/* Added for subscription based MWI changes */
int mwi_subscribe_flag = 0;

/* Setting Default Values for FGRP Changes */
int is_nested_group = 0;
int shmcleartimer = 60;
int shmdeletetimer = 10;
int cMaxForksInGrp = 150;
int noofshmsegments = 2000;
int noofshmsegments_prev = 2000;
int fgrphuntgrouplimit = 200;
int watchersleep = 2;
int MAX_FORK_BRANCHES= 20;
int extonly_enable = 0;

int delete_to_header_failover = 0;
int nGrpId_Parser = 6;
int _nEnableCalleridBlockCDR = 0;
int nEnableCosTransfer = 0;
int nStirShakenRoute = 0;
int badnumber_reject_code = 0;

/*Added for fixing duplicate call token */
int serviceid = 0;
unsigned uniq_process_id = 0;
unsigned cps = 0;

char *mwi_subscribe = NULL;
char *mwi_notify = NULL;

char *acduseragent = NULL;
char *msuseragent = NULL;

char *registrar = NULL;
char *ws_registrar = NULL;
char *ws_icpips = NULL;
char *ms_uris = NULL;
char *oms_uris = NULL;
char *acd_uris = NULL;
char *special_numbers = NULL;
char *threepcc_server = NULL;//Added for CNAM
char *own_ip = NULL; //Added for CNAM
char *cnam_enable = NULL;

/* Imported Functions for worksmart */
cmd_function t_relay = NULL;
cmd_function ws_on_reply = NULL;
cmd_function t_on_failure = NULL;
cmd_function set_iattr = NULL;
cmd_function use_media_proxy = NULL;
cmd_function force_rtp_proxy = NULL;
cmd_function append_hf = NULL;
cmd_function remove_hf = NULL;
cmd_function replace_hf = NULL;
cmd_function racc_302 = NULL;
cmd_function racc_missed_call_entry = NULL;
cmd_function sl_send_reply = NULL;
cmd_function t_check_status = NULL;
cmd_function ext_rewriteuri = NULL;
cmd_function ws_t_reply = NULL;
cmd_function _get_agentid=NULL;

cmd_function get_tran=NULL;
cmd_function save_stir_details_to_msg=NULL;

/*Parsing IBCP IP addresses from the configuration file*/
#define PARSE_IP_LIST(iplist,ips) {\
	char *ip=NULL;\
	char *temp=NULL;\
	int count=0;\
	ip = ips;\
	temp = ips;\
	while(ips!='\0'){\
		ip = strchr(temp ,WS_ENTRYSEPERATOR);\
		if(ip==NULL)\
			break;\
			*ip='\0';\
			ip++;\
			strncpy(iplist[count], temp,sizeof(iplist[count])-1);\
			count++;\
			temp = ips = ip;\
	}\
	if(temp){\
		strncpy(iplist[count], temp,sizeof(iplist[count])-1);\
	}\
}

/* Exported commands from the module */
static cmd_export_t cmds[]={
	{"worksmart_get_dialplan",  (cmd_function) worksmart_get_dialplan2,	2, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"use_secondary_server",  (cmd_function) use_secondary_server,	1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_set_transfer_uri",     (cmd_function) ws_set_transfer_uri,     1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_set_contact_uri",  (cmd_function) ws_set_contact_uri,          1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_process_subscribe",    (cmd_function) ws_process_subscribe,    1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_process_notify",    (cmd_function) ws_process_notify,  1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_process_911", (cmd_function) ws_process_911, 2, 0, REQUEST_ROUTE},
	{"isinlist", (cmd_function)isinlist_f, 1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_isinlist", isinlist_f, -1, 0, 0 },
	{"ws_process_invite_replaces",  (cmd_function) ws_process_invite_replaces,  1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_process_302",    (cmd_function) ws_process_302,  1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"use_secondary_wsicp_server",  (cmd_function) use_secondary_wsicp_server,	1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"ws_clear_forkgroup_shared_memory",  f_clear_forkgroup_shared_memory, -1, 0,0},
	{"ws_process_response", ws_process_response, 0, 0, ONREPLY_ROUTE},
	{0, 0, 0, 0, 0}
};

/* Exported parameters from the module */
static param_export_t params[]={
	{"db_url",  STR_PARAM, &db_url},
	{"proxy_domain", STR_PARAM, &proxy_domain},
	{"emergency_server", STR_PARAM, &emergency_server},
	{"fax_server", STR_PARAM, &fax_server},
   	{"smartconnect_server", STR_PARAM, &smartconnect_server},	
	{"3pcc_server",STR_PARAM,&threepcc_server},//Added for CNAM Project
	{"own_ip",STR_PARAM,&own_ip},// Added for CNAM Project
	{"cnam",STR_PARAM,&cnam_enable},// Added for CNAM Project
	{"national_prefix", STR_PARAM, &national_prefix},
	{"international_prefix", STR_PARAM, &international_prefix},
	{"prefix_length", INT_PARAM, &prefix_length},
   	{"umslb_proxy", STR_PARAM, &umslb_proxy},
   	{"umslb_proxy1", STR_PARAM, &umslb_proxy1},
   	{"conference_server", STR_PARAM, &conference_server},
   	{"servertz", STR_PARAM, &servertz},
	{"country_code", INT_PARAM, &country_code},
	{"mwi_subscribe", STR_PARAM, &mwi_subscribe},
   	{"mwi_notify", STR_PARAM, &mwi_notify},
   	{"acduseragent", STR_PARAM, &acduseragent},
   	{"msuseragent", STR_PARAM, &msuseragent},
	{"registrar", STR_PARAM, &registrar},
   	{"ws_registrar", STR_PARAM, &ws_registrar},
	{"ws_icpips", STR_PARAM, &ws_icpips},
	{"msuris", STR_PARAM, &ms_uris},
	{"omsuris", STR_PARAM, &oms_uris},
	{"acduris", STR_PARAM, &acd_uris},
	{"specialnumbers", STR_PARAM, &special_numbers},
	{"mediaserver_ips", STR_PARAM, &mediaserver_ips},
	{"registrar_domain", STR_PARAM, &registrar_domain},
	{"ws_dummy_site", STR_PARAM,&ws_dummy_site},
	{"max_bla_lines", INT_PARAM,&max_bla_lines},
	{"global_callblocking", INT_PARAM,&global_callblocking},
	{"datacenter_id", INT_PARAM,&datacenter_id},
	{"other_datacenterid", INT_PARAM,&nOtherDataCenterId},
	{"route_acd_conf_calls_to_homedc", INT_PARAM,&nRouteAcdAndConfCallsToHomeDc},
	{"serviceid",INT_PARAM,&serviceid},
	{"is_nested_group",INT_PARAM,&is_nested_group},
	{"shmcleartimer",INT_PARAM,&shmcleartimer},
	{"shmdeletetimer",INT_PARAM,&shmdeletetimer},
	{"max_forks_for_group",INT_PARAM,&cMaxForksInGrp},
	{"shmsegments",INT_PARAM,&noofshmsegments},
	{"shmsegments_prev",INT_PARAM,&noofshmsegments_prev},
	{"fgrphuntgrouplimit",INT_PARAM,&fgrphuntgrouplimit},
	{"watchersleep",INT_PARAM,&watchersleep},
	{"enable_dc_transfer", INT_PARAM,&enable_dc_transfer},
	{"MAX_FORK_BRANCHES", INT_PARAM,&MAX_FORK_BRANCHES},
	{"extonly_enable",INT_PARAM,&extonly_enable},
	{"Contact_Regcheck",INT_PARAM,&Contact_Regcheck},
	{"Contact_invalid_Chars",STR_PARAM,&Contact_invalid_Chars},
   	{"wsicp_primary", STR_PARAM, &wsicp_primary},
   	{"wsicp_secondary", STR_PARAM, &wsicp_secondary},
	{"mwi_subscribe_flag", INT_PARAM,&mwi_subscribe_flag},
	{"ws_support_site", STR_PARAM,&ws_support_site},
	{"ws_support_exten", STR_PARAM,&ws_support_exten},
	{"primary_fwdout_server", STR_PARAM, &primary_fwdout_server},
	{"secondary_fwdout_server", STR_PARAM, &secondary_fwdout_server},
	{"delete_to_header_failover",INT_PARAM,&delete_to_header_failover},
	{"max_ext_len",INT_PARAM,&nMaxExtLen},
	{"nGrpId_Parser", INT_PARAM, &nGrpId_Parser},
    {"cdr_callerid_block", INT_PARAM,&_nEnableCalleridBlockCDR},
	{"engage_mediaanchor",INT_PARAM,&engage_mediaanchor},
	{"enable_cos_transfer",INT_PARAM,&nEnableCosTransfer},
	{"stir_shaken_route",INT_PARAM,&nStirShakenRoute},
	{"badnumber_reject_code",INT_PARAM,&badnumber_reject_code},
	{"endpoint_cdr_prefix",  STR_PARAM, &cEndPonitCdrPrefix},
	{0, 0, 0}
};

struct module_exports exports= {	
	"worksmart", 		/* Module name			*/
	cmds,				/* Exported functions	*/
	params,				/* Exported parameters	*/
	mod_init,       	/* Module init function	*/
	0,    				/* Response function 	*/
	mod_destroy ,		/* Destroy function 	*/
	0,       			/* On cancel function	*/
	child_init 			/* Child init function	*/
};

static int mod_init(void)
{
	/* Imported Methods */
	t_relay = find_mod_export("tm", "t_relay", 0, REQUEST_ROUTE);
	ws_t_reply = find_mod_export("tm", "t_reply", 2, REQUEST_ROUTE | FAILURE_ROUTE );
	ws_on_reply = find_mod_export("tm", "t_on_reply", 1, REQUEST_ROUTE);
	t_on_failure = find_mod_export("tm", "t_on_failure", 1, REQUEST_ROUTE);
	t_check_status = find_mod_export("tm", "t_check_status", 1 , REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE);
	append_hf = find_mod_export("textops","append_hf", 1, REQUEST_ROUTE);
	remove_hf = find_mod_export("textops","remove_hf", 1, REQUEST_ROUTE);
	set_iattr = find_mod_export("avp", "set_iattr", 2, REQUEST_ROUTE);
	
	use_media_proxy = find_mod_export("mediaproxy", "use_media_proxy", 0, REQUEST_ROUTE);
	
	force_rtp_proxy = find_mod_export("nathelper", "force_rtp_proxy", 0, REQUEST_ROUTE);
	
	racc_302 = find_mod_export("racc","racc_process_moved_temporarily", 1, REQUEST_ROUTE);
    racc_missed_call_entry = find_mod_export("racc","ws_missedcall_entry_for_rejected_calls", 1, REQUEST_ROUTE);
	sl_send_reply = find_mod_export("sl_module", "sl_send_reply", 2 , REQUEST_ROUTE);
	ext_rewriteuri = find_mod_export("ext", "ext_rewriteuri", 1, REQUEST_ROUTE);
	_get_agentid = find_mod_export("wsutils", "get_agentid", 2, REQUEST_ROUTE|ONREPLY_ROUTE);
	get_tran = find_mod_export("tm", "t_get_tran", 2, ONREPLY_ROUTE);
	save_stir_details_to_msg = find_mod_export("tm", "t_save_stir_details_to_msg", 2, ONREPLY_ROUTE);

        if (!_get_agentid) {
                LOG(L_ERR, "[mod_init]: Exporting get_agentid from wsutils failed \n");
                return -1;
        }

	PARSE_IP_LIST(wsregips,ws_registrar)
	PARSE_IP_LIST(wsicpips,ws_icpips)
	PARSE_IP_LIST(mediaips,mediaserver_ips)

	PARSE_IP_LIST(msuris,ms_uris)
	PARSE_IP_LIST(omsuris,oms_uris)
	PARSE_IP_LIST(acduris,acd_uris)
	
	PARSE_IP_LIST(specialnumbers,special_numbers)
	
	if (( shmid = shmget(IPC_PRIVATE, sizeof(WSServerState), 0666|IPC_CREAT)) < 0) {
	        LOG(L_ERR , "Cann't create Shared Memory  %s\n\n" , strerror(errno));
		return -1;
	} 
	shm_key = ftok("/tmp/fgrp_shmdetails",0x23ABCE);
	if(shm_key == -1) {
		LOG(L_ERR , "[worksmart:mod_init]Cann't get Key %s\n\n" , strerror(errno));
		return -1;
	}
	if(noofshmsegments <= 0){
		noofshmsegments = 1000;
		LOG(L_ERR,"[worksmart:mod_init] did not get value for noofshmsegments from configuration so assigning default value as 1000 \n");
	}
	/* Added for clearing stuck   fgrp shmid  before restart or crash */
	if (( shm_clear_id = shmget(shm_key, SHMSEGMENTSIZE_PREV , 0666|IPC_CREAT|IPC_EXCL)) < 0) {
		LOG(L_ERR , "[worksmart:mod_init] Cann't create Shared Memory  %s ,might be segment already exists or some error \n\n" , strerror(errno));
		if (( shm_clear_id = shmget(shm_key, SHMSEGMENTSIZE_PREV , 0666|IPC_CREAT)) < 0) {
			LOG(L_ERR , "[worksmart:mod_init] Cann't create Shared Memory  %s\n\n" , strerror(errno));
		}else {
			if ((shmids_ptr  = (SHMIDS *)shmat (shm_clear_id, (char *) 0, 0)) == ( SHMIDS *) -1) {
				LOG(L_ERR , "[worksmart:mod_init] Cann't attach Shared Memory\n\n");
			}else {
				shmid_clear_onstartup();
			}
		}
	}
	
	shmdt(&shm_clear_id);
	shmctl(shm_clear_id,IPC_RMID,NULL);
	shm_clear_id = 0;
	shmids_ptr = NULL;

	/* Added for inserting fgrp shmid into shared memory */
	if (( shm_clear_id = shmget(shm_key, SHMSEGMENTSIZE , 0666|IPC_CREAT)) < 0) {
		LOG(L_ERR , "[worksmart:mod_init] Cann't create Shared Memory  %s \n\n" , strerror(errno));
		return -1;
	}
		
	if ((shmids_ptr  = (SHMIDS *)shmat (shm_clear_id, (char *) 0, 0)) == ( SHMIDS *) -1) {
			LOG(L_ERR , "[worksmart:mod_init] Cann't attach Shared Memory\n\n");
			return  -1;
	}
	memset(shmids_ptr, 0, SHMSEGMENTSIZE);
	
	sem_key = (char *) shmids_ptr+(sizeof(int)*2);
	if(sem_init(sem_key,1,1) == -1){ /* initilizing semaphore */
		LOG(L_ERR,"[worksmart:mod_init] ERROR: semaphore Inititilization failed\n");
		return -1;
	}
	
	LOG(L_ERR,"[worksmart:mod_init] Attached SHM for FGRP Nodes, id:%d shm_ptr:%p units:%d \n", shm_clear_id, shmids_ptr, noofshmsegments);

	/*!
	 * Project: CNAM
	 * Date:18-11-2009
	 * Usage :Load tm module to send Publish Method--CNAM**/
	loader_tm  = (load_tm_f )find_export ("load_tm",NO_SCRIPT , 0  ) ;
	if (loader_tm == NULL){
			LOG(L_WARN , "[mod_init] Loading Module failed\n");
			return -1;
	}
	loader_tm ( &_tmb );
	/*----------------*/

	return 0;
}

/*Added Unique Token generation changes for fixing duplicate tokens across services/processes --Ajay/Saidulu */
/*
 *	For Unsigned INT Number range												*
 * __________________________________________									*
 * |  F  |  F  |  F |  F |  F |  F |  F |  F  | -> 4bytes of unsigned int range  *
 * | 0-7| 0-31|   0 - 0x1FFFF       |  0-127  |								    *
 * | sid| pid |    Seconds range    |calls/sec|  } -> unit wise Ranges			*
 * ------------------------------------------- 								    *
 */


void generate_unique_process_range(int Rank)
{
	int id = 0;
	id |= (serviceid % SERVICEID_RANGE);
	id = id << PROCESS_BITS_LEN;
	id |= (Rank % PROCESS_COUNT);
	uniq_process_id = id << PROCESS_RANGE_BITS_LEN ;

	srand(getpid());
	cps = rand(); /* Getting random value for startup, instead of zero */

	LOG(L_ERR, "[generate_unique_process_range] Rank: %d Rem:%d service_id: %d id:%d uniq_range:%u cps:%u \n", 
							Rank, (Rank % PROCESS_COUNT), serviceid, id, uniq_process_id, cps);
}

unsigned generate_unique_token(void)
{
	unsigned max_int =  0xFFFFFFFF; 
	unsigned res = 0, cur_cps = 0;
	unsigned rand = time(0) & seconds_range;
	rand = rand << cps_bits_len;
	cur_cps = (cps++ & max_cps);
	if(!cur_cps){
		cps++;
		cur_cps= 1;
	}
	
	rand |= cur_cps;
	res = uniq_process_id | (rand & PROCESS_TOKEN_RANGE);

	if(res > max_int){ /* This should not happen */
		LOG(L_ERR,"[generate_unique_token]ERROR: somthing wrong in token:%x Limit:%x \n", res, max_int);
		res = time(0) % max_int;
	}
	
	return res;
}

/* END */
int isInList(ip_list list, char *  source ) {
	int count = 0;
	
	while ( list[count][0] != '\0'){
		if ( !strcmp(list[count], source) ) {
			return 1;
		}
		count++;
	}
	return 0;
}

static int f_clear_forkgroup_shared_memory(struct sip_msg *msg , char *cToken, char *cBuffer) {

	int i = 0,count = 0, nKey = 0, shmid = 0, nRows=0;
	char *ptr = NULL;

	typedef struct {
		int nToken;
		volatile int nMaxForksInGrp;
		int nCount;
		short nProcessed[nRows];
		int nRingSecs[nRows];
		int nPhoneType[nRows];
		int nFwdflag[nRows];
		char cAgentid[nRows][256];
		char cUserName[nRows][256];
		char cGrp_Title[nRows][256];
	} WSHGroup;

	SHMIDS * list= NULL;
	i = SHM_OFFSET + sizeof(SHMIDS);
	list = (char *)shmids_ptr + i;

	if((ptr = strstr(cToken,"wspib"))){
		ptr = ptr+5;
		if(ptr) {
			nKey = atoi(ptr);
		}
	}else{
		nKey = atoi(cToken);
	}

	if(nKey == 0) {
		LOG(L_ERR , "[f_clear_forkgroup_shared_memory] nKey is zero !!!\n");
		return -1;
	}
	
	if((shmid = shmget(nKey, sizeof(WSHGroup), 0666)) < 0) {
		LOG(L_ERR , "[f_clear_forkgroup_shared_memory] Cann't get Shared Memory Token: %s:%d Error: %s \n\n",cToken,nKey,strerror(errno));
		return -1;
	}

	while (count < noofshmsegments-2){
		if(list && list->shmid == shmid && (list->in_use == 1)){
			LOG(L_ERR,"[f_clear_forkgroup_shared_memory] Removing Node for SHMID: %d \n",list->shmid);
			list->in_use = -1;
			shmctl(list->shmid,IPC_RMID,NULL);
			list->shmid = 0;
			list->in_use = 0;
			break;
		}
		list+=1;
		count++;
	}
	return 1;
}

static int isinlist_f(struct sip_msg *msg, char * type, char * srcipport )
{
	char packetipport[32];
	//int  recv_port=0;
	int ret = 0; 
	if( srcipport == NULL  ){
		memset(packetipport,0,sizeof(packetipport));
		ret = snprintf(packetipport, sizeof(packetipport)-1,"%s:%d", ip_addr2a(&msg->rcv.src_ip), msg->rcv.src_port);
		if(ret > sizeof(packetipport)){
			return -1;
		}
	}else {
		memset(packetipport,0,sizeof(packetipport));
		snprintf(packetipport,  sizeof(packetipport)-1, "%s" , srcipport);		
		if( ret  > sizeof(packetipport)){
			return -1;
		}
	}
	ret = atoi(type);

	switch(ret) {
		case 1:		
			if ((isInList(wsregips, packetipport))){
				Source = Registrars; 	
				return 1;
			}
		break ;
		case 2:
			if ((isInList(wsicpips, packetipport ))){
				Source = OtherSource; 	
				return 1;
			}
		break ;
		case 3:
			if ((isInList(wsicpips,packetipport ))){
				Source = OtherSource; 	
				return 1;
			}
			if ((isInList(wsregips, packetipport ))){
				Source = Registrars; 	
				return 1;
			}
		break ;
		case 4:
			if ((isInList(mediaips, packetipport ))){
				media_failover = 1;
				return 1;
			}
			break ;
							
	}
	
	return -1;
}


static enum CallSource get_source(struct sip_msg * msg ) {

	if( -1 != parse_headers(msg ,HDR_USERAGENT,0))
	{
		if(msg->user_agent && msg->user_agent->body.len>0) 
		{
			LOG(L_ERR  ,  "[get_source] '%.*s'  '%s'\n", msg->user_agent->body.len , msg->user_agent->body.s, acduseragent);
			if(!strncmp(msg->user_agent->body.s, acduseragent,  msg->user_agent->body.len))
			{
				LOG(L_ERR  ,  "[get_source] returning ACD Server \n");
				//return ACDServer;
				return MediaServer;
			}else if(!strncmp(msg->user_agent->body.s, msuseragent,  msg->user_agent->body.len))
			{
				LOG(L_ERR  ,  "[get_source] returning  MEDIA Server \n");
				return MediaServer;
			}
		}else{

			LOG(L_ERR , "[get_source]There is noting in useragent\n");
		}
	}
	LOG(L_ERR , "[get_source] returning other source :4:\n");
	return OtherSource;
}

int route_to_outboundserver1( struct  sip_msg * msg,char *phone,struct ws_uri *wsuri,char *moc_agent){
	char  buffer[320]="";
	if(wsuri->context){
		bzero(buffer,sizeof(buffer));
		if(moc_agent == NULL)
			moc_agent = wsuri->group;
		snprintf(buffer, sizeof(buffer)-1, "oms-%s-%s-%s-%s", phone, moc_agent,wsuri->context  , wsuri->token);
	
		LOG(L_ERR, "[route_to_outboundserver1] buffer : %s \n",buffer);

	}else{
    		return route_to_outboundserver(msg, phone);
	}

	ws_set_iattr( msg, 90 );
	

	if(WSState->nMediaServerType == PRIMARY){
	        LOG(L_WARN,"[route_to_outboundserver1]PRIMARY>>>>>>>>>>>>\n");
        	return route_to_server(msg, buffer, umslb_proxy,OMS);
    	}else{
        	LOG(L_WARN,"[route_to_outboundserver1] SECONDARY>>>>>>>>>>>>\n");
        	return route_to_server(msg, buffer, umslb_proxy1,OMS);
    	}
}

int route_to_outboundserver( struct  sip_msg * msg  ,  char *phone){
    char  buffer[320]="";
	struct sip_uri puri;
	char *sitename;
	char fromuser[100];
	char time_s[32]="";
	str fromuri;
	unsigned uniq_value = 0;

	if(!msg){
		LOG(L_ERR , "[route_to_outboundserver] Msg structure is NULL !!!!!\n");
		return -1;
	}
	bzero (time_s, sizeof (time_s));
	uniq_value = generate_unique_token();
	snprintf (time_s, sizeof (time_s) - 1 , "%010u",uniq_value);
	
	if (parse_from_header(msg) == -1) {
			LOG (L_ERR, "Oh! Failed parsing from header\n");
			return -1;
	}
	
	bzero(fromuser,sizeof(fromuser));
			
	fromuri = get_from(msg)->uri;
		
	if (parse_uri(fromuri.s, fromuri.len, &puri) < 0) {
		snprintf(buffer, sizeof(buffer)-1, "%.*s", fromuri.len, fromuri.s);
		LOG (L_ERR, "Huh? Call without a From header?!\n");
		return -1;
	}
	snprintf(fromuser, sizeof(fromuser)-1, "%.*s", puri.user.len, puri.user.s);
	LOG(L_WARN, "[route_to_outboundserver]From Header: %s\n", fromuser);
	sitename = strchr(fromuser,'-');
	if(sitename){
		sitename++;
	}
	LOG(L_ERR, "Sitename: %s\n", sitename);
	
	ws_set_iattr( msg, 90 );
	LOG(L_ERR, "[route_to_outboundserver] sitename:%s.................... \n",sitename);
	bzero(buffer,sizeof(buffer));
    	snprintf(buffer, sizeof(buffer)-1, "oms-%s-0-%s-%s", phone, sitename  , time_s);
	if(WSState->nMediaServerType == PRIMARY){
        	LOG(L_WARN,"[route_to_outboundserver] PRIMARY>>>>>>>>>>>>\n");
        	return route_to_server(msg, buffer, umslb_proxy,OMS);
    	}else{
        	LOG(L_WARN,"[route_to_outboundserver] SECONDARY>>>>>>>>>>>>\n");
        	return route_to_server(msg, buffer, umslb_proxy1,OMS);
    	}
}


int route_to_pstn (struct sip_msg *msg, char * phone) {
	ws_set_iattr( msg, 90 );
    return route_to_outboundserver(msg, phone);
}

int route_to_media_server(struct sip_msg *msg, char * phone, char * cCalleridBlock)
{
	char *tmp = NULL;
	char *tp = NULL;
	char tcontact[256];
	char fulluri[256];

	LOG(L_WARN,"[route_to_media_server]warmflag:%d\n",warmflag);
	LOG(L_ERR,"[route_to_media_server] phone '%s' \n", phone );
	if ( ( (get_source(msg) == MediaServer) && (warmflag == 0) ) )
	{
		if (parse_headers(msg, HDR_CONTACT, 0) == -1)
		{
			LOG(L_WARN,"[route_to_media_server] Unable to parse Contact HDR \n");
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}

		if (!msg->contact)
		{
			LOG(L_ERR,"[route_to_media_server] No Contact Found\n");
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}
		
		if (msg->contact->body.len > 0)
		{
			snprintf(tcontact, sizeof(tcontact)-1, "%.*s", msg->contact->body.len, msg->contact->body.s);
			tcontact[msg->contact->body.len] = '\0';
		}
		
		tmp = strchr(tcontact , '@');
		if (tmp == NULL || ++tmp == NULL)
		{
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}

		if(tmp){
			tp = strchr(tmp , '>');
			if(tp){
				LOG(L_ERR,"[route_to_media_server] Removing '>' in the actual contact as we add later\n");
				*tp = '\0';
			}
		}

		memset(fulluri, 0, sizeof(fulluri));
		snprintf(fulluri, sizeof(fulluri)-1/*255*/, "Contact: <sip:%s@%s>\r\n", phone, tmp);
		
		LOG(L_ERR,"[route_to_media_server] Changed contact: %s\n",fulluri);
		
		_nNoRelay = 1;
		if (add_lump_rpl(msg, fulluri, strlen(fulluri), LUMP_RPL_HDR) == 0)
		{
			send_reply(msg, 500, "Internal Server Error, Hint:302-1");
			LOG(L_ERR,"[route_to_media_server] ERROR:append_to_reply : unable to add lump_rl\n");
			return 1;
		}

		memset(fulluri, 0, sizeof(fulluri));
		if(msg->nRejectRelay > 0 && strlen(msg->cRejectReason) > 0){
			memset(fulluri, 0, sizeof(fulluri));
			snprintf(fulluri, sizeof(fulluri)-1/*255*/, "X-Reject-Reason: %d;%s\r\n", msg->nRejectRelay, msg->cRejectReason);
			add_lump_rpl(msg, fulluri, strlen(fulluri), LUMP_RPL_HDR);
		}
		
		/*send_reply(msg, 302, "Moved temporarily");*/
		/*This change for 404 looping from ICP to Media server!*/
		if (_nOrigSeq == 1)
		{
			LOG(L_ERR,"Moved Temporarly 1!\n");
			send_reply(msg, 302, "Moved temporarily");
		}
		else
		{
			LOG(L_ERR,"Moved Temporarly 2!\n");
			ws_t_reply(msg,(char *)302,"Moved temporarily");	
		}

		/* VMIP Billing issue fix - Vijay 07052009 */		
//		if (!strncmp(phone, "cnf-", 4))
		{
			if (racc_302)
			{
				racc_302(msg, phone, cCalleridBlock);
			}
		}
		return 1;
	}
	
	warmflag = 0;
	LOG(L_ERR,"warmflag is '%d'\n",warmflag);
	if (WSState->nMediaServerType == PRIMARY)
	{
		return route_to_server(msg, phone, umslb_proxy,MS);
	}
	else
	{
		return route_to_server(msg, phone, umslb_proxy1,MS);
	}
}
/*!
 * Project: CNAM
 * Date:18-11-2009
 * Usage:Check callerid is phonenumber
 * */

int isphnum(char *string, int len){

	int i;
	/*Skip '+' if there*/
	if((*string == '+')){
			string++;
			len--;
	}

	for(i=0; i < len;i++){
			if(!isdigit(string[i])){
					return 0;//caller id is not number
			}
	}

	return 1;//Callerid is number
}
/* !
 * Project : CNAM
 * Date :18-11-2009
 * Usage : Send Phonenumber to 3pcc Server to do CNAM lookup
 * 	1.Parse the From URI in INVITE message and get the callerid
 * 	2.Validate URI
 * 	3.Check Local DB for CNAM
 * 	4.If not available in local DB send PUBLISH method to 3pcc Server
 * 		
 */

int ws_cnam_request(struct sip_msg *msg,int siteid){

		int cid_len = 0 ;

		char cnam_query[512]="";
		char temp_cid[128]="";
		char callerid[128]="";

		char *temp = NULL;
		char *cid = NULL;

		str uri = {NULL ,0};

		db_res_t *res=NULL;
		
		/*Queries to execute*/
		char *cnam_enablequery ="SELECT 1 FROM siteinfo_new where siteid='%d' AND cnam=1 ";
		if(!msg || !msg->from){
			LOG(L_ERR , "[ws_cnam_request] From header field missing !!!!  \n");
			return -1;
		}
		
		/*Parse FROM URI to get only Callerid and do validation */
		uri = get_from(msg)->uri;

		if(!(uri.s && uri.len)){
				LOG(L_ERR,"[ws_cnam_request][Error] From URI is NULL or Empty\n");	
				return -1;
		}

		LOG(L_WARN,"[ws_cnam_request]From URI:%.*s\n",uri.len,uri.s);	

		if((strncmp(uri.s,"sip:",4))){
				LOG(L_ERR,"[ws_cnam_request][Error] Parsing From URI failed Doesnt contain 'sip:' URI:%s\n",uri.s);	
				return -1;
		}

			
		cid = uri.s;
		cid += 4;//Point cid to callerid

		if(!(temp = strchr(cid,'@')) || (cid == temp)){
				LOG(L_ERR,"[ws_cnam_request][Error] whie parsing from URI:%s , Doesnt contain '@' or Callerid ",uri.s);	
				return -1;
		}
		
		cid_len=temp-cid;


		if((cid_len >  MAXPSTN_DIGITS) || (cid_len < MINPSTN_DIGITS)){
				LOG(L_ERR,"[ws_cnam_request][Error]Callerid length > 15 OR < 10 cid_len:%d",cid_len);	
				return -1;		
		}

		if(!isphnum(cid,cid_len)){
				LOG(L_ERR,"[ws_cnam_request][Error] Callerid is not number\n");	
				return -1;
		}
		
		snprintf(temp_cid, sizeof(temp_cid)-1, "%.*s", cid_len, cid); //copy callerid to temp buffer
		cid = temp_cid;
		
		/*Check format of phonenumber*/
		switch(*cid){
			/*Process the phonenumber starting with '+'*/
			case '+':
					cid++;
					if((*cid) != '1'){
							LOG(L_ERR,"[ws_cnam_request][Error]Callerid is not a valid  USA phonenumber:%s\n",temp_cid);
							return -1;
					}
					
					cid++;
					cid_len-=2;
					
					
					break;
			/*Process the phonenumber starting with '1'*/		
			case '1':
					
					cid++;
					cid_len--;
					break;
			/*Skip the phonenumbers starting with '0'*/		
			case '0':
					LOG(L_ERR,"[ws_cnam_request][Error]Callerid International number starting with '0' cid:%s\n",cid);	
					return -1;
					
			default:
					
					break;

		}			
		if(cid_len != MINPSTN_DIGITS){
				LOG(L_ERR,"[ws_cnam_request][Error]Callerid length After skiping codes is  not equal to 10 cid:%s\n",cid);	
				return -1;	
		}

		snprintf(callerid, sizeof(callerid)-1, "%.*s", cid_len, cid); //copy the MINPSTN_DIGITS phone number to buffer


		/*Check CNAM feature is enbled for this siteid*/
		memset(cnam_query,0,sizeof(cnam_query));
		snprintf(cnam_query,sizeof(cnam_query)-1,cnam_enablequery,siteid);

		LOG(L_WARN,"[ws_cnam_request]cnam enable query:%s\n",cnam_query);

		ws_dbf.raw_query (h, cnam_query, &res);
		
		if(res == NULL){
				LOG(L_WARN,"[ws_cnam_request]CNAM enable query 'res' is NULL\n");
				return -1;
		}
		
		if (RES_ROW_N(res) > 0) {
				LOG(L_WARN,"[ws_cnam_request]CNAM Feature is Enabled for this siteid:%d",siteid);	
				ws_dbf.free_result (h, res);
		}else{
				LOG(L_WARN,"[ws_cnam_request]CNAM Feature is disabled for this siteid:%d",siteid);	
				ws_dbf.free_result (h, res);
				return 1;
		}
		
		
		/*Sending Publish method to 3pcc server*/
		if(send_publish_message(callerid,siteid,threepcc_server,own_ip)){
				LOG(L_WARN,"[ws_cnam_request]Sending PUBLISH failed\n");
				return 0;
		}
		return 0;
}


int route_to_acd_server(struct sip_msg *msg, char *phone, char *realdid, int nType, int nFlag)
{
	char *tmp = NULL;
	char tcontact[256];
	char fulluri[256] = "", cHeader[64] = "";

	bzero (tcontact, sizeof (tcontact));	
	bzero (fulluri, sizeof (fulluri));	
	bzero (cHeader, sizeof (cHeader));	

	if (get_source(msg) == MediaServer && ((nType == IVR_Command) || (nType == ACD_DID) || (nType == PSTN_Transfer)))
	{
		if (parse_headers (msg, HDR_CONTACT, 0) == -1)
		{
			LOG(L_WARN,"[route_to_acd_server] Unable to parse Contact HDR \n");
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}

		if (!msg->contact)
		{
			LOG(L_WARN,"[route_to_acd_server] No Contact Found\n");
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}

		if(msg->contact->body.len > 0)
		{
			snprintf(tcontact, sizeof(tcontact)-1, "%.*s", msg->contact->body.len,msg->contact->body.s);
			tcontact[msg->contact->body.len] = '\0';
		}

		tmp = strchr(tcontact , '@');
		if (tmp == NULL || ++tmp == NULL)
		{
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}

		memset(fulluri, 0, sizeof(fulluri));
		snprintf(fulluri, sizeof(fulluri)-1/*255*/, "Contact: <sip:%s@%s\r\n", phone, tmp);

		_nNoRelay = 1;
		if (add_lump_rpl(msg, fulluri, strlen(fulluri), LUMP_RPL_HDR) == 0)
		{
			send_reply(msg, 500, "Internal Server Error, Hint:302-1");
			LOG(L_ERR,"[route_to_acd_server] ERROR:append_to_reply : unable to add lump_rl\n");
			return 1;
		}
		
		if(realdid && strlen(realdid) >= 10){
			memset(fulluri , 0 , sizeof(fulluri));
			snprintf(fulluri , sizeof(fulluri)-1 , "WSRnumber: %s\r\n" , realdid);

			if (add_lump_rpl(msg, fulluri, strlen(fulluri), LUMP_RPL_HDR) == 0)
			{
				LOG(L_ERR,"[route_to_acd_server] ERROR:unable to add header WSRnumber\n");
			}
		}

		if((nFlag == 1) && (!is_header_exists(msg,"X-Noreplookup",13,cHeader,sizeof(cHeader)))){ /* Added for ACD overflow cases */
			memset(cHeader,0,sizeof(cHeader));
			snprintf(cHeader, sizeof(cHeader)-1,"X-Noreplookup: Yes\r\n");
			if (add_lump_rpl(msg, cHeader, strlen(cHeader), LUMP_RPL_HDR) == 0)
			{
				LOG(L_ERR,"[route_to_acd_server] ERROR:unable to add header WSRnumber\n");
			}
		}	

		if (racc_302 && phone && strlen(phone) > 0 && (!strncmp(phone,"opr-",4) || !strncmp(phone,"ivr-",4) || !strncmp(phone,"nop-",4))){
			racc_302(msg, phone, NULL);/*Added changes for get ivr details in cdr's*/
		}

		send_reply(msg, 302, "Moved temporarily");
		return 1;
	}

	if((nFlag == 1) && (!is_header_exists(msg,"X-Noreplookup",13,cHeader,sizeof(cHeader)))){ /* Added for ACD overflow cases */
		memset(cHeader,0,sizeof(cHeader));
		snprintf(cHeader, sizeof(cHeader)-1,"X-Noreplookup: Yes\r\n");
		add_header(msg,cHeader,strlen(cHeader));
	}

	if (WSState->nMediaServerType == PRIMARY)
	{
		LOG(L_ERR,"[route_to_acd_server] PRIMARY>>>>>>>>>>>>\n");
		return route_to_server(msg, phone, umslb_proxy,ACDMS);
	}
	else
	{
		LOG(L_ERR,"[route_to_acd_server] SECONDARY>>>>>>>>>>>>\n");
		return route_to_server(msg, phone, umslb_proxy1,ACDMS);
	}
}

/* Setting call pickup uri after parking */
int worksmart_set_pickup_extension (struct sip_msg *msg, char *p1, char *p2,char *token)
{
	str uri, fm_uri;
	struct sip_uri parsed_uri;
	short len;
	char tmp1[50], tmp2[50], pickup_uri[100], *s, *ms;

	if(!msg){
		LOG(L_ERR,"[ws_set_pickup_extension]: Msg structure is NULL\n");
		return -1;
	}
	uri = msg->first_line.u.request.uri;
	if (parse_uri (uri.s, uri.len, &parsed_uri) < 0 ) {
		LOG (L_WARN, "[ws_set_pickup_extension]: Failed to parse uri\n");
		return -1;
	}
	else {
		bzero (tmp1, sizeof (tmp1));	
		snprintf(tmp1, sizeof(tmp1)-1, "%.*s", parsed_uri.user.len, parsed_uri.user.s);
	}

	if (-1 == (parse_from_header (msg))) {
		LOG (L_WARN, "[ws_set_pickup_extension]: Failed to parse from header\n");
		return -1;	
	}	
	fm_uri = get_from (msg)->uri;
	if( fm_uri.s == NULL || fm_uri.len == 0 ){
			LOG(L_ERR , "[ws_set_pickup_extension] Some thing wrong in From header\n");
			return -1;
	}

	if (parse_uri (fm_uri.s, fm_uri.len, &parsed_uri)) {
		LOG (L_WARN, "[ws_set_pickup_extension]: Failed to parse from uri\n");	
		return -1;
	}
		
	if( parsed_uri.user.s == NULL   ||  parsed_uri.user.len == 0 ){
		LOG(L_ERR , "[ws_set_pickup_extension] Some thing wrong in From header\n");
		return -1;
	}

	if ((! strncmp (parsed_uri.user.s, "ext-", 4)) || (! strncmp (parsed_uri.user.s, "cmd-", 4))) {
		LOG (L_WARN, "[ws_set_pickup_extension]: We dont process ext/cmd calls for parking\n");	
		return -1;	
	}
	bzero (tmp2, sizeof (tmp2));
	snprintf(tmp2, sizeof(tmp2)-1, "%.*s", parsed_uri.user.len, parsed_uri.user.s);

	s = strchr (parsed_uri.user.s, '-');
	
	bzero (tmp2, sizeof (tmp2));
	memccpy (tmp2, s+1, '@', sizeof (tmp2));

	len = strlen (tmp2);
	if (len) tmp2[len-1] = 0x00;

	if(WSState->nMediaServerType == PRIMARY){
		ms = umslb_proxy;
	}else{
		ms = umslb_proxy1;
	}
	
	if ('0' != tmp2[0]) {
		bzero (pickup_uri, sizeof (pickup_uri));
		snprintf (pickup_uri, sizeof (pickup_uri)-1, "sip:pkp-99%s-0-%s-%s@%s", tmp1, tmp2,token, ms);
	}
	else
		return  -1;

	/* Set the newly formed URI */
    uri.s = pkg_malloc (strlen (pickup_uri) + 1);
	if (NULL == uri.s)  {
		send_reply(msg, 500, "Internal Server Error, Hint : pkp-1");
		_nNoRelay = 1;
		return 1;
	}
	
    bzero (uri.s, strlen (pickup_uri) + 1);
	strncpy (uri.s, pickup_uri, strlen (pickup_uri));
	uri.len = strlen (pickup_uri);
		
	if (msg->new_uri.s)	pkg_free (msg->new_uri.s);
	msg->parsed_uri_ok = 1;
	msg->new_uri = uri;
	
	LOG (L_WARN, "[ws_set_pickup_extension]: uri is: %s\n", msg->new_uri.s);
 
	return 1;
}


/* Getting the context */
int worksmart_route_to_ivr (struct sip_msg *msg, struct PstnPhone stPhone) {
	char time_s [32];
	char query[4096];
	char result[128];
	db_res_t *res=NULL;
	db_row_t row;
	unsigned uniq_value = 0;
	char * formatstring = ""
	    "SELECT STRAIGHT_JOIN concat(concat(concat(concat(concat(concat(concat('ivr-', specialid), '-'), greetingid), '-'), REPLACE(sitename,'.','-')), '-'), '%s') "
		"  FROM siteinfo_new s, special sp, groupgreetings g "
		" WHERE s.siteid = sp.siteid "
		"   AND sp.siteid = g.siteid "
		"   AND g.siteid = %d "       
		"   AND sp.code = g.specialid "
		"   AND g.specialid = %d "
		"   AND ( (localfrom > localto AND ( time(now()) BETWEEN localfrom AND time('23:59:59') OR time(now()) BETWEEN '00:00:00' AND localto)) OR ( time(now()) BETWEEN localfrom AND localto ) ) "
		"   AND g.specialid NOT IN ( SELECT code FROM greetingmasks gm WHERE ( (date(now()) BETWEEN localfrom AND localto AND mtype = 1) "
		"    OR ((date(convert_tz(now(),'%s',(SELECT tz.tdiff FROM greetingmasks gm, timezonenames tz WHERE gm.timezone = tz.tid AND siteid=%d AND date(convert_tz(now(),'%s',tz.tdiff))=fromdate AND mtype=2 limit 1))) = fromdate AND periodic = 0 ) "
		"    OR ( dayofweek(convert_tz(now(),'%s',(SELECT tz.tdiff FROM greetingmasks gm, timezonenames tz WHERE gm.timezone = tz.tid AND siteid=%d AND date(convert_tz(now(),'%s',tz.tdiff))=fromdate AND mtype=2 limit 1)))=dayofweek(fromdate) AND periodic = 1) AND mtype = 2 ) ) AND gm.code = g.specialid AND gm.siteid   =  %d ) "
		" UNION "
		"SELECT STRAIGHT_JOIN concat(concat(concat(concat(concat(concat(concat('ivr-', sp.code), '-'), greetingid), '-'), REPLACE(sitename,'.','-')), '-'), '%s') "
		"  FROM siteinfo_new s, special sp, greetingmasks gm "
		" WHERE s.siteid = sp.siteid "    
		"   AND sp.siteid = gm.siteid "
		"   AND gm.siteid = %d "  
		"   AND sp.code = gm.code "
		"   AND gm.code = %d "
		"   AND ( ( date(now()) BETWEEN localfrom and localto AND periodic =0 ) OR ( ( ( day(now()) BETWEEN day(localfrom) AND day(localto)) AND month(now()) = month(localfrom))  AND periodic =1) ) " 
		"   AND mtype = 1 "
		" UNION "
		"SELECT STRAIGHT_JOIN concat(concat(concat(concat(concat(concat(concat('ivr-', sp.code), '-'), greetingid), '-'), REPLACE(sitename,'.','-')), '-'), '%s')  " 
		"  FROM siteinfo_new s, special sp, greetingmasks gm  "
		" WHERE s.siteid = sp.siteid   " 
		"   AND sp.siteid = gm.siteid "
		"   AND gm.siteid = %d "
		"   AND sp.code = gm.code "
		"   AND gm.code = %d  "
		"   AND (( date(convert_tz(now(),'%s',(SELECT tz.tdiff FROM greetingmasks gm, timezonenames tz WHERE gm.timezone = tz.tid AND siteid=%d AND date(convert_tz(now(),'%s',tz.tdiff))=fromdate AND mtype=2 limit 1))) = fromdate AND periodic = 0 ) "
		"    OR ( dayofweek(convert_tz(now(),'%s',(SELECT tz.tdiff FROM greetingmasks gm, timezonenames tz WHERE gm.timezone = tz.tid AND siteid=%d AND date(convert_tz(now(),'%s',tz.tdiff))=fromdate AND mtype=2 limit 1)))=dayofweek(fromdate) AND periodic = 1)) "   
		"   AND mtype = 2 ";
	bzero (time_s, sizeof (time_s));
	uniq_value = generate_unique_token();
	snprintf (time_s, sizeof (time_s) - 1, "%010u", uniq_value);
	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1 , formatstring, time_s, stPhone.nSiteid, stPhone.nAssignedID, servertz, stPhone.nSiteid, servertz, servertz, stPhone.nSiteid, servertz,  stPhone.nSiteid, time_s, stPhone.nSiteid, stPhone.nAssignedID, time_s, stPhone.nSiteid, stPhone.nAssignedID, servertz,  stPhone.nSiteid,servertz, servertz, stPhone.nSiteid, servertz );

	//LOG(L_ERR, "[worksmart_route_to_ivr]Query:\n %s\n", query);
	ws_dbf.raw_query (h, query, &res);
	if(res==NULL){
		LOG(L_WARN,"[worksmart_route_to_ivr]'res' is NULL");
		return -1;
	}
	if (RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		bzero (result, sizeof (result));	
		strncpy(result,row.values[0].val.string_val,sizeof(result)-1); 
		//Check if the call is coming from IVR system.
		// If it is from media server, we should send 302 otherwise, send a regular INVITE.
		LOG(L_WARN, "[worksmart_route_to_ivr] %s", result);
		if (res)
			ws_dbf.free_result (h, res);
		return route_to_media_server(msg, result,"");
//		return route_to_server(msg, result, media_server);
	}
	else {
		// greeting not found for a group.
	    if (res)
    	  ws_dbf.free_result (h, res);
	    formatstring = ""
    	    "SELECT STRAIGHT_JOIN concat(concat(concat(concat(concat(concat(concat('ivr-', %d), '-'), 1), '-'), REPLACE(sitename,'.','-')), '-'), '%s')  "
        	"  FROM siteinfo_new "
	        " WHERE siteid = %d ";
    	LOG(L_ERR, "[worksmart_route_to_ivr] No greeting found\n");
	    /* Play the Default greeting file  25/01/06 -Ramu*/
		memset(query,0,sizeof(query));
	    snprintf(query, sizeof(query)-1, formatstring, stPhone.nAssignedID, time_s, stPhone.nSiteid );
	    //LOG(L_ERR, "[worksmart_route_to_ivr]Query:\n %s\n", query);
	if(res==NULL){
		LOG(L_WARN,"[worksmart_route_to_ivr]'res' is NULL");
		return -1;
	}
	    ws_dbf.raw_query (h, query, &res);

	    if (RES_ROW_N(res) > 0) {
		      row = RES_ROWS(res)[0];
		      bzero (result, sizeof (result));
		      strncpy(result,row.values[0].val.string_val,sizeof(result)-1);
		      LOG(L_WARN, "[worksmart_route_to_ivr] %s", result);
		      if (res)
		        ws_dbf.free_result (h, res);
		      return route_to_media_server(msg, result,"");
	    }
		else{
		      /* Here, we don't have other option but it won't happen*/
		      if (res)
			  	ws_dbf.free_result (h, res);
		      send_reply(msg, 404, "No greeting found");
			  _nNoRelay = 1;;
		}
	    return 1;
	}
	return 1;
}

int mandatory_10digit_dial(struct sip_msg *msg, char *phone) {

	char cid[8] = "", query[256] = "";
	int nRet = 0;
	db_res_t *res = NULL;

	if(msg == NULL || phone == NULL) {
		LOG(L_ERR, "Input parameters are empty\n");
		return 0;
	}

	if(strlen(phone) == 11) {
		strncpy(cid, phone+1, 3);
	}else{
		strncpy(cid, phone, 3);
	}

	snprintf(query,sizeof(query)-1,DIAL_10DIGIT_QUERY, cid);

	ws_dbf.raw_query(h,query,&res);
	if(res) {
		if( RES_ROW_N(res) > 0) {
			LOG(L_ERR, "NANPA found in exclude list : %s\n", query);
			nRet = 1;
		}
		ws_dbf.free_result(h,res);
	}

	return nRet;
}

int prepare_contact_header(struct sip_msg *msg, char *cType) {

	char *ptr = NULL;
	char cContactHeader[256] = "", cContactIp[32]="";

	memset(cContactHeader, 0, sizeof(cContactHeader));
	memset(cContactIp, 0, sizeof(cContactIp));

	/* parsing contact header to get server Ip */
	if( parse_headers (msg, HDR_CONTACT, 0) == -1) {
		LOG(L_ERR," [prepare_contact_for_fraud] parsing contact header failed \n");
		return -1;
	}

	if(msg->contact == NULL || msg->contact->body.len == 0 ) {
		LOG(L_ERR," [prepare_contact_for_fraud] contact is null \n");
		return -1;
	}

	snprintf(cContactHeader, sizeof(cContactHeader)-1, "%.*s", msg->contact->body.len, msg->contact->body.s);
	ptr = strchr(cContactHeader,'@');
	if(ptr && ++ptr) {
		strncpy(cContactIp,ptr,sizeof(cContactIp)-1);   // Server IP
	}

	if(strlen(cContactIp) == 0) {
		LOG(L_ERR," [prepare_contact_for_fraud] Ip in contact is empty : cContactHeader %s  \n", cContactHeader);
		return -1;
	}

	memset(cContactHeader, 0, sizeof(cContactHeader));
	snprintf(cContactHeader, sizeof(cContactHeader)-1,"Contact: <sip:%s-0-0-0@%s\r\n", cType, cContactIp);

	if (add_lump_rpl(msg, cContactHeader, strlen(cContactHeader), LUMP_RPL_HDR) == 0) {
		LOG(L_ERR,"[prepare_contact_for_fraud] append_hf failure Contact: %s \n", cContactHeader);
		return -1;
	}

	return 1;
}

int get_real_did (struct sip_msg * msg, char * pPhone,int pphone_len) {

	char buffer[16]="";
	db_res_t *res=NULL;
	memset(buffer,0x00,sizeof(buffer));
	int nLen = normalize(pPhone, buffer, sizeof(buffer),pphone_len);
	
	int nRet = 0;

	if ( nLen == 7 ) {
		char user[64];
		struct sip_uri parsed_uri;
		char * pTemp;
		char callerid[16];
		char query[512];
		str ftag={NULL,0};
		char from_tag[128]="";
		char * formatstring = ""
						"SELECT callerid "
					    "  FROM pbxusers p , subscriber s"
				        " WHERE s.username = '%s'"
				        "   AND p.agentid = s.agentid";

        if (-1 == parse_from_header (msg)) {
            LOG (L_ERR, "[get_real_did] Error parsing from header...\n");
            return 0;
        }

        if (parse_uri (get_from(msg)->uri.s, get_from(msg)->uri.len, &parsed_uri)) {
            LOG (L_ERR, "[get_real_did]: Failed to parse uri\n");
            return 0;
        }
        bzero(user, sizeof(user));
        memcpy(user, parsed_uri.user.s, parsed_uri.user.len);
        pTemp = strstr(user, "-");
        if ( pTemp == NULL ) {
			if(ws_ctccall_flag){
				/* Added fixes from NPA number dialing from CTC calls for CTC ACD SUP changes --vravi/srinivas */
				ftag = get_from(msg)->tag_value;
				if (ftag.s && ftag.len) {
					snprintf(from_tag,sizeof(from_tag)-1,"%.*s",ftag.len,ftag.s);
				}
				if( strstr( from_tag,CTC_TAG) && isdigit(user[1]) && strlen(user) >= 10){
					bzero(pPhone,10);
					if(user[0] == '+' && user+1)
						strncpy(pPhone, user+1, 4);
					else
						strncpy(pPhone, user, 4);
					strcat(pPhone, buffer);
					return strlen(pPhone);
				}else  // He is not a registered user.
					return 0;
			}else  // He is not a registered user.
				return 0;
        }
        if ( pTemp == user ) {
            // Username started with hiphen! strange.
            return 0;
        }

		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1, formatstring, user);
		ws_dbf.raw_query (h, query, &res);
		if(res==NULL){
			LOG(L_ERR,"[get_real_did]'res' is NULL");
			return -1;
		}
		if ( res && RES_ROW_N(res) > 0) {
			db_row_t row = RES_ROWS(res)[0];
			bzero(callerid,sizeof(callerid));
			strncpy(callerid,row.values[0].val.string_val,sizeof(callerid)-1);
			if (res) {
				ws_dbf.free_result (h,res);
			}
		} else if (res) {
			ws_dbf.free_result (h,res);
			return 0;
		}
		bzero(pPhone,10);
		strncpy(pPhone, callerid, 4);
		strcat(pPhone, buffer);
		LOG(L_WARN, "[get_real_did] full number: %s \n", pPhone);
		nRet = mandatory_10digit_dial(msg,pPhone);
		return (nRet==1)?nRet:strlen(pPhone);
	}
	strncpy(pPhone, buffer,pphone_len-1);
	return strlen(pPhone);
}

enum GroupType get_group_type (int nGroup, int nSiteID, char *disasterphonenumber,int disasterph_len) {
	char * formatstring = ""
		"SELECT extendedgroup, acdenabled, enableoperator, disasterflag, disasterphone "
		"  FROM special"
		" WHERE code = %d"
		"   AND siteid = %d";
	char query[1000];
	int nExtended, nACD, nEnableOperator, nDisasterFlag;
	enum GroupType gType = RegularGroup;

	db_res_t *res=NULL;
	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, formatstring, nGroup, nSiteID);
	LOG(L_ERR, "[get_group_type] Query:\n %s\n", query);
	ws_dbf.raw_query (h, query, &res);
	if(res==NULL){
		LOG(L_ERR,"[get_group_type]'res' is NULL");
		return -1;
	}

	if ( RES_ROW_N(res) > 0) {
		db_row_t row;
		row = RES_ROWS(res)[0];
		nExtended = get_db_int(row.values[0]);
		nACD = get_db_int(row.values[1]);
		nEnableOperator = get_db_int(row.values[2]);
		nDisasterFlag = get_db_int(row.values[3]);
		if ( nExtended && nACD) {
			//BUG BUG BUG. A group cannot be both ACD and Extended
			gType = InvalidGroupType;
		} else if ( nExtended ) {
			if ( nEnableOperator ) {
				gType = OperatorRouted;
			} else {
				gType = ExtendedGroup;
			}
		} else if ( nACD ) {
			gType = ACDGroup;
			 LOG(L_WARN, "[get_group_type] nDisasterMode:%d\n",WSState->nDisasterMode);
			if (nDisasterFlag ||  WSState->nDisasterMode ) {
				gType = DisasterACDGroup;
				if(row.values[4].val.string_val && disasterphonenumber) {
					snprintf(disasterphonenumber, disasterph_len-1, "%.*s", strlen(row.values[4].val.string_val),row.values[4].val.string_val);
					LOG(L_WARN,"[get_operator_extension]Disaster Num: '%s' \n",disasterphonenumber);
				}
			}
		} else {
			gType = InvalidGroupType;
		}
	}
	if (res)
	ws_dbf.free_result (h,res);
	return gType;
}
int get_operator_extension(int nGroup, int nSiteid ) {
	char query[512];
	db_res_t *res=NULL;
	db_row_t row;
	int nExtension = 0;
	char * format_string = ""
		"SELECT p.extensions "
		"  FROM special s, pbxusers p"
		" WHERE p.siteid = %d "
		"   AND p.siteid = s.siteid "
		"   AND s.code = %d "
		"   AND p.agentid = s.operator ";
	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, format_string, nSiteid, nGroup);
	ws_dbf.raw_query (h, query, &res);
	if(res==NULL){
		LOG(L_ERR,"[get_operator_extension]'res' is NULL");
		return -1;
	}
//	LOG(L_ERR, "[get_operator_extension] Query:\n %s\n", query);

	if ( RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		nExtension = row.values[0].val.int_val;
	}
	if (res)
		ws_dbf.free_result (h,res);
	return nExtension;
}


int validate_callerid(char * fromnumber, int nSiteid, char *cExtn, int nLen,int fromnum_len)
{
	db_res_t *res=NULL;
	db_row_t row;
	int nRoute = 0;
	char query[512]="", cPhoneNum[64] = "",cBuffer[16] = "";

	if(!fromnumber || !nSiteid) {
		LOG(L_ERR, "[validate_callerid] Received Empty CallerID..\n");
		return 0;
	}

	normalize(fromnumber, cBuffer, sizeof(cBuffer),fromnum_len);
	if(cBuffer[0]=='+') {
		strncpy(cBuffer, &cBuffer[1],sizeof(cBuffer)-1);
	}

	memset(cPhoneNum, 0, sizeof(cPhoneNum));
	if(strlen(cBuffer) == 10) {
		snprintf(cPhoneNum, sizeof(cPhoneNum)-1,"1%s",cBuffer);
	}
	else {
		strncpy(cPhoneNum, cBuffer, sizeof(cPhoneNum)-1);
	}

	memset(query, 0, sizeof(query));
	snprintf(query,sizeof(query)-1,ACCOUNT_BLOCK_QUERY,nSiteid,cPhoneNum);

	ws_dbf.raw_query (h, query, &res);
	if(res && RES_ROW_N(res) > 0){
		row = RES_ROWS(res)[0];
		nRoute = get_db_int(row.values[0]);
		if (RES_ROWS(res)[0].values[1].nul == FALSE && cExtn) {
			strncpy(cExtn,row.values[1].val.string_val,nLen);
		}
	}
	
	if(res)
		ws_dbf.free_result (h,res);
	return nRoute;
}

int route_to_blockdestination(struct sip_msg *msg, char *cExtn, char *cSitename, char *cToken, int nRoute,char *cSix11extensionsitename, char * request_uri)
{
	char ruri_out[256] = "",cVoicemailBuff[256] ="";

	if(!msg || !cExtn || !cSitename || !cToken) {
		LOG(L_WARN, "[route_to_blockdestination] Either of above values are empty..\n");
		return -1;
	}
	
	if(nRoute == VML_FLAG){
		bzero(ruri_out,sizeof(ruri_out));
		snprintf(ruri_out, sizeof(ruri_out)-1, "vml-%s-0-%s-%s",cExtn, cSitename, cToken);
        if((_nEnableCalleridBlockCDR == 1) && racc_missed_call_entry){/* Inserting cdr enteries for dispalying missed cause why call is hanged up here -- Balaji & Badree  [24-06-2019] */
            bzero(cVoicemailBuff,sizeof(cVoicemailBuff));
            snprintf(cVoicemailBuff ,sizeof(cVoicemailBuff )-1,"Voicemail:%s",cSix11extensionsitename);
            racc_missed_call_entry(msg, ruri_out, cVoicemailBuff);
        }
		return route_to_media_server(msg,ruri_out,(_nEnableCalleridBlockCDR == 1)?"CALLERIDBLOCKVOICEMAIL":"");
	}
	else if(nRoute == BUSY_FLAG){
        if( (_nEnableCalleridBlockCDR == 1) && racc_missed_call_entry ){/* Inserting cdr enteries for dispalying missed cause why call is hanged up here -- Balaji & Badree  [24-06-2019] */
            racc_missed_call_entry(msg, request_uri, cSix11extensionsitename);
        }
        if((_nEnableCalleridBlockCDR == 1) && (!strncmp(cToken,"wspib",5))){
            send_reply(msg, 486, "Busy Here");
        }else{
            send_reply(msg, 603, "Declined");
        }
        _nNoRelay = 1;
	}
	return 1;
}

int get_group_plan (struct sip_msg * msg, char * prio, int nGroupId, int nSiteId, char * sitename, char *realdid, char * token , int nType, int nDcid, char *cExt) {
	enum GroupType gType;
	char newuri[128];
	char disasterphonenumber[50]="";
	char * formatstring = ""
		"SELECT STRAIGHT_JOIN DISTINCT 1, greetingid, operatortype"
		"  FROM greetingmasks gm,special sp"
		" WHERE sp.siteid = %d "
		"   AND gm.siteid = sp.siteid "
		"   AND sp.code = gm.code"
		"   AND gm.code = %d"
		"   AND ( ((now()) between localfrom AND localto)"
		"        OR (periodic = 1 AND mtype = 1 AND ((DATE_FORMAT(now(),'%s') BETWEEN DATE_FORMAT(localfrom,'%s') AND DATE_FORMAT(localto,'%s'))"
		" 	OR (date_format(now(),'%s')='12-31' AND YEAR(localfrom) < YEAR(localto) AND time(now()) BETWEEN time(localfrom) AND '23:59:59') OR (date_format(localfrom,'%s')='12-31' AND YEAR(localfrom) < YEAR(localto) AND date_format(now(),'%s')  BETWEEN '01-01 00:00:00' and date_format(localto,'%s'))))"	
		"	OR (    mtype = 2 AND periodic = 1  AND ((DATE_FORMAT(now(), '%w') = DATE_FORMAT(localfrom, '%w')"
		"       AND time(now()) BETWEEN time(localfrom) AND time(localto))"
		" OR (DATE_FORMAT(now(), '%w') = DATE_FORMAT(localfrom, '%w') AND time(now()) BETWEEN time(localfrom) AND '23:59:59') AND DATE(localfrom) < DATE(localto)"
	   	" OR (DATE_FORMAT(now(), '%w') = DATE_FORMAT(localto, '%w') AND time(now()) BETWEEN '00:00:00' AND time(localto)) AND DATE(localfrom) < DATE(localto))))"
		" UNION "
                "SELECT STRAIGHT_JOIN DISTINCT 2, greetingid, operatortype "
                "  FROM groupgreetings gm, special sp"
                " WHERE gm.siteid = sp.siteid "
                "   AND sp.siteid = %d"
                "   AND sp.code = specialid "
                "   AND specialid = %d  "
                "   AND ( (localfrom > localto AND ( time(now()) BETWEEN localfrom AND time('23:59:59') OR time(now()) BETWEEN '00:00:00' AND localto)) OR ( time(now()) BETWEEN localfrom AND localto ) ) "
                "   ORDER BY 1,2 ";
	
	db_res_t *res=NULL;
	char query[2048];
	int nFlag = -1;
	int nGreetingID = 1;
	struct ws_uri wsuri;
	int oprtype=0;
	char cLocationIP[32] = "",cRequestUri[256]="";

	gType = get_group_type(nGroupId, nSiteId, disasterphonenumber,sizeof(disasterphonenumber));
	switch ( gType ) {
		case ACDGroup:
		/*!
		 * Project :CNAM
		 * Date :18-11-2009
		 * Usage: Send CNAM request
		 * * */

 		   	LOG(L_WARN, "[get_group_plan] ACD Group\n");
			if(strncmp(cnam_enable,"yes",3) || ws_cnam_request(msg,nSiteId)){
				LOG(L_ERR, "[get_group_plan] CNAM is not enabled\n");
				memset(newuri,0,sizeof(newuri));
				snprintf(newuri,sizeof(newuri)-1,"acd-0-%d%0*d-%s-%s", nSiteId, nGrpId_Parser, nGroupId, sitename, token);
			}else{
				memset(newuri,0,sizeof(newuri));
				snprintf(newuri,sizeof(newuri)-1, "acdcnam-0-%d%0*d-%s-%s", nSiteId, nGrpId_Parser, nGroupId, sitename, token);
				LOG(L_WARN, "[get_group_plan]CNAM is enabled\n");
			}
			/*-----------*/

			if(enable_dc_transfer && nRouteAcdAndConfCallsToHomeDc && nDcid && nDcid != datacenter_id) {

				memset(cRequestUri,0,sizeof(cRequestUri));
				snprintf(cRequestUri, sizeof(cRequestUri)-1,"WSRequestURI: %s\r\n",newuri);
				add_header(msg,cRequestUri,strlen(cRequestUri));

				GetRemoteDataCenterIP(nOtherDataCenterId,cLocationIP,sizeof(cLocationIP));
				LOG(L_WARN, "[get_group_plan] It is an across DC ACD Call transfer. So routing to home DC WSICP \n");
				/* Setting this flag to avoid extra billing . */
				msg->extra_info.nOtherDataCenterFlag = 1;
				if(strlen(cLocationIP) > 0) {
					return route_to_server(msg, cExt, cLocationIP,MS);
				}
			}
			
			LOG(L_WARN, "[get_group_plan]Routing to acdserver:%s\n",newuri);
			return route_to_acd_server(msg,newuri,realdid,nType,0);
			break;

	case ExtendedGroup:
			LOG(L_ERR, "[get_group_plan] Extended\n");
			// check if the **operator** greeting is present or not -Ramu
			memset(query,0,sizeof(query));
		    	snprintf(query, sizeof(query)-1, formatstring, nSiteId, nGroupId, "%m-%d %H:%i:%s", "%m-%d %H:%i:%s", "%m-%d %H:%i:%s","%m-%d", "%m-%d", "%m-%d %H:%i:%s", "%m-%d %H:%i:%s", nSiteId, nGroupId );
			LOG(L_WARN, "[get_group_plan]Query: \n%s\n", query);
			ws_dbf.raw_query (h, query, &res);
			if(res==NULL){
				LOG(L_ERR,"[get_group_plan]'res' is NULL");
				return -1;
			}
			
			if (RES_ROW_N(res) > 0) {
				nFlag =  get_db_int (RES_ROWS(res)[0].values[0]);
				nGreetingID = get_db_int(RES_ROWS(res)[0].values[1]);
				LOG(L_WARN, "[get_group_plan] nFlag: %d and nGreetingID : %d\n",nFlag, nGreetingID);
				
				if (nGreetingID != 0) {
					// Mask found
					LOG(L_ERR, "[get_group_plan]  ExtendedGroup : Mask Found/Regular IVR\n");
					if (RES_ROWS(res)[0].values[2].nul == FALSE){
                                                        oprtype = get_db_int(RES_ROWS(res)[0].values[2]);
                                        }
                                        if(oprtype == WS_OPRTYPE_GROUP || oprtype == WS_OPRTYPE_USER){
												memset(newuri,0,sizeof(newuri));
                                                snprintf(newuri, sizeof(newuri)-1, "ivr-%d-%d-%s-%s", nGroupId, nGreetingID, sitename, token);
                                        }else{
                                                LOG(L_ERR,"[ExtentedGroup]There is no user or group setting!\n");
												memset(newuri,0,sizeof(newuri));
                                                snprintf(newuri, sizeof(newuri)-1, "nop-%d-%d-%s-%s", nGroupId, nGreetingID, sitename,token);
                                        }

                                        if (res){
                                                ws_dbf.free_result (h, res);
                                        }
                                        return route_to_acd_server(msg,newuri,realdid,nType,0);
				} else {
				  // **operator** greeting found
					LOG(L_WARN, "[get_group_plan]  ExtendedGroup : **operator** greeting found\n"); 	
					if (res) ws_dbf.free_result (h, res);
						return worksmart_route_to_operator2 (msg, nSiteId,nGroupId, sitename,token);
				}
			}
		  	else {			
				LOG(L_WARN, "[get_group_plan]  ExtendedGroup : Default IVR\n");
				if(res) ws_dbf.free_result (h, res);
				memset(newuri,0,sizeof(newuri));				
				snprintf(newuri, sizeof(newuri)-1, "ivr-%d-1-%s-%s", nGroupId, sitename, token);
				return route_to_media_server(msg, newuri,"");
			}
			break;
			
	case OperatorRouted:
			return worksmart_route_to_operator2 (msg, nSiteId,nGroupId, sitename,token);
			break;
	case DisasterACDGroup:
		if (strlen(disasterphonenumber)) {
			memset(&wsuri,0x00,sizeof(wsuri));
			wsuri.context = sitename;
			wsuri.token = token;
			wsuri.group = "0";
			memset(newuri,0,sizeof(newuri));				
			snprintf(newuri, sizeof(newuri)-1, "oms-%s-%s-%s-%s", disasterphonenumber,wsuri.group, sitename,token);
			return route_to_acd_server(msg, newuri,"",nType,1); /* 5th parameter '1' indicates to add 'X-Noreolookup header in ACD overflow cases */
		}
		else {
			memset(newuri,0,sizeof(newuri));				
            snprintf(newuri, sizeof(newuri)-1, "vml-%d-404-%s-%s", nGroupId, sitename,token);
			return route_to_media_server(msg,newuri,"");
		}
		break;	
/*
		case OperatorRouted:
			//Get his operator's extension and process ext call
			LOG(L_ERR, "[get_group_plan] Operator Routed\n");
			nExtension = get_operator_extension(nGroupId, nSiteId );
			if ( nExtension > 0 ) {
				struct ws_uri wsuri;
				char buffer[100];
				sprintf(buffer, "ext-%d-%d-%s-0000000000", nExtension, nGroupId, sitename);
				parse_ws_uri_buffer(buffer, strlen(buffer), &wsuri);
				return process_ext_call2(msg, prio, &wsuri);
			}
			break;
*/			
		default:
			LOG(L_WARN, "[get_group_plan] Unlikely place!! Claling a regular group!\n");
			send_reply(msg, 484, "Address Incomplete. Check your PBX configuration!");
			_nNoRelay = 1;
			return 1;
			break;
	};
	return -1;
}

/*Function for reading data from executed Database query*/
int Getting_DbResult(db_res_t *res, char *cBuffer ,int nBuffSize)
{
    db_row_t row;
    int nRet = -1,nVal = 0;

    if(res == NULL)
    {
        LOG(L_ERR,"[Getting_DbResult]'res' is NULL");
        return -1;
    }

    if (RES_ROW_N(res) > 0)
    {
        nRet = 1;
        row = RES_ROWS (res)[0]; 
        if (row.values[0].nul == FALSE )
        {
            if((nVal = get_db_int(row.values[0])))
            {   
                snprintf(cBuffer,nBuffSize,"%d",nVal);
            }
            else
            {
                strncpy (cBuffer, row.values[0].val.string_val,nBuffSize);
            }
        }
    }
    if (res)
    {
        ws_dbf.free_result (h,res);
        res = NULL;
    }
  return nRet;
}


/*Added Function to Process 611 TransferCalls --Jagan*/
int process_611_calls(struct sip_msg *msg, struct ws_uri *wsuri,char *priority)
{
	int nRet = -1;
	db_res_t *res = NULL;
	char cQuery[1024],cAgentId[64] = "",cUserExtn [32] = "",cSiteName[64]="",*cPtr = NULL,cSix11extensionsitename[256] ="";

	memset(cQuery, 0x00, sizeof(cQuery)-1);
	memset(cAgentId, 0x00, sizeof(cAgentId)-1);
	memset(cUserExtn, 0x00, sizeof(cUserExtn)-1);
	memset(cSiteName, 0x00, sizeof(cSiteName)-1);
    memset(cSix11extensionsitename, 0x00, sizeof(cSix11extensionsitename)-1);
	snprintf(cQuery, sizeof(cQuery)-1, GET611_AGENTID_QUERY, wsuri->context);
    
	ws_dbf.raw_query (h, cQuery, &res);

	nRet = Getting_DbResult(res, cAgentId,sizeof(cAgentId)-1);
	if(nRet == -1)
	{
		wsuri->context =  ws_support_site;
		wsuri->command =  ws_support_exten ; 
		LOG(L_ERR," [process_611_calls] Getting AgentId Query Failed:'%s'", cQuery);
	}
	else
	{
		memset(cQuery, 0x00, sizeof(cQuery)-1);
		
		snprintf(cQuery, sizeof(cQuery)-1,GET_611USER_EXTN_QUERY,cAgentId);
		ws_dbf.raw_query (h, cQuery, &res);

		nRet = Getting_DbResult(res, cUserExtn, sizeof(cUserExtn)-1);
		if(nRet == -1)
		{
			wsuri->context =  ws_support_site;
			wsuri->command =  ws_support_exten ;
			LOG(L_ERR,"[process_611_calls] Getting Username Query Failed:'%s'",cQuery);
		}
		else
		{
			if(cPtr = strchr(cAgentId,'@'))
			{
 				cPtr++;
				if(cPtr)
					strcpy(cSiteName,cPtr);
           
				cPtr = strchr(cSiteName,'.');
				while(cPtr)
				{
					*cPtr = '-';
					cPtr = strchr(cSiteName,'.');
				}
			}
		}
		/*Here We are assigning Corresponding Customer/Agent Who are Handle 611 Calls --Jagan*/
		wsuri->command = cUserExtn ; 
		wsuri->context = cSiteName ;
	}
    
    if((_nEnableCalleridBlockCDR ==1) && (strlen(wsuri->context) > 0) && (strlen(wsuri->token) > 0)){
        snprintf(cSix11extensionsitename, sizeof(cSix11extensionsitename)-1,"%s#%s",wsuri->command,wsuri->context);
        nRet = validate_calleid_611(msg,wsuri->token,cSix11extensionsitename);

        if (nRet ==1)
            return 0;
    }
	LOG(L_ERR,"[process_611_calls] Call Routing to Support Who's Exten:'%s' and Sitename:'%s' \n",wsuri->command,wsuri->context);
	return process_ext_call2 (msg, priority, wsuri,  0,  1, 0 );
}

int get_callierid_for_fax_calls(char *cExt, char *cSiteName, char *cPhoneNumber, int nNumLen) {

	db_res_t *res=NULL;
	db_row_t row;
	char query[512]="";

	if(cExt == NULL || cSiteName == NULL) {
		LOG(L_ERR,"[get_callierid_for_fax_calls] input parameters are empty\n");
		return 0;
	}

	memset(query, 0, sizeof(query));
	snprintf(query,sizeof(query)-1,FAX_CALLERID_QUERY, cExt, cSiteName);

	LOG(L_ERR,"[get_callierid_for_fax_calls] query : %s \n", query);

	ws_dbf.raw_query (h, query, &res);
	if(res && RES_ROW_N(res) > 0){
		row = RES_ROWS(res)[0];
		if (RES_ROWS(res)[0].values[0].nul == FALSE && strlen(row.values[0].val.string_val) > 0) {
			strncpy(cPhoneNumber,row.values[0].val.string_val,nNumLen);
			ws_dbf.free_result (h,res);
			return 1;
		}
		if (RES_ROWS(res)[0].values[1].nul == FALSE && strlen(row.values[1].val.string_val) > 0) {
			strncpy(cPhoneNumber,row.values[1].val.string_val,nNumLen);
			ws_dbf.free_result (h,res);
			return 1;
		}

	}

	if(res) {
		ws_dbf.free_result (h,res);
	}

	return 0;
}

int ws_set_rpid( struct sip_msg *msg, char *target_server, const char *wsuser,char* cExtn, int nSiteid, int *is_blocked_call, int nLen, int nGcbflag, int nAddRpid, char *cSiteName)/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
{
		char fromuser[256] = "";
		char display[256] = "";
		char rpid[256] = "";
		char phonenumber[256] = "";
		struct to_body *fromb = NULL;
		struct sip_uri uri;
		int add_rpid = 1;
		int ret = 0;
		
		if (msg == NULL || target_server == NULL) {
			send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
			_nNoRelay = 1;
			return ret;
		}
		
		/*Don't need to parse From header if WS user name is available --kkmurthy 2009-10-08*/
		if(wsuser){
			strncpy(fromuser, wsuser, sizeof(fromuser)-1);
			strncpy(display, wsuser, sizeof(display)-1);
		}else{
			if(!msg || !msg->from){
				LOG(L_ERR , "[ws_set_rpid] From header field missing !!!!  \n");
				return -1;
			}

			if (! msg->from->parsed )
				parse_from_header(msg);
			fromb = get_from (msg);
			if (fromb == NULL){
				send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
				_nNoRelay = 1;
				return ret;
			}
			parse_uri(fromb->uri.s, fromb->uri.len, &uri);
			snprintf(fromuser, sizeof(fromuser)-1, "%.*s", uri.user.len,uri.user.s);
			if (fromb->display.len > 0) {
				snprintf(display, sizeof(display)-1, "%.*s", fromb->display.len,fromb->display.s);
			}
		}

		memset(phonenumber, 0, sizeof(phonenumber)-1);
		if (fromuser[0] == '+') {
				memset(rpid,0,sizeof(rpid));
				snprintf(rpid, sizeof(rpid)-1,"Remote-Party-ID: %s <sip:%s@%s>\r\n",display, &fromuser[1], target_server);
		}else if (isdigit(fromuser[0])){
			if(cSiteName != NULL && strlen(cSiteName) > 0 && nAddRpid == 0 && (strlen(fromuser) >= MINEXTLEN) && (strlen(fromuser) <= nMaxExtLen)) {					
				nAddRpid = get_callierid_for_fax_calls(fromuser, cSiteName, phonenumber, sizeof(phonenumber));
			}
			if(strlen(phonenumber) == 0) {
				normalize(fromuser, phonenumber, sizeof(phonenumber),sizeof(fromuser));
			}
			memset(rpid,0,sizeof(rpid));
			snprintf(rpid, sizeof(rpid) - 1,"Remote-Party-ID: %s <sip:%s@%s>\r\n",display, phonenumber, target_server);
		} else if (strstr(fromuser,"Anonymous")){ /* Added this for fixing Anonymous user in fromname for anonymity header*/
                        add_rpid = 0;   /* If From uri is Anonymous , not adding RPID header -vravi */
		} else {	
			/* Not adding Custom callid at msicp --vravi/dileep */	
			memset(rpid,0,sizeof(rpid));
			snprintf(rpid, sizeof(rpid)-1,"Remote-Party-ID: %s <sip:%s@%s>\r\n",display, fromuser, target_server);
		}
		LOG(L_WARN,"[ws_set_rpid]RPID:%s, %d, %d \n",rpid, nAddRpid, add_rpid);
		if (nAddRpid == 1 && (add_rpid ==1 ) && (strlen(rpid) > 0) && append_hf) {
			str cmd1={rpid,strlen(rpid)};
			if(append_hf(msg, (char *)&cmd1,0)==1){
				LOG(L_ERR,"append_hf success\n");
				ret = 1;
			}else
				LOG(L_ERR,"append_hf failure\n");
		}
		
		if(global_callblocking && nGcbflag && is_blocked_call && (fromuser[0] == '+' || isdigit(fromuser[0]))) {
			*is_blocked_call = validate_callerid(fromuser,nSiteid,cExtn,nLen,sizeof(fromuser));
		}
		return ret;
}

int get_did_plan (struct sip_msg *msg, char *prio, char *pPhone,char *moc_agent,int nType, struct ws_uri *tmp_wsuri)
{
	char realdid[64]="";
	char query[1024],ruri_out[256]="";
	db_res_t *res=NULL;
	db_row_t row;
	struct PstnPhone stPhone;
	memset(&stPhone, 0, sizeof(stPhone));
	struct ws_uri wsuri;
	/*Billing bug..tulsi-12-11-08*/
	char *sitename=NULL;
	char fromuser[128]="";
	str fromuri;
	struct sip_uri puri;
	char  buf[512]="",cOtherdc[64]="";
	char buffer[1024]="";
	int ret = 0, nGroup_DID = 0,nFound = 0,nRefBy_value = 0, nRefTo_value = 0,nDcid=0,nAccDcid = 0;
	char *cPtr = NULL,*token = NULL, *cpContext = NULL, cExtn[8] = "",cTargetServer[128]= "";
	int is_blocked_call= 0;
    int rpid_flag = 0,appid=0;
	char cnfbuf[256]="",*cnfptr=NULL,cRequestUri[256]="",cForwardFromUser[256] = "";
	int nRet = 0;

	/* To avoid the crash: 21-sep-2005 (Only a temporary patch) */
    if (NULL == pPhone || !tmp_wsuri) {
		LOG(L_ERR, "[get_did_plan] No Phone Number or tmp uri structure is NULL\n");
		return -1;
	}


	memset(realdid,0,sizeof(realdid));
	strncpy(realdid, pPhone, sizeof(realdid)-1);
	nRet = get_real_did(msg, realdid,sizeof(realdid));
	if ( !nRet ) {
                //Invalid Phone call
                send_reply(msg, 484, "Address Incomplete - No callerID");
                _nNoRelay = 1;
                return 1;
        }else if(nRet == 1) {
		prepare_contact_header(msg, "mtd");
		send_reply(msg, 302, "Moved temporarily");
		_nNoRelay = 1;
		return 1;
	}
		
	if (pPhone[0] == '+') {
		strncpy(realdid, &pPhone[1],sizeof(realdid)-1);
		LOG(L_ERR, "[get_did_plan] E164 Format : %s\n",realdid);
	} /*else {//commented by tulsi for fix the NPA dialling bug id 4491
		strcpy(realdid, pPhone);
	}*/
	
	/* if context value is like "00-sitename" then parse it to discard "00-" */
	memset(query,0,sizeof(query));
	if(tmp_wsuri->type && !strncmp(tmp_wsuri->type,"pst",3)) {
		cpContext = tmp_wsuri->context;
		if(cpContext && isdigit(cpContext[0]) && (cPtr = strchr(cpContext,'-')))
		{
			cpContext = ++cPtr;
		}
	}
	if(cpContext){
		snprintf(query, sizeof(query)-1, ACCOUNTDIDQUERY_TRANSFER, cpContext, realdid);
	}else {
        snprintf(query, sizeof(query)-1, ACCOUNTDIDQUERY, realdid);
	}
    ws_dbf.raw_query (h, query, &res);
	token = tmp_wsuri->token?tmp_wsuri->token:"";
    if (!res) {
       	LOG(L_ERR,"[get_did_plan] No Rows found\n");
       	bzero(ruri_out,sizeof(ruri_out));
       	snprintf(ruri_out,sizeof(ruri_out)-1,"inv-0-0-pandoranetworks-com-%s",token);
       	return route_to_server (msg,ruri_out,umslb_proxy,MS);
    }
    if ( RES_ROW_N(res) > 0 ) {
       	nFound = 1;
       	stPhone.phone = realdid;
       	row = RES_ROWS(res)[0];
       	stPhone.nSiteid = get_db_int(row.values[0]);
       	stPhone.nAssignedID = get_db_int(row.values[1]);
       	stPhone.nAssignedTo = get_db_int(row.values[2]);
       	strncpy(stPhone.sitename, row.values[3].val.string_val,sizeof(stPhone.sitename)-1);
		if (!VAL_NULL(&row.values[4])){
	       	nRefTo_value = get_db_int(row.values[4]);
		}
		if (!VAL_NULL(&row.values[5])){
			nRefBy_value = get_db_int (row.values[5]);
		}
		if (!VAL_NULL(&row.values[6])){
			nDcid = get_db_int(row.values[6]);
		}
        if (!VAL_NULL(&row.values[7])){
            nAccDcid = get_db_int(row.values[7]);
        }
	}		
	
	if(res){
       	ws_dbf.free_result (h,res);
    }

    if(!nDcid && nAccDcid > 0) {
        nDcid = nAccDcid; /*Assigning account level home data centre dcid*/
    }

	LOG(L_WARN, "[get_did_plan] Values: SiteId: %d GroupId: %d AssignedTo: %d, nRefBy_value: %d, nRefTo_value: %d, nDcid : %d nAccDcid: %d \n", stPhone.nSiteid, stPhone.nAssignedID, stPhone.nAssignedTo,nRefBy_value,nRefTo_value,nDcid,nAccDcid);

   	if ((nRefBy_value == BLOCK_OB || nRefBy_value == BLOCK_BOTH) || (nRefTo_value == BLOCK_IB || nRefTo_value == BLOCK_BOTH) )
   	{  
		LOG(L_WARN, "[get_did_plan]Checked Calling privileges and Terminating call here\n");	
		return -1;
   	}

	/* Added changes for checking whether this user is having proper class of service for making another account DID calls --Saidulu/Selvan */
	if((extonly_enable == 1 ) && nFound && stPhone.nSiteid && get_isext911_plan(msg,stPhone.nSiteid)){
		bzero(buffer,sizeof(buffer));
		snprintf(buffer,sizeof(buffer)-1,"cos-0-0-pandoranetworks-com-%s",token);
		return route_to_server(msg,buffer,umslb_proxy,MS);
	}
	/*Framing target server to call ws_set_rpid fucntion*/
	memset(cTargetServer, 0,sizeof(cTargetServer));
	if(stPhone.nAssignedTo == ASSIGNED_TO_SMARTCONNECT) {
		strncpy(cTargetServer, smartconnect_server, sizeof(cTargetServer)-1);
	}
	else if (stPhone.nAssignedTo == ASSIGNED_TO_FAX) {
		strncpy(cTargetServer, fax_server, sizeof(cTargetServer)-1);
	}
	else {
		strncpy(cTargetServer, umslb_proxy, sizeof(cTargetServer)-1);
	}

	rpid_flag = ws_set_rpid(msg, cTargetServer,NULL,cExtn,stPhone.nSiteid,&is_blocked_call,sizeof(cExtn)-1,1, nFound, tmp_wsuri->context);
	if(is_blocked_call > 0) {
		return route_to_blockdestination(msg,cExtn,stPhone.sitename, token, is_blocked_call,"","");
	}

	if(!nFound){
		memset(cForwardFromUser, 0, sizeof(cForwardFromUser));
		if(nEnableCosTransfer == 1) {
			if (nType == ACD_DID && is_header_exists(msg,"ForwardFromUser", 15, cForwardFromUser, sizeof(cForwardFromUser)) == 0 ) {
				bzero(ruri_out,sizeof(ruri_out));
				snprintf(ruri_out,sizeof(ruri_out)-1,"oms-%s-0-%s-%s", realdid, moc_agent,token);
				return route_to_acd_server( msg,ruri_out,"",0,1);
			}
		}else if (nType == ACD_DID) {
			bzero(ruri_out,sizeof(ruri_out));
			snprintf(ruri_out,sizeof(ruri_out)-1,"oms-%s-0-%s-%s", realdid, moc_agent,token);
			return route_to_acd_server( msg,ruri_out,"",0,0);
		}
		/*This if added by tulsi for fixing the WARM transfer to DID to DID.*/
		ret = set_rpid_from_user(msg, realdid, NULL, token); 
		if ( ret == 1 ) 
		{
                	/*wsuri.context = NULL;*/
                   	memset(&wsuri,0x00,sizeof(struct ws_uri));
					/*Here we may not get the sitename at the time of doing blind trasnfer to PSNT
					 * Because from header is not valid..*/
                   	if(stPhone.sitename[0] != '\0')
				   	{
                        wsuri.token = token;
                        wsuri.context = stPhone.sitename;
                   	}else{/*This else added for fix "null" sitename at the time of sending to oms of warm or blind transfer call*/
						wsuri.token = token;
						if(nType == MOBILECALL && moc_agent){
							sitename = strchr(moc_agent,'-');
							if(sitename){
								*sitename = '\0';
								sitename++;
							}
						}
						else {
							/*Getting the from header-added code on 12-11-08-tulsi-for fix the 4878 bug*/
							if (parse_from_header(msg) == -1) {
								LOG (L_ERR, "Oh! Failed parsing from header\n");
								return -1;
							}
							bzero(fromuser,sizeof(fromuser));
							fromuri = get_from(msg)->uri;
							if (parse_uri(fromuri.s, fromuri.len, &puri) < 0) {
								snprintf(buf, sizeof(buf)-1, "%.*s", fromuri.len, fromuri.s);
								LOG (L_ERR, "Huh? Call without a From header?!\n");
								return -1;
							}
							snprintf(fromuser, sizeof(fromuser)-1, "%.*s", puri.user.len, puri.user.s);
							LOG(L_WARN, "[get_did_plan] From Header: %s\n", fromuser);
							sitename = strchr(fromuser,'-');
							if(sitename){
								*sitename = '\0';
								sitename++;
								/*Added for mobile call --kkmurthy 2nd January 2009*/
								if(moc_agent == NULL)
									moc_agent = fromuser;
							}
						}
						if(sitename == NULL){		
							strncpy(stPhone.sitename,ws_dummy_site,sizeof(stPhone.sitename)-1);
							wsuri.context = stPhone.sitename;
						}else{
							LOG(L_WARN, "[get_did_plan] Sitename: %s\n", sitename);
							wsuri.context = sitename;
						}
					}
                   		/*route_to_pstn(msg, realdid);*/
	                   	return route_to_outboundserver1( msg ,realdid,&wsuri,moc_agent);
			}else if ( ret == -3 ){ 
	          		LOG(L_WARN, "[get_did_plan] From header is digit...\n");
					return -4; 
       		}else {
				LOG(L_WARN, "[get_did_plan] COS for outbound calls Forbidden \n");
				if(nEnableCosTransfer) {
					send_reply(msg, 302, "Moved temporarily");
  					_nNoRelay = 1;
				}else{
					bzero(ruri_out,sizeof(ruri_out));
					snprintf(ruri_out,sizeof(ruri_out)-1,"cos-0-0-pandoranetworks-com-%s",token);
					return route_to_server (msg,ruri_out,umslb_proxy,MS);
				}
       		}
            return 1;
			//return route_to_server (msg, "cos-0-0-pandoranetworks-com-0000000000",media_server);
		}
	
	if ( !stPhone.nAssignedTo ) {
		// This is an unassigned internal pbx number. Just return 404.
		LOG(L_ERR, "[get_did_plan] Unassigned number\n");
		bzero(ruri_out,sizeof(ruri_out));
		snprintf(ruri_out,sizeof(ruri_out)-1,"inv-0-0-pandoranetworks-com-%s",token);
		return route_to_server (msg,ruri_out,umslb_proxy,MS);
	}
	bzero(buffer,sizeof(buffer));
    /*Added below if condition for send this call to otherdatacenter through self wsicp --Balaji/Badareenadh [31/12/2019]*/
    if(stPhone.nAssignedTo != ASSIGNED_TO_CONFERENCE && nDcid!=datacenter_id && nDcid != 0){
        memset(cRequestUri,0,sizeof(cRequestUri));
        snprintf(cRequestUri, sizeof(cRequestUri)-1,"WSRequestURI: %s\r\n",token);
        str cmd1={cRequestUri,strlen(cRequestUri)};
        if (append_hf) {
            if(append_hf(msg, (char *)&cmd1,0)==1)
                LOG(L_WARN,"append_hf success\n");
            else
                LOG(L_WARN,"append_hf failure\n");
        }
        snprintf(buffer, sizeof(buffer)-1, "%s",realdid);
        msg->extra_info.nOtherDataCenterFlag = 1;
        if(WSicpState->nICPServerType == PRIMARY){
            return route_to_server(msg, buffer , wsicp_primary,MS);
        }else{
            return route_to_server(msg, buffer , wsicp_secondary,MS);
        }
    }
	/*Added for checking did for interaccount call and send oms- to umslb --rajesh 15apr2021 */
	if(nStirShakenRoute && nType == PSTN_Transfer ){
		if ( route_to_balancer_on_did_checks(msg, realdid,token,stPhone.sitename) != -1 ){
			return 1;
		}
	}
	switch ( stPhone.nAssignedTo ) {
		case ASSGINED_TO_GROUP:
			//Ext group or acd group
			//worksmart_get_context2
			LOG(L_WARN, "[get_did_plan] Assigned to group\n");
			return get_group_plan(msg, prio, stPhone.nAssignedID, stPhone.nSiteid, stPhone.sitename,realdid,token,nType,nAccDcid, realdid);
			break;
			
		case ASSIGNED_TO_NORMAL_GROUP_DID:
			nGroup_DID = 1;
		case ASSIGNED_TO_EXTENSION:
			LOG(L_WARN, "[get_did_plan] Assigned to Extension\n");
			memset(buffer,0,sizeof(buffer));
			snprintf(buffer, sizeof(buffer)-1, "xfer-%d-%d-%s-%s",stPhone.nAssignedID,nGroup_DID,stPhone.sitename, token);
			if(nType == ACD_DID){
				parse_ws_uri_buffer(buffer, strlen(buffer), &wsuri);
				return ws_ext_call_process (msg, prio, &wsuri, 0,0,0,ASSIGNED_TO_EXTENSION, rpid_flag);
			}else{
				ws_set_iattr(msg, 250);
				return route_to_media_server (msg, buffer,"");
			}
			break;
			
		case ASSIGNED_TO_CONFERENCE:
			LOG(L_WARN, "[get_did_plan] Assigned to Conference\n");
			memset(buffer,0,sizeof(buffer));
			/* Checking for Conference DID dcid, In case of otherDC DID routing the call to homeDC wsicp*/
			if(nRouteAcdAndConfCallsToHomeDc == 0 && nDcid!=datacenter_id){
				memset(cOtherdc,0,sizeof(cOtherdc));
				snprintf(cOtherdc, sizeof(cOtherdc)-1,"OtherDc: %d\r\n", nDcid);
				str cmd1={cOtherdc,strlen(cOtherdc)};
				if (append_hf) {
					if(append_hf(msg, (char *)&cmd1,0)==1)
						LOG(L_WARN,"append_hf success\n");
					else
						LOG(L_WARN,"append_hf failure\n");
				}
				/* Getting the WSWEBCallInfo header value to get "act-" url in case of prompted code */
				if(is_header_exists(msg,"WSWEBCallInfo",13,buffer,sizeof(buffer))){
					if((cnfptr=strchr(buffer,'#')))
						cnfptr++;
				}
				/* Framing the uri "act-DID-code-sitename-token" to fix the AcrossDC conference prompted code issue */
				if(cnfptr && !strncmp(cnfptr,"act-",4)){
					strcpy(cnfbuf,cnfptr+4);	
					if((cnfptr=strchr(cnfbuf,'-'))){
						snprintf(buffer, sizeof(buffer)-1, "act-%s%s",pPhone,cnfptr);
					}
				}else{
					snprintf(buffer, sizeof(buffer)-1, "%s", pPhone);
				}
				if(WSicpState->nICPServerType == PRIMARY){
					return route_to_server(msg, buffer , wsicp_primary,MS);
				}else{
					return route_to_server(msg, buffer , wsicp_secondary,MS);
				}
			}
			snprintf(buffer, sizeof(buffer)-1, "ext-%d-00-%s-%s", stPhone.nAssignedID, stPhone.sitename,token);
			parse_ws_uri_buffer(buffer, strlen(buffer), &wsuri);
			return process_conf_call2(msg,&wsuri,0);	
			break;
			
		case ASSIGNED_TO_X11PROFILE:
			LOG(L_WARN, "[get_did_plan] Assigned to X11_returnedcall\n");
			return process_x11_returnedcall(msg, pPhone);
			break;
			
		case ASSIGNED_TO_FAX:
			LOG(L_WARN, "[get_did_plan] Assigned to ASSIGNED_TO_FAX\n");
			memset(buffer,0,sizeof(buffer));
			snprintf(buffer, sizeof(buffer)-1, "fax-%d-%s-%s-%s", stPhone.nAssignedID, realdid, stPhone.sitename, token);
			ws_set_iattr(msg, 90);
			return route_to_server(msg, buffer, fax_server,OTHER);
			break;
		case ASSIGNED_TO_SMARTCONNECT:
			LOG(L_WARN, "[get_did_plan] Assigned to ASSIGNED_TO_SMARTCONNECT\n");
			memset(&buffer, 0, sizeof(buffer));
			snprintf(buffer, sizeof(buffer)-1, "9994-%d-%s", stPhone.nSiteid, realdid);
			return route_to_server(msg, buffer, smartconnect_server,OTHER);
			break;
		case ASSIGNED_TO_VOICEMAIL:
			bzero (buffer, sizeof (buffer));
			snprintf (buffer, sizeof (buffer)-1, "vmr-9999-0-%s-%s", stPhone.sitename,token);
			//return route_to_server(msg, buffer,voicemail_server);
			return route_to_media_server(msg, buffer,"");
			break;

		 case ASSIGNED_TO_BLA:
			bzero (buffer, sizeof (buffer));
			snprintf(buffer, sizeof(buffer)-1, "bla-%d-00-%s-%s", stPhone.nAssignedID, stPhone.sitename, token);
			if(nType == ACD_DID) {
				parse_ws_uri_buffer(buffer, strlen(buffer), &wsuri);
				return process_bla_call (msg, prio , &wsuri, 0);
			}
			ws_set_iattr(msg, 250);
			return route_to_media_server (msg, buffer,"");
			break;
		case ASSIGNED_TO_UCAPP:
			appid = get_wsucapp_id(stPhone.nAssignedID,stPhone.nSiteid);
			if( appid > 0 ){
				memset(buffer,0,sizeof(buffer));
				snprintf(buffer,sizeof(buffer)-1, "wsucapp-%s-%d-%s-%s", realdid, appid, stPhone.sitename,token);
				return route_to_media_server (msg, buffer, "");
			}else{
				LOG(L_ERR,"[get_did_plan] No Proper App ID found\n");
				send_reply(msg, 500, "Internal Server Error, Hint: No Proper UCapp Found");
			}
			break;
	};
	return -4; /*Added by tulsi for media pool. But this is no need..need t review once again*/
}

int route_to_feature_server (struct sip_msg *msg, int nFeature,char *token) {
	struct sip_uri parsed_uri;
	db_res_t *res=NULL;
	db_row_t row;
	char context[128];
	char newuri[128];
	char fromuser[64];
	static char * formatstring = ""
					      "SELECT replace(replace(substring(p.agentid, instr(p.agentid,'@')+1), '.', '-'), '_', '-'), p.agentid, p.siteid"
			    	  	  "  FROM pbxusers p, subscriber s"
					      " WHERE s.username = '%s'"
						  "   AND p.agentid = s.agentid";

	static char query [1024];
    int nSiteId = 0;


	if (-1 == parse_from_header (msg)) {
		send_reply(msg, 500, "Internal Server Error, Hint: FS-2");
		_nNoRelay = 1;
		return 1;
	}

	if (parse_uri (get_from(msg)->uri.s, get_from(msg)->uri.len, &parsed_uri)) {
		send_reply(msg, 500, "Internal Server Error, Hint: FS-3");
		_nNoRelay = 1;
		return 1;
	}
	memset(fromuser, 0, sizeof(fromuser));
	memcpy(fromuser, parsed_uri.user.s, parsed_uri.user.len);

	//Get his context.
	snprintf(query, sizeof(query)-1, formatstring, fromuser);
	//LOG(L_ERR, "[route_to_feature_server]\n %s\n", query);

	ws_dbf.raw_query (h, query, &res);

	if ( !res ) {
		send_reply(msg, 500, "Internal Server Error, Hint: FS-4");
		_nNoRelay = 1;
		return 1;
	} else if (RES_ROW_N(res) > 0) {
		row = RES_ROWS (res)[0];
		memset(context,0,sizeof(context));
		strncpy (context, row.values[0].val.string_val, sizeof(context)-1);
        nSiteId = get_db_int(row.values[2]);
		if (res)
			ws_dbf.free_result (h,res);
	} else {
		send_reply(msg, 403, "Forbidden - No such user Hint: FS-5");
		_nNoRelay = 1;
		if (res)
			ws_dbf.free_result (h,res);	
		return 1;
	}

	switch ( nFeature )	{
		case SmartConnect:
				memset(&newuri, 0, sizeof(newuri));
				snprintf(newuri, sizeof(newuri)-1, "9994-%d-9994",nSiteId);
				route_to_server(msg, newuri, smartconnect_server,OTHER);
				break;
		case AcdAgent:
	            //ACD-Agent Login with 9997   --Added on 15/07/05 Ramu
	            //route_wsacdserver(agt-blah-blah );
	            bzero (newuri, sizeof (newuri));
	            snprintf (newuri, sizeof (newuri)-1, "agt-9997-0-%s-%s", context,token);
        	    return route_to_acd_server(msg, newuri, "", 0,0);
	            break;
		case GreetingAdmin:
			//route_to_media_server(rec-blah blah blab);
			bzero (newuri, sizeof (newuri));
			snprintf (newuri, sizeof (newuri)-1, "rec-9998-0-%s-%s", context,token);
			return route_to_media_server(msg, newuri,"");
			break;
		case VoicemailAdmin:
			//route_to_media_server(vmr-blah blah blab);    /* need to modify before framing this uri to include group code in the uri */	
			bzero (newuri, sizeof (newuri));
			snprintf (newuri, sizeof (newuri)-1, "vmr-9999-0-%s-%s", context,token);
			return route_to_media_server(msg, newuri,"");
			break;
		case AutoProvision:
			bzero (newuri, sizeof (newuri));
			snprintf (newuri, sizeof (newuri)-1, "aps-9994-0-%s-%s", context,token);
			return route_to_media_server(msg, newuri,"");
			break;
		default:
			send_reply(msg, 484, "Address Incomplete Hint: FS-6");
			_nNoRelay = 1;
			return 1;
			break;
	}
	return 1;
}

int get_group_operator_plan ( struct sip_msg *msg, char * prio, struct ws_uri * wsuri, int nFlag,int VmlFlag) {
	char * formatstring = ""
		"SELECT p.extensions, si.siteid, s.title,1,si.stir_route,p.stir_enable,p.stir_route from special s, siteinfo_new si, pbxusers p"
		" WHERE si.siteid = s.siteid"
		"   AND s.siteid = p.siteid"
		"   AND p.agentid = operator"
		"   AND code = %s"
		"   AND si.sitename =REPLACE(REPLACE('%s','-','.'),'_','.') and acctype!=2 and acctype!=4"
		"	UNION"
		"	SELECT operator,si.siteid, s.title,2,si.stir_route,0,0 from special s, siteinfo_new si "
		"	WHERE si.siteid = s.siteid AND code = %s"
	   	"	AND  si.sitename = REPLACE(REPLACE('%s','-','.'),'_','.') and acctype!=2 and acctype!=4";
	char query[2048];
	db_res_t *res=NULL;
	int nExtension = 0, nSiteID = 0,nUsr_Flag=0;
	char buffer[128], cSpamBuffer[128] = "";
	int nStirRoute = 0, nStirEnable = 0;

	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group ) {
		return -1;
	}

	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, formatstring, wsuri->group, wsuri->context,wsuri->group, wsuri->context);
	ws_dbf.raw_query(h, query, &res);
	if(res==NULL){
		LOG(L_ERR,"[get_group_operator_plan]no result set with query:%s\n",query);
		return -1;
	}
	if ( RES_ROW_N(res) > 0 ) {
		if(RES_ROWS(res)[0].values[0].val.string_val)
			nExtension = get_db_int(RES_ROWS(res)[0].values[0]);
		nSiteID = get_db_int(RES_ROWS(res)[0].values[1]);
		if(RES_ROWS(res)[0].values[2].val.string_val){
			strncpy(group_name,RES_ROWS(res)[0].values[2].val.string_val , sizeof(group_name)-1);	
		}
		nUsr_Flag = get_db_int(RES_ROWS(res)[0].values[3]);
		if(RES_ROWS(res)[0].values[4].val.string_val) { /* Account level spam settings. */
			nStirRoute = get_db_int(RES_ROWS(res)[0].values[4]);
		}
		if(RES_ROWS(res)[0].values[5].val.string_val) { /* User spam settings is enable or not */
			nStirEnable = get_db_int(RES_ROWS(res)[0].values[5]);
		}
		/* Applying user spam settings */
		if(nStirEnable == 1 && RES_ROWS(res)[0].values[6].val.string_val) { 
			nStirRoute = get_db_int(RES_ROWS(res)[0].values[6]);
		}

	}
	if (res)
	ws_dbf.free_result(h, res);

	if(nStirRoute != STIR_ROUTE_WITH_SPAM && IsSpamCall(msg) && (prepare_contact_spam_route_fail(msg, nStirRoute) == 1)) {
		memset(buffer, 0, sizeof(buffer));
		snprintf(buffer, sizeof(buffer)-1, "ext-%d-%s-%s-%s", nExtension, wsuri->group, wsuri->context, wsuri->token);
		snprintf(cSpamBuffer, sizeof(cSpamBuffer)-1, "SPAM#%d", nStirEnable);
		racc_missed_call_entry(msg, buffer, cSpamBuffer);
		send_reply(msg, 302, "Moved temporarily");
  		_nNoRelay = 1;
		return 1;
	}

	if ( nExtension == 0) {
		if(VmlFlag){
			memset(buffer,0,sizeof(buffer));
            /* reverted 'vml-00-00-%s-%s' at place of 'vml-00-11-%s-%s' for dial by name in AA call ---- Balaji  */
            snprintf (buffer, sizeof (buffer)-1, "vml-00-00-%s-%s", wsuri->context, wsuri->token);
			route_to_media_server(msg, buffer,"");
			return 1;
		}
		return -1;
	}

	memset(buffer,0,sizeof(buffer));
	if(nUsr_Flag == Ivr_User){/*Operator as user*/
		snprintf(buffer, sizeof(buffer)-1, "ext-%d-%s-%s-%s", nExtension, wsuri->group, wsuri->context, wsuri->token);
		parse_ws_uri_buffer(buffer, strlen(buffer), wsuri);
		return process_ext_call2(msg, prio, wsuri, nFlag,0,0);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
	}else if(nUsr_Flag == Ivr_Group){/*Operator as group*/
		return get_opr_grp_plan(msg, prio, wsuri, nExtension,nFlag);
	}
	return -1;
}
void use_rtp_proxy (struct sip_msg *msg) {
#ifdef USE_RTP_PROXY
	if ( force_rtp_proxy ) {
		force_rtp_proxy (msg, NULL, NULL);
	}
#endif
#ifdef USE_MEDIA_PROXY
	if ( use_media_proxy ) {
   		use_media_proxy(msg, NULL, NULL);
	}
#endif

}

/*This fuction usage  is checking the invite came from the media server or not with REPLACES header. But we are not using as of now*/
int ws_check_for_attendedtransfer(struct sip_msg *msg ,char  *ruri)
{
		struct hdr_field *hf = NULL;
		int ret=0;
		char ipport[32]="";
		char recv_ip[32]="";
		int  recv_port=0;
		int flag=0;

#if 0
		if (!strncmp(ruri,"pst-",4)){
				LOG(L_ERR,"[ws_check_for_attendedtransfer] No need to send call to media server\n");
				return -1;
		}
#endif
		/* Here we are finding this invite has the Replaces header or not*/
		for (hf=msg->headers; hf; hf=hf->next)
		{
				if (hf->name.len!=8)continue;
				if (strncasecmp (hf->name.s, "Replaces", 8) != 0)continue;
				LOG(L_ERR, "Got INVITE for Supervised transfer Call \n");
				flag = 1;
				break;
		}
		
		memset(recv_ip,0,sizeof(recv_ip));
		strncpy(recv_ip, ip_addr2a(&msg->rcv.src_ip), sizeof(recv_ip)-1);
		recv_port = msg->rcv.src_port;
		memset(ipport,0,sizeof(ipport));
		ret = snprintf(ipport, sizeof(ipport)-1,"%s:%d", recv_ip, recv_port);
		if(ret > sizeof(ipport)){
				LOG(L_ERR, "[ws_check_for_attendedtransfer]IPPORT Size is Exceeds!\n");
				return -1;
		}

		if(((isInList(mediaips,ipport)) == 1 ) && (flag == 1))
		{
				/*Came from the media pool*/
				return 1;
		}else{
				/*Came from the other server*/
				return -1;
		}
		return -1;
}

/*Added for for Mobile call processing --kkmurthy 9th December 2008*/
static int check_special_characters (char * data){
	int i;
	if( data == NULL  || strlen(data) <= 0){
		return -1;
	}
	for (i=0;i<strlen(data);i++)
	{
		if (!isdigit(data[i]))
		{
			return -1;
		}
	}
	return 0;
}

// Added for handling ace forward call to particular phone type
static int process_aceforwardcall (struct sip_msg *msg, char *priority, struct ws_uri *wsuri)
{
      int ret = 0, nExten = 0 , nRespCode = 0, branch = 0, nFailure = 0;
	  int nSeq = atoi(priority);
      char phone[256]= "";
	  char newuri[512] = "";
	  int nDcidHeader = 0;

	  if ( !wsuri || !wsuri->context || !wsuri->group || !wsuri->token || !wsuri->type || !wsuri->command) {
		  LOG(L_ERR, "[process_aceforwardcall] Got user details are NULL \n");
	      return -1;
	  }
	  if(nSeq == 1) {
    	   memset(phone , 0 , sizeof(phone));
	       snprintf(phone , sizeof(phone) - 1 , "%s-%s" , wsuri->group , wsuri->context);
    	   ret = get_uri_contact(msg , phone , &branch,NULL, &nDcidHeader);
	       if(branch <= 0 || ret == 0){
    	           LOG(L_ERR, "[process_aceforwardcall] did user register? here\n");
				   nFailure = 1;
		   }
		   if ( !nFailure && t_on_failure ) {
    	       t_on_failure (msg, (char *)(nSeq + 1), NULL);
			   ws_set_iattr(msg, 60);
           }
	  }
	  if(nFailure || nSeq > 1) {
			  nRespCode = get_resp_code(msg);
              memset(phone , 0 , sizeof(phone));
	          snprintf(phone , sizeof(phone) - 1 , "%s-%s" , wsuri->group , wsuri->context);
			  memset(newuri,0,sizeof(newuri));
			  nExten = get_direct_extension(phone, 0);
			  if(nExten == -1) {
		  			LOG(L_ERR, "[process_aceforwardcall] Failed to get User extension \n");
					return -1;
			  }
			  snprintf (newuri, sizeof (newuri)-1, "vml-%d-%d-%s-%s", nExten, nRespCode, wsuri->context, wsuri->token);
			  
			  return route_to_media_server(msg, newuri,"");
	  }
      return 1;
}

/*Added for for Mobile call processing --kkmurthy 12th November 2008*/
static int process_mobilecall(struct sip_msg *msg, char *priority, struct ws_uri *wsuri) 
{
	char query[512]="",result[256]="",mobilecalluser[256]="",*p=NULL;
	db_res_t *res=NULL;
	db_row_t row;
	int mcpriority = -1,branch = 0,ret = -1,nLength = 0, nPvalue = 0, nDcidHeader = 0;
  	
	LOG(L_WARN, "[process_mobilecall] mobilecall processing extension:%s  priority:%s sitename:%s\n",wsuri->command,wsuri->group,wsuri->context);

	if(!wsuri->command || !wsuri->group || !wsuri->context){
		LOG(L_ERR, "[process_mobilecall] invalid mobile URI?\n");
		return -1;	
	}
	mcpriority = atoi(wsuri->command);
		
	memset(mobilecalluser,0,sizeof(mobilecalluser));
	snprintf(mobilecalluser,sizeof(mobilecalluser)-1,"%s@%s",wsuri->group,wsuri->context);
	if((p = strchr(mobilecalluser,'-')))
		*p='.';
	if((p = strchr(mobilecalluser,'1')) || 	(p = strchr(mobilecalluser,'2'))){
		*p++ = '\0';
		strcat(mobilecalluser,p);
	}

	LOG(L_WARN, "[process_mobilecall] mobile call user is %s\n",mobilecalluser);
	memset(query,0x00,sizeof(query));
	snprintf (query, sizeof(query)-1, MOBILECALL_QUERY, mobilecalluser,mcpriority);
	LOG(L_WARN, "[process_mobilecall]2 %s\n", query);
	ws_dbf.raw_query (h, query, &res);
  	if (res == NULL || RES_ROW_N(res) <= 0) {
		LOG(L_ERR, "[process_mobilecall] mobile call priority is not found %d\n",mcpriority);
		if (res)
			ws_dbf.free_result (h,res);
		return -1;
	}
	row = RES_ROWS(res)[0];
	if (RES_ROWS(res)[0].values[0].nul == FALSE) {
		memset(result,0x00,sizeof(result));
		strncpy(result,row.values[0].val.string_val,sizeof(result));
	}
	if (!VAL_NULL(&row.values[1])){
		nPvalue = get_db_int (row.values[1]);
	}
	if (res)
		ws_dbf.free_result (h,res);
	
	if (nPvalue == BLOCK_OB || nPvalue == BLOCK_BOTH )
	{
       	LOG(L_WARN, "[process_mobilecall]Checked Calling privileges and Terminating call here:%d\n",nPvalue);
       	return -1;
	}	
	LOG(L_WARN, "[process_mobilecall] mobile call priority:%d value:%s\n",mcpriority,result);
	if(!check_special_characters (result)){
		if((p = strchr(mobilecalluser,'@')))
				*p = '-';
		if((p = strchr(mobilecalluser,'.')))
				*p = '-';
		nLength = strlen(result);
		LOG(L_WARN, "international prefix:%s length:%d\n",international_prefix,prefix_length);
		if (!strncmp(result, international_prefix, prefix_length)) {
			LOG(L_WARN, "[process_mobilecall]routing to PSTN\n");
		   /*ws_set_rpid(msg,umslb_proxy,mobilecalluser);*/
			 if(set_rpid_from_user(msg, result, NULL, wsuri->token)){
				ret = route_to_outboundserver1( msg,result,wsuri,wsuri->group);
			 }else{
				ret = route_to_server (msg, "cos-0-0-pandoranetworks-com-0000000000",umslb_proxy,MS);
			 }
			 return ret;
		} else if (nLength >= 7 && nLength <= 11 ) {
			LOG(L_WARN, "[process_mobilecall]routing to DID\n");
			ret = get_did_plan(msg,priority,result,mobilecalluser,MOBILECALL,wsuri);
			LOG(L_WARN, "[process_mobilecall]routing to DID:%d\n",ret);
			return ret;
		}else{
			LOG(L_WARN, "[process_mobilecall]Invalid phone number:%s ?\n",result);
			return -1;
		}
	}else{
		LOG(L_WARN, "[process_mobilecall]routing to EXTN\n");
		ret = get_uri_contact(msg,result, &branch,NULL, &nDcidHeader);
		if(branch <= 0){
			LOG(L_ERR, "[process_mobilecall] did user register? %d\n",mcpriority);
			return -1;	
		}
	}
	return 1;
}

int process_inv_call(struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int ob_ext_txfer)
{

	int flag = -1;
	int nRes = -1;
	int is_conf = 0;
	int siteid = 0;
	int grpcode = 0;
	char sitename[256] = "";
	char fullquery[4096] = "";
	char newuri[512]="";

	db_row_t row ;
	db_res_t *result = NULL;
	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group || !wsuri->token) {
		LOG (L_WARN, "[process_inv_call] Invalid wsuri format??:\n");
		return -1;
	}

	char * ws_ext_query = ""
	" SELECT 1 assignedto, REPLACE(si.sitename,'.','-'), si.siteid, ag.conference, 0 grpcode FROM agent ag, "
	" siteinfo_new si WHERE si.siteid = ag.siteid AND si.siteid IN (SELECT a.siteid "
	" FROM siteinfo_new a, accountassociationdetails b WHERE a.multiaccountassociationid = b.associationid AND b.sitename = REPLACE('%s','-','.') AND a.sitename != REPLACE('%s','-','.')) "
	" AND si.sitename != REPLACE('%s','-','.') AND extensions = '%s' "
	" UNION "
	" SELECT 2 assignedto, REPLACE(si.sitename,'.','-'), si.siteid, 0 conference, sp.code FROM special sp, "
	" siteinfo_new si WHERE si.siteid = sp.siteid AND sp.siteid IN (SELECT a.siteid FROM "
	" siteinfo_new a, accountassociationdetails b WHERE a.multiaccountassociationid = b.associationid "
	" AND b.sitename = REPLACE('%s','-','.') AND a.sitename != REPLACE('%s','-','.')) AND si.sitename != REPLACE('%s','-','.') "
	" AND extensions = '%s' LIMIT 1 ";

	memset(fullquery,0,sizeof(fullquery));
	snprintf(fullquery,sizeof(fullquery)-1,ws_ext_query,wsuri->context,wsuri->context,wsuri->context,wsuri->command,wsuri->context,wsuri->context,wsuri->context,wsuri->command);
	ws_dbf.raw_query (h, fullquery, &result);
	if(result==NULL){
		LOG(L_ERR,"[process_inv_call]'result' is NULL with Query : '%s' \n", fullquery);
		return nRes;
	}

	if (RES_ROW_N(result) > 0) {
		row = RES_ROWS(result)[0];
		if (row.values[0].nul == FALSE)
			flag = get_db_int (row.values[0]);
		if (row.values[1].nul == FALSE) {
			strncpy(sitename,row.values[1].val.string_val,sizeof(sitename)-1);
			if(strlen(sitename)>0)
				wsuri->context = sitename;
		}
		if (row.values[2].nul == FALSE)
			siteid = get_db_int (row.values[2]);
		if (row.values[3].nul == FALSE)
			is_conf = get_db_int (row.values[3]);
		if (row.values[4].nul == FALSE)
			grpcode = get_db_int (row.values[4]);
	}
	if(result){
		ws_dbf.free_result(h,result);
	}

	if( flag == 1 ){
		if(is_conf == 1)
			nRes = process_conf_call2 (msg, wsuri,0);
		else{
			if(ob_ext_txfer == OB_Transfer)
				nRes = process_ob_ext_call2(msg, priority, wsuri);
			else
				nRes = process_ext_call2 (msg, priority, wsuri, 0, 1,0);
		}
	}else if(flag == 2){
		nRes = ws_grp_ext_call_process (msg, priority, wsuri, 0, 1 , 0, 0);
	}else{
		memset(newuri,0,sizeof(newuri));
		snprintf (newuri, sizeof (newuri)-1, "vml-00-00-%s-%s",wsuri->context, wsuri->token);
		LOG(L_WARN, "[process_inv_call] invalid extension after associative account check*=*=*=*=*=*=* uri is '%s'\n", newuri);
		return route_to_media_server(msg, newuri,"");
	}
	return nRes;
}

int worksmart_get_dialplan2 (struct sip_msg *msg, char *prio1, char *prio2) {
	enum UriType utType;
	struct ws_uri wsuri;
	int nRes = -1;
	char uri[255], buffer[DIDSIZE],ruri_out[256]="",time_s[32]="",cSitename[256] = "" ;
	struct sip_uri sipuri;
	int i, uri_len;
	_nNoRelay = 0;
	_nOrigSeq = atoi(prio1);
	char cAgentid[128] = "";
	char tempext[16]="";
	char tempBuff[256] = "";
	
	/*Added for mediapool attended transfer-tulsi*/
	_nSupSeq = atoi(prio2);
	int mediapool = 0;
	char *ptr = NULL,cNewUri[128] = "";
	int associate_acc_chk = 0;
	unsigned uniq_value = 0;
	int nStart = 0,nEnd = 0,nDiff=0,nAppend = 0, nVal =0,nPrevSize=0,nFlag=0;
	char cRejectReason[128] = "";

	bzero (uri, sizeof (uri));
	parse_uri(msg->first_line.u.request.uri.s, msg->first_line.u.request.uri.len, &sipuri);
	
	memcpy(uri, sipuri.user.s, ((sipuri.user.len < 99) ? sipuri.user.len : 99));

	uri_len = strlen (uri);
	if ( uri_len < 2 || uri_len > 98 ) {
		LOG (L_ERR, "[ws_get_dialplan2]: Invalid Phone Number %s\n", uri);
		return send_reply(msg, 484, "Bad Request (Invalid Phone Number)");
	}
	LOG (L_WARN, "[ws_get_dialplan2]uri is *$*$ '%s'*$*$\n",uri);
	
/* The following block replaces the old CHECK_SPECIAL_CHARACTERS.
 * This is more severe on validation the user part of a uri
 */	
#define VALIDATE_URI(uri)	for (i=0;uri[i];i++) {\
	if (! isalnum (uri[i])) {\
		if (NULL == strchr (URI_VALID_CHARS, uri[i])) {\
			LOG (L_ERR, "Invalid URI: %s (Dialed a number with special characters)\n", uri);\
			send_reply(msg, 484, "Bad Request: Invalid number!");\
			return 1;\
		}\
	}\
}
	VALIDATE_URI(uri);

	/* Added for FROM header check , if not found reply with 400 response --vravi */
	if(!msg || (!msg->from)){
		 return send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
	}

	if(Contact_Regcheck){
		if(Validate_Contact(msg) == -1){/*Checking special symbols from contact and from headers,if found send negative response*/
			LOG (L_ERR, "[worksmart_get_dialplan2]: Invalid Cahracters in from uri or in contact\n");
			return send_reply(msg, 484, "Bad Request (Invalid Characters in from header)");
		}
	}
	
	bzero (time_s, sizeof (time_s));
	uniq_value = generate_unique_token();
	snprintf (time_s, sizeof (time_s) - 1, "%010u",uniq_value);

	
#if 0
	/*This checking for finding the call came from the mediaservers or not...*/
	mediapool = ws_check_for_attendedtransfer(msg,uri);
	
	if(mediapool == 1){
		media_failover = 1;
	}else{
		media_failover = 0;
	}
#endif

	utType = get_uri_type(msg, &wsuri, uri,time_s,prio1,sizeof(uri),sizeof(time_s));
	LOG (L_WARN, "[worksmart_get_dialplan2] Type ... %d \n",utType);
	
	warmflag = 0; //Resetting warmflag before processing every request --Swaroopa/Abhilash --08-08-2018
    msg->extra_info.nOtherDataCenterFlag = 0;
	switch ( utType) {

		case NINE11:
			nRes=process_x11_returnedcall(msg,wsuri.command);
			break;
		
		case VM_OPRCALL:
			nRes = process_vmopr_call (msg, prio1 , &wsuri, 0);
			break;

		case Bla_PhoneNumber:
			nRes = process_bla_call (msg, prio1 , &wsuri, 0);
			break;

		/*EXT - EXT Call Via MS --- Vijay Feb2009 */
		case IEXTCALL:
			nRes = ws_ext_call_process (msg, prio1, &wsuri, 0,1,0,IEXTCALL, 0);
			break;
		/* Handling ALL type of group ext calls --- kimo/Ajay Jan 2012 */
		case GEXT :
			nRes = ws_grp_ext_call_process (msg , prio1, &wsuri, 0, 1 , 0, 0);
			break;
		/* Handling Hunt group calls if group have more than max_branches --Anusha */
		case FGRP :
			if(wsuri.command && wsuri.group && strlen(wsuri.command) && strlen(wsuri.group)){
				nStart = atoi(wsuri.command);
				nEnd = atoi(wsuri.group);
				nDiff = nEnd - nStart;
				if(nDiff >= MAX_FORK_BRANCHES) {
					nPrevSize = nDiff/MAX_FORK_BRANCHES + (nDiff%MAX_FORK_BRANCHES > 0 ?1:0);
					nRes = generate_forkgroup(nStart,nEnd,cNewUri,msg,wsuri.context,wsuri.token,nPrevSize,&nAppend,nVal,0,sizeof(cNewUri));/*Nested fork groups are generated*/
				}else{
					nRes = create_fork_branches(msg , &wsuri);/*Branches are created for forking groups*/
				}
			}
			break;
		/*EXT - EXT Call Via MS --- Vijay Feb2009 */
		case ICOMCALL:
			nRes = process_ext_call2 (msg, prio1, &wsuri, 0, 1,0);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
			break;

		case WSPCR:
			snprintf(cAgentid, sizeof(cAgentid) - 1,"%s-%s", wsuri.group, wsuri.context);
			if(modify_from_to_for_buddydialing(msg, cAgentid, wsuri.token) == 1) {
				nRes = 1;
				break;
			}

			nRes = get_direct_extension(cAgentid, 0);
			if(nRes == -1){
				break;		
			}		
			snprintf(tempext,sizeof(tempext)-1,"%d",nRes);
			wsuri.command=tempext;
		case ACDXFER:	
		case Extension :
			if(wsuri.command && atoi(wsuri.command) > 0) {
				/* First check if this is conference extenstion or not, if not process as regular extension --Ramu*/
				nRes = process_conf_call2 (msg, &wsuri,0);
				/* Regular Extension */
				if (nRes == -1){
					if(wsuri.group && !strcmp(wsuri.group,"transfer")){
						associate_acc_chk = ASSOCIATIVE_ACC_CHECK;
					}
					if(wsuri.group && strlen(wsuri.group) >0 && !strcmp(wsuri.group,"streamletivr")){ /*Calling this function directly in case of a group calls...(if group have without exension,fetching details with group code)*/
						nRes = ws_grp_ext_call_process (msg , prio1, &wsuri, 0, 1 , 0, 1);
					}else{
						nRes = ws_ext_call_process (msg, prio1, &wsuri, 0, 1, associate_acc_chk,Extension, 0);
					}
				}
			}
			break;
			/*In this case works the 'act' call's from the media server---tulsi--18/02/2008*/
		case Acccode_Type:
			/*This is to fix the msicp crash issue with SJ Phone transfer reported by ramesh:07-04-08*/
			if(wsuri.context ==  NULL || wsuri.command==NULL || wsuri.group==NULL){
				return -1;
			}
			LOG(L_WARN,"worksmart_get_dialplan2[Acccode_Type]: Sitename:%s\n acccoden:%s\nnumber:%s\n",wsuri.context,wsuri.command,wsuri.group);
			uri_len = strlen(wsuri.command);
			if(uri_len>=7){
					LOG(L_WARN, "worksmart_get_dialplan2: Dialed Number:%s\n", wsuri.command);
					nRes = get_did_plan(msg,prio1,wsuri.command,NULL,0,&wsuri);
			}else if (((uri_len > 2 )&& (uri_len <=5))){
					nRes = process_ext_call2 (msg, prio1, &wsuri, 0, 0,0);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
			}
			break;
																											
		case IVR_Command :
			if(wsuri.command){
				if (!strcmp(wsuri.command, "0")) {/*Got dtmf as 0 for ivr calls */
					/* First call route to 0th destination user or group,if not then call route to operator */
					nRes = get_group_cmd_plan(msg, prio1, &wsuri, 0);/*If nRes =2 then ivr destination(Dest) is group so call process_ivr_command2
																      nRes =-1 did not get Dest ,so call will route to operator
																	  nRes =3 Destination is set as operator,so call get_group_operator_plan*/
					if(nRes == Ivr_Group){
					    nRes = process_ivr_command2 (msg, prio1, &wsuri, 0);
					}else if(nRes == Invalid_Ivr || nRes == Ivr_Operator){
						nRes = get_group_operator_plan(msg, prio1, &wsuri, 0,1);
					}
        			break;				
				}
				else if(!strcmp(wsuri.command, "01")|| strlen (wsuri.command) > 2) {
					if(strcmp(wsuri.command, "001")){
						nFlag=1;
					}
				/* Got operator as destination,if operator is not found call will route to 0th destination user or group*/
					nRes = get_group_operator_plan(msg, prio1, &wsuri, nFlag,0);
					if (nRes == Invalid_Ivr) {/*Operator is not found,so call have to be route 0th destination user or group*/
						strcpy(wsuri.command,"0");			
						nRes = get_group_cmd_plan(msg, prio1, &wsuri, 0);
						if(nRes == Ivr_Group || nRes == Invalid_Ivr){/*If nRes =2 then ivr destination(Dest) is group so call process_ivr_command2*/
			    			nRes = process_ivr_command2 (msg, prio1, &wsuri, 0);
						}
					}
	        		break;				
				}	
				else {/*Got Dtmf as 1 to # for ivr calls */
					nRes = get_group_cmd_plan(msg, prio1, &wsuri, 0);
					if(nRes == Ivr_Group || nRes == Invalid_Ivr){/*If nRes =2 then ivr destination(Dest) is group so call process_ivr_command
											  nRes =3 Destination is set as operator,so call get_group_operator_plan*/
						nRes = process_ivr_command2 (msg, prio1, &wsuri, 0);
					}else if(nRes == Ivr_Operator){
						nRes = get_group_operator_plan(msg, prio1, &wsuri, 0,0);
					}
				break;
				}
			}
			break;
	case VMD_Command :	

			if(wsuri.command && !strcmp(wsuri.command, "01")) {
				LOG(L_WARN, "worksmart_get_dialplan2: [VMD_Command: 0]Dialed Number:%s\n", wsuri.command);
				strcpy(wsuri.command,"0");			
				nRes = get_group_operator_plan(msg, prio1, &wsuri, 0,0);
				if (nRes == -1) {
					nRes = get_group_cmd_plan(msg, prio1, &wsuri, 1);
					if(nRes == 2 || nRes == -1){
			    		nRes = process_ivr_command2 (msg, prio1, &wsuri, 0);
					}
				}
        		break;				
			}	
			break;
		case Xfer4DID:
			nRes = ws_ext_call_process (msg, prio1, &wsuri, 0,0,0,Xfer4DID,0);
			LOG (L_WARN, "[worksmart_get_dialplan2][case Xfer4DID ] res='%d'\n",nRes);
			break;
		case CNF_Request:
			nRes = process_conf_call2 (msg, &wsuri,1);
			break;	
		case ACD_Agent_Extension:
		    nRes = process_acd_agent_call2(msg, prio1, &wsuri);
		    break;
		case PSTN_Transfer :
		    	if(_nOrigSeq > 1 && msg->nRejectRelay > 0){

				if(msg->nRejectRelay > 0 && strlen(msg->cRejectReason) > 0){
					memset(tempBuff, 0, sizeof(tempBuff));
					snprintf(tempBuff, sizeof(tempBuff)-1, "X-Reject-Reason: %d;%s\r\n", msg->nRejectRelay, msg->cRejectReason);
					add_lump_rpl(msg, tempBuff, strlen(tempBuff), LUMP_RPL_HDR);
				}

				memset(tempBuff, 0, sizeof(tempBuff));
				snprintf (tempBuff, sizeof (tempBuff)-1, "tranfail-%d-0-0-0", msg->nRejectRelay);//uri to play robocall detected/auth failed message and disconnect
				LOG(L_ERR,"Sending 603 here...!\n");
				ws_t_reply(msg,(char *)603,"Declined");	
				return 1;
			}
			uri_len = strlen(wsuri.command);/*This added for fix msicp crash on 16-10-08-tulsi-bugid-4720*/
			if(uri_len > 16){
				send_reply(msg,400,"Bad Request Hint: Phone Number too Long");
				LOG (L_ERR, "[ws_get_dialplan2] URI too long: %s\n", uri);
				return 1;
			}
			/*This checking for finding the call for attended transfer...*/
			mediapool = ws_check_for_attendedtransfer(msg,uri);

			if(_nSupSeq == 1)
			{
				memset(buffer, 0 , DIDSIZE);
				strncpy(tempBuff,wsuri.command,sizeof(tempBuff)-1);
				normalize(tempBuff, buffer, sizeof(buffer),sizeof(tempBuff));
				wsuri.command=tempBuff;
				/*Added for mobile call --kkmurthy 29th December 2008*/
				if(mediapool == 1){
					nRes = get_did_plan(msg, prio1, buffer,"transfer",0,&wsuri);
				}else{
					nRes = get_did_plan(msg, prio1, buffer,"transfer",PSTN_Transfer,&wsuri);
				}
			}else{/*else added by tulsi for fix the media server pooling
				Route the call to media server based on the array
				Get did
				Check this did is our did or not
				If yes send this to media server with xfer
				not send to PSTN network
				Neet to set flag for finding the server*/
				warmflag = 1;
				LOG(L_WARN,"[worksmart_get_dialplan2] PSTN_Tranfer warmflag:%d\n",warmflag);
				memset(buffer, 0 , DIDSIZE);
				strncpy(tempBuff,wsuri.command,sizeof(tempBuff)-1);
				normalize(tempBuff, buffer, sizeof(buffer),sizeof(tempBuff));
				wsuri.command=tempBuff;
				if(mediapool == 1){
					nRes = get_did_plan(msg, prio1, buffer,"transfer",0,&wsuri);
				}else{
					nRes = get_did_plan(msg, prio1, buffer,"transfer",PSTN_Transfer,&wsuri);
				}
				media_failover = 1;
			}
			if(nRes == -4)
			{
				/*Dialled NPA number or DID is not our worldsmart number-bugid 4491.-tulsi*/
				/*Route to pstn*/
				memset(buffer, 0 , DIDSIZE);
				strncpy(tempBuff,wsuri.command,sizeof(tempBuff)-1);
				normalize(tempBuff, buffer, sizeof(buffer),sizeof(tempBuff));
				wsuri.command=tempBuff;
				memset(cSitename, 0 , sizeof(cSitename));
				if(wsuri.context)
					strncpy(cSitename,wsuri.context,sizeof(cSitename)-1);
				if(isdigit(cSitename[0]) && (ptr = strchr(cSitename,'-')))
					wsuri.context = ++ptr;
				nRes = route_to_outboundserver1( msg,buffer,&wsuri,"transfer");
			}
			break;
		case Pickup_Request :
			nRes = worksmart_set_pickup_extension(msg, prio1, prio2,wsuri.token);
			//Route to pkp-blah blah blah @media server
			break;
		case ACD_Request :
			//Route to acd-blah blah blah @media server
			break;
		case DIR_Request :
			//Route to dir-blah-blah @media server
			nRes = process_dir_request (msg, &wsuri);
			break;
		case INV_Request :
			//Route to inv-blah-blah @media server
			bzero(ruri_out,sizeof(ruri_out));
			snprintf(ruri_out,sizeof(ruri_out)-1,"inv-0-0-pandoranetworks-com-%s",wsuri.token);
			nRes = route_to_server(msg,ruri_out,umslb_proxy,MS);
			break;
		case ACD_Voicemail:
			//Route to vmln-blah-blah @media server
		    bzero(uri,sizeof(uri));
		    snprintf(uri,sizeof(uri)-1,"%s-%s-%s-%s-%s",wsuri.type,wsuri.command,wsuri.group,wsuri.context,wsuri.token);
		    nRes = route_to_server(msg,uri,umslb_proxy,MS);
        	break;
		case ACD_Voicemail_Transfer:
		    //Route to vmlo-blah-blah @media server
		    bzero(uri,sizeof(uri));
		    snprintf(uri,sizeof(uri)-1,"%s-%s-%s-%s-%s",wsuri.type,wsuri.command,wsuri.group,wsuri.context,wsuri.token);
		    nRes = route_to_server(msg,uri,umslb_proxy,MS);
		    break;
		case ACD_Agent:
		    //Route to agt-blah-blah@acd_server
		    bzero(uri,sizeof(uri));
		    snprintf(uri,sizeof(uri)-1,"%s-%s-%s-%s-%s",wsuri.type,wsuri.command,wsuri.group,wsuri.context,wsuri.token);
	        nRes = route_to_server(msg,uri,umslb_proxy,ACDMS);
		    break;
		case ACD_DID:
			memset(buffer, 0 , DIDSIZE);
			strncpy(tempBuff,wsuri.command,sizeof(tempBuff)-1);
            normalize(tempBuff, buffer, sizeof(buffer),sizeof(tempBuff));
			wsuri.command=tempBuff;
			/*nRes = route_to_pstn(msg, buffer);*/
			nRes = get_did_plan(msg, prio1, buffer,wsuri.context,ACD_DID,&wsuri);
			break;	
		case MWI_Request:
			//Route to mwi-ext-sitename@media_server
			bzero(uri,sizeof(uri));
			snprintf(uri,sizeof(uri)-1,"%s-%s-%s-%s-%s",wsuri.type,wsuri.command,wsuri.group,wsuri.context,wsuri.token);
			nRes = route_to_server(msg,uri,umslb_proxy,MS);
			break;
		case Push_To_Talk:
			//Route to ptt-conference room@confserver
			bzero(uri,sizeof(uri));
			snprintf(uri,sizeof(uri)-1,"%s-0-%s-%s-%s",wsuri.type,wsuri.group,wsuri.context,wsuri.token);
			nRes = route_to_server(msg,uri,conference_server,OTHER);
			break;
		case Phone_Number :
			// 1. Check if this is an internal phone number ( phone_numbers )
			//		Yes-> 1.1 Check if this is assigned to any extended group or extension
			//			Yes-> 1.1.1 Get greeting(if ext group) and foward to media server
			//			Yes-> 1.1.1 Get His dialplan ( if it is a direct DID )
			//			 No-> 1.1.2 Reply 404
			//		No-> 1.2 Check if he has ability to call out
			//			Yes-> 1.2.1 Forward to pstn
			//			 No-> 1.2.2 Reply 403 forbidden or 5xx service unavailable
			LOG(L_WARN, "worksmart_get_dialplan2: %s\n", uri);
			uri_len = strlen(uri);
			LOG(L_WARN, "[worksmart_get_dialplan2] uri length '%d'\n",uri_len);
			if (uri_len > 12) {
				send_reply(msg,400,"Bad Request Hint: Phone Number too Long");
				LOG (L_WARN, "[ws_get_dialplan2] URI too long: %s\n", uri);
				return 1;
			}
			nRes = get_did_plan(msg, prio1, uri,NULL,0,&wsuri);
			 LOG (L_WARN, "[ws_get_dialplan2] [case: Phone_Number] res is '%d'\n",nRes);
			break;
		case International_Phnumber :
        	// Check the class of services for this user and set callerid
		    LOG(L_WARN, "[International_Phnumber] worksmart_get_dialplan2: %s\n", uri);
			if ( set_rpid_from_user(msg, uri, NULL, wsuri.token) ) {
		    	/*nRes = route_to_pstn(msg, uri);*/
				nRes = route_to_outboundserver1( msg,uri,&wsuri,NULL);
			}
			else {
				bzero(ruri_out,sizeof(ruri_out));
				snprintf(ruri_out,sizeof(ruri_out)-1,"cos-0-0-pandoranetworks-com-%s",wsuri.token);
			    nRes = route_to_server (msg,ruri_out,umslb_proxy,MS);	
		    }
			break;
		case Feature_Request :
			//Route to media server
			nRes = route_to_feature_server (msg, atoi(wsuri.command),wsuri.token);
			break;
		case WS_URI :
			//What to do with you baby?
			break;
		case Local_User:
		case CTC_Request:
		    // 3pcc call just Lookup if not found return - 404
		    if(ws_ctccall_flag){
		    	nRes = process_3pccsup_call(msg,uri,_nSupSeq);
			}else{
		    	nRes = process_3pcc_call(msg,uri);
			}
		    break;
		case Disaster_Request:
			// Authenticated Request from MS from enabling Disaster in Proxy 
			nRes = ws_systemwide_disaster(msg,wsuri.token); 
			break;
		
		case OB_Transfer:
			nRes = process_conf_call2 (msg, &wsuri,0);
			if(nRes == -1){
				nRes = ws_ext_call_process (msg, prio1, &wsuri, 0,0,ASSOCIATIVE_ACC_CHECK,OB_Transfer, 0);
			}
			 break;

		case MOBILECALL:
			nRes = process_mobilecall(msg, prio1, &wsuri);
			if(nRes == -1){
				LOG (L_WARN, "[ws_get_dialplan2] Mobile call user is not found: %s\n", uri);
				return send_reply(msg,603,"Declined due to lack mobile call previleges");
			}
			break;
		case ACEFORWARD:
			LOG (L_WARN, "[ws_get_dialplan2] '%s', '%s' \n", wsuri.group, wsuri.context);
			nRes = process_aceforwardcall(msg, prio1, &wsuri);
			if(nRes == -1){
					LOG (L_WARN, "[ws_get_dialplan2] aceforward call to particular user phone type is not found: %s\n", uri);
					return send_reply(msg,404,"User Not Found");
			}
			break;
		case SIX11:
			/*To handle 611 TransferCalls --Jagan*/
			nRes = process_611_calls(msg, &wsuri, prio1);
			break;
		case LCR_Request:

			if((badnumber_reject_code > 0) && wsuri.command && !isInList(specialnumbers , wsuri.command) && ((strlen(wsuri.command) < 10 ) || ((strlen(wsuri.command) >= 10) && ( strncmp(wsuri.command,"011",3)) && ( strncmp(wsuri.command,"+1",2)) && ( wsuri.command[0] == '+' ? wsuri.command[1] == '0': wsuri.command[0] == '0')))){
				memset(cRejectReason,0,sizeof(cRejectReason));
				snprintf(cRejectReason, sizeof(cRejectReason)-1,"X-Reject-Reason: 6;%s\r\n","invalid-dialled-number");
				add_lump_rpl(msg, cRejectReason, strlen(cRejectReason), LUMP_RPL_HDR);
				send_reply(msg,badnumber_reject_code,"Declined");
				LOG (L_WARN, "[ws_get_dialplan2] TO number is less than 10 digit or its contains 0 prefix : %s\n", uri);
				return 1;
			}
			/*
			if(isFruadTriggered(msg)){
					memset(cRejectReason,0,sizeof(cRejectReason));
					snprintf(cRejectReason, sizeof(cRejectReason)-1,"X-Reject-Reason: 7;%s\r\n","fraud_triggered");
					add_lump_rpl(msg, cRejectReason, strlen(cRejectReason), LUMP_RPL_HDR);
					send_reply(msg,badnumber_reject_code,"Declined");
					return 1;
			}
			*/

			if(atoi(prio1) == 1) {
				preparePAIheader(msg);
			}
			nRes = route_to_fwdout_server(msg, prio1 , &wsuri);
			break;
		default:
			//what to do with you baby?
			break;
	};

	if ( nRes > 0 ) {
		if (nRes != 2 && atoi(prio1) == 1 && atoi(prio2) == 1) {
			if ( ws_on_reply ) {
				ws_on_reply(msg, (char *) 1, NULL);
			} else {
				LOG(L_ERR, "ws_on_reply is NULL\n");
			}
			//use_rtp_proxy(msg);
		} else if (nRes != 2 ) {
			if ( ws_on_reply ) {
				ws_on_reply(msg, (char *) 1, NULL);
			} else {
		                LOG(L_ERR, "ws_on_reply is NULL\n");
            		}
		} 		
		if ( /*uType == ACD_Agent_Extension  || utype == IVR_Command ||*/ nRes == 2 ) {
			if ( ws_on_reply ) {
				ws_on_reply(msg, (char *) 2, NULL);
			} else {
		                LOG(L_ERR, "**ws_on_reply is NULL\n");
			}
		}
		if ( !_nNoRelay &&  t_relay ) {
			if ( t_relay(msg, NULL, NULL) < 0 ) {
				LOG(L_ERR, "**--** Relay failed 1\n");
			}
		}
	}
	LOG(L_ERR,"[ws_get_dialplan2] res- '%d'\n",nRes);
	memset(&group_name, 0, sizeof(group_name));
	return (nRes>0?1:nRes);
}

int is_permited (char * extension, char * context, char * dstphone ) {
   char * national_formatstring = ""
								"SELECT STRAIGHT_JOIN callerid"
								"  FROM pbxusers p, classofrules c, agent a"
								" WHERE c.outboundlocalldzoneid = 0"
								"    AND p.siteid = c.siteid"
								"    AND c.id = a.classid"
								"    AND a.agentid = p.agentid"
								"    AND REPLACE(REPLACE(sitename, '_', '-'), '.', '-') = '%s'"
								"    AND p.extensions = %s"
								" UNION "
								"SELECT STRAIGHT_JOIN callerid"
								"  FROM  pbxusers p, agent a, classofrules c, ldzonecodes ld, state_codes s"
								" WHERE p.siteid = c.siteid"
								"    AND c.id = a.classid"
								"    AND a.agentid = p.agentid"
								"    AND REPLACE(REPLACE(sitename, '_', '-'), '.', '-') = '%s'"
								"    AND p.extensions = %s"
								"    AND code = %s"
								"    AND s.scid = ld.scid"
								"    AND ld.zoneid = c.outboundlocalldzoneid";

	/* Class of Services for International Calls.  
	 * First get the country code then test his class of rules Ramu Date:12/12/05 
	*/
	char * formatstring = ""
			"SELECT countrycode FROM international_tariff  "
			" WHERE '%s' LIKE CONCAT(zonecode, '%')  "
			" ORDER BY LENGTH( zonecode ) DESC LIMIT 1 ";

	char * international_formatstring = ""
									"SELECT STRAIGHT_JOIN callerid "
									"  FROM pbxusers p, agent a , classofrules c, siteinfo_new si "
									" WHERE c.outboundintezoneid = 0 "
									"   AND a.extensions = %s"
									"   AND REPLACE(REPLACE(si.sitename, '_','-'),'.','-') = '%s'"
									"   AND si.intlenabled = 1 "
									"   AND si.siteid = a.siteid "
									"   AND a.siteid = c.siteid "
									"   AND c.id = a.classid "
									"   AND a.agentid = p.agentid "
									"UNION  "
									"SELECT STRAIGHT_JOIN callerid "
									"  FROM pbxusers p, agent a, classofrules c, siteinfo_new si, countries co, intzonecodes i"
									" WHERE a.extensions = %s"
									"   AND REPLACE(REPLACE(si.sitename, '_','-'),'.','-') = '%s'"
									"   AND si.intlenabled = 1 "
									"   AND si.siteid = a.siteid "
									"   AND a.siteid = c.siteid "
									"   AND c.id = a.classid "
									"   AND a.agentid = p.agentid "
									"   AND c.outboundintezoneid = i.zoneid "
									"   AND co.ccode = '%s'"
									"   AND i.contrycode = co.cid";
									
    char pCode[8];
    char query[1024];
    db_res_t *res=NULL;
    db_row_t row;
	char result[16];
    bzero(pCode, sizeof(pCode));
    strncpy(pCode, dstphone + 1, 3);

	// Check is this International phone number or not?
	if(!strncmp(dstphone, international_prefix, prefix_length)) {
		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1, formatstring, dstphone);		
		//LOG (L_ERR,"[is_permited] Query1:\n%s\n", query);
		ws_dbf.raw_query (h, query, &res);
		if (RES_ROW_N(res) > 0) {
			row = RES_ROWS(res)[0];
			bzero (result, 10);
			strncpy(result,row.values[0].val.string_val,sizeof(result)-1);
			if (res)
				ws_dbf.free_result (h,res);
  		}
		else {
			if (res)
				ws_dbf.free_result (h,res);
			return 0;
		}
		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1, international_formatstring, extension, context, extension, context, result);
	} else {
		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1, national_formatstring, context, extension, context, extension, pCode);
	}
    //LOG(L_ERR, "[is_permited] Query:\n %s\n", query);
    ws_dbf.raw_query (h, query, &res);
	if ( !res ) {
		return 0;
	}
    if ( RES_ROW_N(res) <= 0 )  {
        if (res)
        	ws_dbf.free_result(h,res);
        return 0;
    }
    return 1;
}

int Get_User_Details(char *cAgentId, char *cSiteName, char *cExt, int nSize) {
		
	char cQuery[1024] = "";
	db_res_t *res=NULL;
	db_row_t row;

	char *cSelectExtFromPbxId = "SELECT extensions from pbxusers where agentid = '%s' and sitename = '%s'";

	memset(cExt, 0, nSize);
	memset(cQuery,0,sizeof(cQuery));

	if(cAgentId == NULL || strlen(cAgentId) == 0 || cSiteName == NULL || strlen(cSiteName) == 0) {
		LOG(L_ERR,"[Get_User_Details] Input parameters are emoty \n");
		return -1;	
	}

	snprintf(cQuery, sizeof(cQuery)-1, cSelectExtFromPbxId, cAgentId, cSiteName);	

	ws_dbf.raw_query(h, cQuery, &res);

	if(res==NULL){
		LOG(L_ERR,"[Get_User_Details] no result set with query:%s\n",cQuery);
		return -1;
	}

	if(res && RES_ROW_N(res) > 0 ){
		row = RES_ROWS(res)[0];
		if(!VAL_NULL(&row.values[0])){
			snprintf(cExt, nSize-1,"%d", get_db_int (row.values[0]));
		}
	}

	if (res) {
		ws_dbf.free_result (h,res);
		res = NULL;
	}

	return 1;
}

int Prepare_Contact_for_COS_302(struct sip_msg *msg, char *cForwardFromUser, char *cToken) {

	char *ptr = NULL;
	char cContactHeader[256] = "", cContactIp[32]="", cExt[32] = "", cSiteName[128] = "";

	memset(cSiteName, 0, sizeof(cSiteName));
	memset(cContactHeader, 0, sizeof(cContactHeader));
	memset(cContactIp,0,sizeof(cContactIp));

	
	if(msg == NULL || cForwardFromUser == NULL || cToken == NULL || strlen(cForwardFromUser) == 0 || strlen(cToken) == 0) {
		LOG(L_ERR," [Prepare_Contact_for_COS_302] Input paramenters are empty \n");
		return -1;
	}

	/* parsing contact header to get server Ip */
	if( parse_headers (msg, HDR_CONTACT, 0) == -1) {
		LOG(L_ERR," [Prepare_Contact_for_COS_302] parsing contact header failed \n");
		return -1;
	}

	if(msg->contact == NULL || msg->contact->body.len == 0 ) {
		LOG(L_ERR," [Prepare_Contact_for_COS_302] contact is null \n");
		return -1;
	}

	snprintf(cContactHeader, sizeof(cContactHeader)-1, "%.*s", msg->contact->body.len, msg->contact->body.s);
	ptr = strchr(cContactHeader,'@');
	if(ptr && ++ptr) {
		strncpy(cContactIp,ptr,sizeof(cContactIp)-1);   // Server IP
	}

	if(strlen(cContactIp) == 0) {
		LOG(L_ERR," [Prepare_Contact_for_COS_302] Ip in contact is empty : cContactHeader %s  \n", cContactHeader);
		return -1;
	}

	ptr = strchr(cForwardFromUser, '@');
	if(ptr && ++ptr) {
		strncpy(cSiteName, ptr, sizeof(cSiteName)-1);
	}

	if(Get_User_Details(cForwardFromUser, cSiteName, cExt, sizeof(cExt)) == -1) {
		LOG(L_ERR," [Prepare_Contact_for_COS_302] Get_User_Details is returning -1. cExt : %s, cSiteName: %s, cForwardFromUser %s \n", cExt, cSiteName,cForwardFromUser);
		return -1;
	}

	while((ptr = strchr(cSiteName,'.'))) {
		*ptr = '-';
	}

	memset(cContactHeader, 0, sizeof(cContactHeader));
	snprintf(cContactHeader, sizeof(cContactHeader)-1,"Contact: <sip:vml-%s-404-%s-%s@%s>\r\n",cExt,cSiteName,cToken,cContactIp);
	if (add_lump_rpl(msg, cContactHeader, strlen(cContactHeader), LUMP_RPL_HDR) == 0) {
		LOG(L_ERR,"[Prepare_Contact_for_COS_302] append_hf failure Contact: %s \n", cContactHeader);
		return -1;
	}

	if (racc_302)
	{
		memset(cContactHeader, 0, sizeof(cContactHeader));
		snprintf(cContactHeader, sizeof(cContactHeader)-1,"vml-%s-404-%s-%s\n",cExt,cSiteName,cToken);
		racc_302(msg, cContactHeader, "ClassOfService");
	}

	LOG(L_ERR,"[Prepare_Contact_for_COS_302] Preapred contact header and sending 302 cContactHeader : %s \n", cContactHeader);
	return 0;
}

int FunCheckWSDID(char *cDid) {

	char cQuery[256] = "";
	char * cAccountDIDQuery = "SELECT 1 from accountphonenumbers where phnumber = '%s'";
    
	db_res_t *res=NULL;

	memset(cQuery,0,sizeof(cQuery));

	if(cDid == NULL || cDid[0] != '1' || strlen(cDid)!=11) {
		return 0;
	}

	snprintf(cQuery, sizeof(cQuery)-1,cAccountDIDQuery, cDid);		
	ws_dbf.raw_query (h, cQuery, &res);
	if(res){
		if(RES_ROW_N(res) > 0 ){
			ws_dbf.free_result (h,res);
			res = NULL; 
			return 1;	
		}
		ws_dbf.free_result (h,res);
		res = NULL;
	}
	return 0;
}

/*Optimized the function --kkmurthy 2009-10-08*/
int set_rpid_from_user(struct sip_msg *msg, char * dstphone, char *cTransferred_Agentd, char *cToken ) {

	/* Class of Services for International Calls.  
	 * First get the country code then test his class of rules Ramu Date:12/12/05 
	*/
	char * formatstring = ""
			"SELECT countrycode FROM international_tariff  "
			" WHERE '%s' LIKE CONCAT(zonecode, '%')  "
			" ORDER BY LENGTH( zonecode ) DESC LIMIT 1 ";

	char * intlnanpa_formatstring = ""
			"SELECT countrycode FROM nanpa_tariff  "
			" WHERE '%s' LIKE CONCAT(zonecode, '%')  "
			" ORDER BY LENGTH( zonecode ) DESC LIMIT 1 ";


    char pCode[8]="";
    char query[4096]="";
    char rpid[256]="";
    char fromuser[256]="";
    struct to_body * fromb;
    struct sip_uri uri;
    db_res_t *res=NULL;
    db_row_t row;
	char result[16]="";
	int flag=0,found = 0;
	char *dash = NULL;
	struct ws_uri wsuri;
	char cTmp[256] = "";
	char cForwardFromUser[256] = "", cAgentId[256] = "";
	int nSkipClassofRulesFlag = 0;
	int nIsDisasterCase = 0;
	int nIsFromIsDigit = 0;
	char intlphone[32] = "", *ptr = NULL, cBuffer[256] = "", cOrgFrom[256] = "";

	if(!msg || !msg->from){
		LOG(L_WARN,"[set_rpid_from_user]either msg or from value is NULL\n");
		send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		_nNoRelay = 1;
         return -1;
	}

    if (! msg->from->parsed )
	   	parse_from_header(msg);
    fromb = get_from (msg);
    if (fromb == NULL) {
        send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		_nNoRelay = 1;
         return -1;
    }
	parse_uri(fromb->uri.s, fromb->uri.len, &uri);
	if(uri.user.s == NULL || uri.user.len == 0){
		send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		_nNoRelay = 1;
		return -1;
	}
	snprintf(fromuser, sizeof(fromuser)-1, "%.*s", uri.user.len,uri.user.s);
	
	strncpy(cOrgFrom, fromuser, sizeof(cOrgFrom)-1);

	if ( nEnableCosTransfer && ((isdigit(fromuser[0]))||(fromuser[0]=='+') || (strchr(fromuser,'-') == NULL))) {
		/* If from is pstn or callerid is annonymous */
		nIsFromIsDigit = 1;
	}

	//if(nEnableCosTransfer && cToken != NULL && strncmp(cToken,"wspib",5) == 0) {
	memset(cForwardFromUser, 0, sizeof(cForwardFromUser));
	if(nEnableCosTransfer) {
		if( cTransferred_Agentd && strlen(cTransferred_Agentd)>0){
			if( FunCheckWSDID(dstphone) == 1) {
				return 1;
			}
			/* Added for disatser/routing cases -- jun-30-2020 */
			LOG(L_WARN,"[set_rpid_from_user] cTransferred_Agentd : '%s' cToken %s \n",cTransferred_Agentd, cToken);
			memset(fromuser, 0x00, sizeof(fromuser));
			strncpy(fromuser, cTransferred_Agentd, sizeof(fromuser)-1);
			strncpy(cForwardFromUser,cTransferred_Agentd, sizeof(cForwardFromUser)-1);
			nIsDisasterCase = 1;
		}else if(is_header_exists(msg,"ForwardFromUser", 15, cForwardFromUser, sizeof(cForwardFromUser))){
			while((ptr = strchr(cForwardFromUser,'.'))) { /* Replacing '.' with '-' */
				*ptr = '-';
			}		
			LOG(L_WARN,"[set_rpid_from_user] ForwardFromUser header exists: '%s' fromuser: '%s' \n",cForwardFromUser, fromuser);
			memset(fromuser, 0x00, sizeof(fromuser));
			strncpy(fromuser, cForwardFromUser, sizeof(fromuser)-1);
		}
	}

	/*enum UriType utType;*/
    strncpy(pCode, dstphone + 1, 3);
	/*check the from user is number or username-added by tulsi*/
	/*+14152336767 or 14152336767*/
	LOG(L_WARN,"[set_rpid_from_user]From user name is <%s>\n",fromuser);

	/*Checking AAPSTN header existence to skip class of rules check in case of AA to PSTN destination --Swaroopa/Vishal */
	memset(cTmp, 0x00, sizeof(cTmp));
	if(is_header_exists(msg,"AAPSTN",6,cTmp,sizeof(cTmp))){
		nSkipClassofRulesFlag = 1;
		/*Remove the header here*/
		
		str cmd1={"AAPSTN",6};
		if (remove_hf) {
			if(remove_hf(msg, (char *)&cmd1,0)==1)
				LOG(L_WARN,"remove_hf AAPSTN success\n");
			else
				LOG(L_ERR,"remove_hf AAPSTN failure\n");
		}
	}

	if ((isdigit(fromuser[0]))||(fromuser[0]=='+')) /*Validating is this call came from the PSTN*/
	{
		LOG(L_WARN,"[set_rpid_from_user]From user name is digit valuse no need to check the cos...:%s\n",fromuser);
		if(strchr(fromuser,'-'))
			return 1;	
		return -3;
	}else{

		if(nSkipClassofRulesFlag ==1){
			return -3;
		}
		memset(&wsuri,0x00,sizeof(wsuri));
		dash = strchr(fromuser,'-');
		if(dash == NULL){ /*Validating is the user is having caller id block or not*/
			LOG(L_WARN,"[set_rpid_from_user]From user name is Restricted or Anonymous, no need to check cos:%s\n",fromuser);
			return -3;
		}else if ( !parse_ws_uri_buffer(fromuser, strlen(fromuser), &wsuri) ) { /*return 0, if the uri is worldsmart*/
			/*Not now..if not possible from media server end-Tulsi*/
			if(!strcmp(wsuri.type,"cnf")){
				LOG(L_ERR,"[set_rpid_from_user]is transfer request from conference call? from URI:%s\n",fromuser);
				return -1;
			}
			return -3;
		}else{/*This is the original username, find the Actual callid*/
			LOG(L_ERR,"[set_rpid_from_user]From user name is <%s>\n",fromuser);
		}
	}

	memset(cAgentId, 0, sizeof(cAgentId));
	if(nEnableCosTransfer) {
		strncpy(cAgentId, fromuser, sizeof(cAgentId)-1);
		_get_agentid(msg,cAgentId,NULL);
	}

	// Check is this International phone number or not?
	if(!strncmp(dstphone, international_prefix, prefix_length)) {
		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1,formatstring, dstphone);		
		ws_dbf.raw_query (h, query, &res);

		if(res && RES_ROW_N(res) > 0 ){
			row = RES_ROWS(res)[0];
			if(!VAL_NULL(&row.values[0])){
				memset(result,0,sizeof(result));
				strncpy(result,row.values[0].val.string_val,sizeof(result)-1);
			}
		}
		if (res) {
			ws_dbf.free_result (h,res);
			res = NULL;
		}
		bzero(query,sizeof(query));
		if(nEnableCosTransfer == 1) {
			snprintf(query, sizeof(query)-1,INTERNATIONAL_QUERY, cAgentId, cAgentId, result, cAgentId, cAgentId, result);
		}else{
			snprintf(query, sizeof(query)-1,OLD_INTERNATIONAL_QUERY, fromuser, fromuser, result);
		}
	} else {
		memset(query,0,sizeof(query));
		if(nEnableCosTransfer == 1) {
			memset(intlphone, 0, sizeof(intlphone));
			snprintf(intlphone,sizeof(intlphone)-1,"011%s",pCode);
			snprintf(query, sizeof(query)-1,intlnanpa_formatstring, intlphone);
			LOG(L_WARN, "[set_rpid_from_user] Query: %s\n", query);
			ws_dbf.raw_query (h, query, &res);

			memset(query,0,sizeof(query));
			if(res && RES_ROW_N(res) > 0) {
				row = RES_ROWS(res)[0];
				if(!VAL_NULL (&row.values[0])){
					memset(result,0,sizeof(result));
					strncpy(result,row.values[0].val.string_val,sizeof(result)-1);
				}
				snprintf(query,sizeof(query)-1, NANPAQUERY, cAgentId, cAgentId, result, cAgentId, cAgentId, result);
			}else{
				snprintf(query, sizeof(query)-1,NATIONAL_QUERY, cAgentId, cAgentId, pCode, cAgentId, cAgentId, pCode);
			}
			if (res) {
				ws_dbf.free_result (h,res);
				res = NULL;
			}
		}else{
			if(nEnableCosTransfer == 1) {
				snprintf(query, sizeof(query)-1,NATIONAL_QUERY, cAgentId, cAgentId, pCode, cAgentId, cAgentId, pCode);
			}else{
				snprintf(query, sizeof(query)-1,OLD_NATIONAL_QUERY, fromuser, fromuser, pCode);
			}
		}
	}
    LOG(L_WARN, "[set_rpid_from_user] Query: %s\n", query);
    ws_dbf.raw_query (h, query, &res);
    if ( !res || RES_ROW_N(res) <= 0 )  {
        if (res) {
        	ws_dbf.free_result(h,res);
			res = NULL;
		}
	
		if(nEnableCosTransfer && strlen(cForwardFromUser) > 0) {
			return Prepare_Contact_for_COS_302(msg, cAgentId, cToken);
		}
		return 0;  /* User doesn't have cos privilages */
	}
    /* To fix the crash: 22-Sep-2005 */
	row = RES_ROWS(res)[0];
	if (row.values[1].nul == FALSE) {
		flag = get_db_int(row.values[1]);
	}
	if(flag != 1){
		if (row.values[0].nul == FALSE) {
			bzero(fromuser,sizeof(fromuser));
	    	strncpy(fromuser, row.values[0].val.string_val,sizeof(fromuser)-1);
			found = 1;	
		}
	}
    if (res){
	   	ws_dbf.free_result(h,res);
		res = NULL;
	}
	if(!found){
		bzero(fromuser,sizeof(fromuser));
		strncpy(fromuser, "Anonymous",sizeof(fromuser)-1);
	}

	/* we should not apped rpid in disater case..as we are newly calling this fun for "cos" patch.
	 * If nIsFromIsDigit is '0', then also we should not add as we are returning "-3" previously above ...before "cos" patch */
	if(nIsDisasterCase == 0 && nIsFromIsDigit == 0) {
		memset(rpid,0,sizeof(rpid));
		if(!strcmp(fromuser,"Anonymous") || !strcmp(fromuser,"Restricted")) {
			snprintf(rpid, sizeof(rpid)-1,"Remote-Party-ID: <sip:%s@%s>\r\n", fromuser, proxy_domain);
			if(strlen(cOrgFrom) > 0 && (strcmp(cOrgFrom,"Anonymous") && strcmp(cOrgFrom,"Restricted"))) {
				/*Removing old P-Asserted-Identity header*/
				str cmd1={"P-Asserted-Identity",19};
				if (remove_hf && remove_hf(msg, (char *)&cmd1,0) !=1) {
					LOG(L_ERR,"[set_rpid_from_user] P-Asserted-Identity remove_hf failure\n");
				}

				/*Adding new P-Asserted-Identity header*/
				memset(cBuffer,0,sizeof(cBuffer));
				snprintf(cBuffer,sizeof(cBuffer)-1,"P-Asserted-Identity: <sip:%s@%s>\r\n", cOrgFrom, proxy_domain);

				str cmd={cBuffer,strlen(cBuffer)};
				if(append_hf(msg, (char *)&cmd,0)!=1){
					LOG(L_ERR,"[set_rpid_from_user] P-Asserted-Identity append_hf failure >> '%s'\n",cBuffer);
				}
			}
		}else{
			snprintf(rpid, sizeof(rpid)-1,"Remote-Party-ID: <sip:+%s@%s>\r\n", fromuser, proxy_domain);
		}
		LOG(L_WARN, "[set_rpid_from_user: Remote-Party-ID: ] %s\n", rpid);
		str cmd1={rpid,strlen(rpid)};
		if (append_hf) {
			if(append_hf(msg, (char *)&cmd1,0)==1)
				LOG(L_WARN,"append_hf success\n");
			else
				LOG(L_WARN,"append_hf failure\n");
		}

	}
	/* Added for "cos" patch */
	if(nIsFromIsDigit == 1) { /* This is PSTN ONBOUND call  and  Tansfering user has cos privilage  */
			return -3;
	}

    return 1;
}

int process_dir_request (struct sip_msg *msg, struct ws_uri *wsuri){

    char uri[100];
	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group || !wsuri->token ) {
		return -1;
	}
	bzero (uri,sizeof(uri));
	snprintf(uri, sizeof(uri)-1, "dir-%s-0-%s-%s",wsuri->command, wsuri->context,wsuri->token);
	LOG(L_WARN, "[process_dir_request --URI] %s\n", uri);
	route_to_server (msg, uri, umslb_proxy,MS);
	return 1;
}

int process_3pcc_call (struct sip_msg *msg, char *username) {
	if (!username)
		return -1;
	char * format_string =  " SELECT contact FROM location "
						"  WHERE username like '%s' ";
	char query[512];
	char result[256];
	db_res_t *res=NULL;
	db_row_t row;

	bzero(query,sizeof(query));
	snprintf (query, sizeof(query)-1, format_string, username);
	LOG(L_WARN, "[process_3pcc_call] %s\n", query);
	ws_dbf.raw_query (h, query, &res);

	if(res==NULL){
		LOG(L_ERR,"[process_3pcc_call]'res' is NULL");
		return -1;
	}
	if (RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		bzero (result, sizeof(result));
		strncpy(result,row.values[0].val.string_val,sizeof(result)-1);
		if (res) {
			ws_dbf.free_result (h,res);
		}
		set_uri2(msg, result);
	}
	else {
		if (res) {
			ws_dbf.free_result (h,res);
		}
		send_reply(msg, 404, "Requested Party Doesn't Exist Hint:P3C-1");
		_nNoRelay = 1;
	}
	return 1;
}


/* Added for ACD supervisory & CTC call routing changes --vravi */
int process_3pccsup_call (struct sip_msg *msg, char *username,int _nSupSeq) {
  
	int ni = 0;
	int ret = 0, nRes = -1;
	str aor;
	struct ws_uri wsuri,wsuri1;
	char actual_user[256]="",tcontact[256]="",buffer[DIDSIZE]="",fack_uri[256]="",tempBuff[256]="";
	char *tmp =NULL, *cSitename =NULL;

	if (!username || !msg){
		LOG(L_ERR , "[process_3pccsup_call] Username is NULL unbale to process\n");
		return -1;
	}
	memset(&wsuri,0x00,sizeof(wsuri));
	if (parse_ws_uri_buffer(username, strlen(username), &wsuri) < 0) {
		return send_reply(msg,404, "Agent Not Found: Hint: Not a valid WSURI (3PCC)");
	}
	LOG(L_ERR , "[process_3pccsup_call] username here is : '%s' :PARSED WSRUI : TYPE < %s > ,COMMAND : < %s >, GROUP < %s >, CONTEXT : < %s >, TOKEN: < %s > \n" , username,wsuri.type,wsuri.command,wsuri.group,wsuri.context,wsuri.token);
	snprintf(actual_user,sizeof(actual_user)-1,"%s-%s",wsuri.group,wsuri.context);
	aor.s = actual_user;
	aor.len = strlen(actual_user);
	
	if(wsuri.group && (strlen(wsuri.group) > 5) &&  isdigit(wsuri.group[0])){

		if (parse_headers (msg, HDR_CONTACT, 0) == -1)
		{
			LOG(L_WARN,"[process_3pccsup_call] Unable to parse Contact HDR \n");
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}
		if (!msg->contact)
		{
			LOG(L_WARN,"[process_3pccsup_call] No Contact Found\n");
			send_reply(msg, 500, "Internal Server Error");
			_nNoRelay = 1;
			return 1;
		}
		if(msg->contact->body.len > 0)
		{
			snprintf(tcontact, sizeof(tcontact)-1, "%.*s", msg->contact->body.len, msg->contact->body.s);
			tcontact[msg->contact->body.len] = '\0';
		}
		if ((tmp = strchr(tcontact , '@'))){
			*tmp = '\0';
			if((tmp = strchr(tcontact , '-'))){
				tmp++;
				cSitename = tmp;
			}
		}
		/* New optimization changes --vravi */
		if( strlen(wsuri.group) > 16){
			send_reply(msg,400,"Bad Request Hint: Phone Number too Long");
			LOG (L_ERR, "[process_3pccsup_call] URI too long: %s\n", wsuri.group);
			return 1;
		}
		snprintf(fack_uri,sizeof(fack_uri)-1,"%s-%s-%s-%s-%s","pst",wsuri.group,wsuri.command,cSitename,wsuri.token);
		memset(&wsuri1,0x00,sizeof(wsuri1));
		if (parse_ws_uri_buffer(username, strlen(username), &wsuri1) < 0) {
			return send_reply(msg,404, "Agent Not Found: Hint: Not a valid WSURI (3PCC)");
		}
		if(_nSupSeq == 1)
		{
			memset(buffer, 0 , DIDSIZE);
			strncpy(tempBuff,wsuri.group,sizeof(tempBuff)-1);
			normalize(tempBuff, buffer, sizeof(buffer),sizeof(tempBuff));
			wsuri.group=tempBuff;
			nRes = get_did_plan(msg,(char *) 1 , buffer,"transfer",PSTN_Transfer,&wsuri1);
		}else{
			warmflag = 1;
			LOG(L_WARN,"[worksmart_get_dialplan2] PSTN_Tranfer warmflag:%d\n",warmflag);
			memset(buffer, 0 , DIDSIZE);
			strncpy(tempBuff,wsuri.group,sizeof(tempBuff)-1);
			normalize(tempBuff, buffer, sizeof(buffer),sizeof(tempBuff));
			wsuri.group=tempBuff;
			nRes = get_did_plan(msg, (char *) 1, buffer,"transfer",PSTN_Transfer,&wsuri1);
		}
	}else {
		ret = ws_geturi_contact(msg, aor, &ni);
		if (ni <= 0)
		{
			send_reply(msg, 404, "Requested Party Doesn't Exist Hint:P3C-1");
			_nNoRelay = 1;
		}
	}
	return 1;

}

int ws_geturi_contact(struct sip_msg *msg, str aor, int *append)
{
	db_res_t *res=NULL;
	db_row_t row;
	char query[1024]="";
	int row_count=0;
	char contact[512]="";
	char uri[512]="";
	int ret=0;

	if(!msg || !own_ip || (own_ip && strlen(own_ip)<=0)){
		LOG(L_WARN, "[ws_geturi_contact] own_ip is NULL or Empty\n");
		return 0;
	}
	snprintf(contact,sizeof(contact), "%.*s", aor.len, aor.s);
	bzero(query, sizeof(query));
	snprintf(query,sizeof(query)-1, "SELECT DISTINCT registrar FROM location WHERE username='%s' AND registrar <> '%s'", contact, own_ip);

	ws_dbf.raw_query(h, query, &res);
	if(res==NULL){
		return 0;
	}

	row_count = RES_ROW_N(res);
	if(row_count <= 0){
		ws_dbf.free_result(h, res);
		return 0;
	}

	while (row_count > 0)
	{
		row = RES_ROWS(res)[row_count-1];
		row_count--;
		if (row.values[0].nul == TRUE){
			continue;
		}
		snprintf(uri,sizeof(uri)-1, "sip:%s@%s", contact, row.values[0].val.string_val);
		if (*append == 0)
		{
			ret = set_uri2(msg, uri);
			if (ret < 0){
				LOG(L_WARN, "[ws_geturi_contact] Failed to Create First Branch\n");
				ws_dbf.free_result(h,res);
				return 0;
			}else{
				(*append)++;
			}
		}else{
			ret = append_branch(msg, uri, strlen(uri), 0, 0, 0);
			if (ret < 0){
				LOG(L_WARN, "[ws_geturi_contact] Failed to Append Branch\n");
				ws_dbf.free_result(h,res);
				return 0;
			}
		}
	}
	ws_dbf.free_result(h,res);
	return 1;
}

int process_conf_call2 (struct sip_msg *msg,  struct ws_uri *wsuri, int flag) {
	/* flag = 0 means needs to authenticate 
	   flag = 1 means authenticated 	*/
    char query[512]="" , qid[16]="" , request_uri[512]="";
	int nId = 0, nDcid = 0;
	char sitename[128]="" , *ptr = NULL;
	char cLocationIP[32]="", cRequestUri[256]="";
	bzero (query, sizeof(query));	
	if (flag==0) {
	        
		char * formatstring = "SELECT concat(concat(concat(concat(concat(concat('cnf-',  c.paswd), '-'), c.confid),'-'),REPLACE(sitename,'.','-')),'-%s') , 1, s.dcid FROM conference  c, siteinfo_new s WHERE c.siteid = s.siteid AND c.extension = '%s' AND sitename='%s' UNION SELECT id , 2, s.dcid FROM uc_apps u , siteinfo_new s WHERE u.siteid = s.siteid AND extension = '%s' AND s.sitename='%s'" ;

		db_res_t *res=NULL;
		db_row_t row;

		if ( !wsuri->command || !wsuri->context ) {
			return -1;
		}
		LOG(L_WARN, "[process_conf_call2] %s %s %s \n", wsuri->token , wsuri->command , wsuri->context);
		memset(sitename , 0 , sizeof(sitename));
		snprintf(sitename , sizeof(sitename)-1 , "%s" , wsuri->context);
		while(ptr = strchr(sitename , '-')){
				*ptr = '.';
		}
		memset(query,0,sizeof(query));
		snprintf(query,sizeof(query)-1, formatstring,wsuri->token, wsuri->command, sitename , wsuri->command, sitename);
		LOG(L_WARN, "[process_conf_call2] \n%s\n", query);
       	ws_dbf.raw_query (h, query, &res);
	
		if(res==NULL){
			LOG(L_ERR,"[process_conf_call2]'res' is NULL");
			return -1;
		}

       	if (RES_ROW_N(res) > 0) {
			row = RES_ROWS(res)[0];
			memset(query,0,sizeof(query)-1);
			memset(qid , 0 , sizeof(qid));
			strncpy (query, row.values[0].val.string_val, sizeof(query)-1);	
			strncpy (qid , row.values[1].val.string_val , sizeof(qid)-1);
			nId = atoi(qid);
			nDcid =  row.values[2].val.int_val;
			if (res)
        	        ws_dbf.free_result (h,res); 
			if(nId == 1){
				if(enable_dc_transfer && nRouteAcdAndConfCallsToHomeDc && nDcid && nDcid != datacenter_id) {
					memset(cRequestUri,0,sizeof(cRequestUri));
					snprintf(cRequestUri, sizeof(cRequestUri)-1,"WSRequestURI: %s\r\n",query);
					add_header(msg,cRequestUri,strlen(cRequestUri));

					GetRemoteDataCenterIP(nOtherDataCenterId,cLocationIP,sizeof(cLocationIP));
					LOG(L_WARN, "[process_conf_call2] It is an Across DC Conf call.So routing call to Home DC WSICP \n");
					/* Setting this flag to avoid extra billing . */
					msg->extra_info.nOtherDataCenterFlag = 1;
					if(strlen(cLocationIP) > 0) {
						return route_to_server(msg, query, cLocationIP,MS);
					}
				}
   				return route_to_media_server(msg, query,"");
			} else if (nId == 2){/* Routing call to wsucapp --Saidulu*/
				memset(request_uri , 0 , sizeof(request_uri));
				snprintf(request_uri , sizeof(request_uri)-1 , "wsucapp-%s-%s-%s-%s" , wsuri->command , query , wsuri->context , wsuri->token);
   				return route_to_media_server(msg, request_uri,"");
			}
	   
		}
		else {
			if (res)	
			ws_dbf.free_result (h,res);
			return -1;
		}	
	}
	else {
		memset(query,0,sizeof(query));
		snprintf(query,sizeof(query)-1,"%s",wsuri->group);
        LOG(L_WARN, "process_conf_call2]:uri = %s\n",query) ;
        return route_to_server(msg, query, conference_server,OTHER);
	}
	return 1; 
}

/* check extension type and process it -- Ajay/kkmurthy 2011-11-11 */
int ws_ext_call_process (struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag,int extn_callid_pass,int associate_acc_chk,int ob_ext_txfer, int rpid_flag)
{
	int nRes = -1;
	int flag = -1;
	char cdata[128] = "";
	char fullquery[1024] = "";
	int nPvalue = 0;
	int nRouteType = 0, nUserStirEnable = 0;
	char buffer[128] = "", cSpamBuffer[128] = "";

	db_row_t row ;
	db_res_t *result = NULL;
	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group || !wsuri->token ) {
		LOG (L_WARN, "[ws_ext_call_process] Invalid wsuri format??:\n");
		return -1;
	}

	memset(fullquery,0,sizeof(fullquery));
	snprintf(fullquery,sizeof(fullquery)-1,GROUP_EXTN_QUERY,wsuri->command,wsuri->context,wsuri->command,wsuri->context);
	ws_dbf.raw_query (h, fullquery, &result);
	if(result==NULL){
		LOG(L_ERR,"[ws_ext_call_process]'result' is NULL with Query : '%s' \n", fullquery);
		return -1;
	}

	/* siteinfo_new stir_route:	1	(reject the call)
	 * 				2	(play promt and reject the call )
	 * 				3	(allow --> if admin set this option , then fallow user settings.
	 *
	 * pbxusers stir_enable:	0	(don't follow user spam settings, fallow account settings only)
	 * 				1	(overide account settings with user settings depending on stir_route column in pbxusers table)			
	 */

	if (RES_ROW_N(result) > 0) {
		row = RES_ROWS(result)[0];
		strncpy(cdata,row.values[0].val.string_val,sizeof(cdata)-1);
		flag = atoi(cdata);
		if (!VAL_NULL(&row.values[1] ) ){
			nPvalue = get_db_int (row.values[1]);
		}
		if (!VAL_NULL(&row.values[2] ) ){ /* Account level settings */
			nRouteType = get_db_int (row.values[2]);
		}
		if (!VAL_NULL(&row.values[3] ) ){ /*nUserStirEnable = 1, then only depend on user settings */
			nUserStirEnable = get_db_int (row.values[3]);
		}
		/* check user settings */
		if (nUserStirEnable == 1 && !VAL_NULL(&row.values[4]) ){ 
			nRouteType = get_db_int (row.values[4]);
		}
	}
	if(result){
		ws_dbf.free_result(h,result);
	}

	if(nPvalue == BLOCK_IB || nPvalue == BLOCK_BOTH || (wsuri->group && !strcmp(wsuri->group,"transfer") && nPvalue == BLOCK_OB))
	{
		LOG(L_ERR,"[ws_ext_call_process] account wise call restriction nPvalue:%d, flag:%d\n", nPvalue,flag);
		return -1;
	}

	/* Added wsuri->group with '0' condition to check spam settings in IVR + Know extesnion cases */
	/* Checking spam settings in blind transfer or forwards to extension cases */
	LOG(L_ERR, "[ws_ext_call_process] DB VALUSE flag : %d, nRouteType : %d, nUserStirEnable : %d, wsuri->group : %s \n", flag, nRouteType, nUserStirEnable, wsuri->group ? wsuri->group : " ");
	if(wsuri->group && strcmp(wsuri->group,"0") && IsSpamCall(msg) && nRouteType != STIR_ROUTE_WITH_SPAM && (prepare_contact_spam_route_fail(msg, nRouteType) == 1)) {
		memset(buffer, 0, sizeof(buffer));
		snprintf(buffer, sizeof(buffer)-1, "ext-%s-%s-%s-%s", wsuri->command, wsuri->group, wsuri->context, wsuri->token);
		snprintf(cSpamBuffer, sizeof(cSpamBuffer)-1, "SPAM#%d", nUserStirEnable);
		racc_missed_call_entry(msg, buffer, cSpamBuffer);
		send_reply(msg, 302, "Moved temporarily");
  		_nNoRelay = 1;
		return 1;
	}

	if( flag == WS_USER_EXTEN ){
		if(ob_ext_txfer == OB_Transfer){
			nRes = process_ob_ext_call2(msg, priority, wsuri);
		}else{
			nRes = process_ext_call2 (msg, priority, wsuri, nFlag, extn_callid_pass,rpid_flag);
		}
	}else if(flag == WS_GROUP_EXTEN){
		nRes = ws_grp_ext_call_process( msg, priority, wsuri, 0, 1, rpid_flag,0);
	}else{
		if(associate_acc_chk == ASSOCIATIVE_ACC_CHECK){
			nRes = process_inv_call (msg, priority, wsuri, ob_ext_txfer);
		}else{
			LOG(L_ERR,"[ws_ext_call_process] unexpected place or invalid extension, flag:%d\n", flag);
			nRes = 1;
		}
	}
	return nRes;
}

/* Handling All type of group ext calls --Kimo/Ajay Jan 2012 */
int ws_grp_ext_call_process( struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag, int extcall, int rpid_flag,int nStreamletivr)
{
	char * Check_group_status = ""
			" SELECT code, acdenabled, extendedgroup, enableoperator, s.siteid, grouptype, s.dcid,s.stir_route "
			" FROM special sp, siteinfo_new s "
			" WHERE sp.siteid = s.siteid "
			" AND REPLACE(REPLACE(s.sitename, '.', '-'), '_', '-') = '%s' "
			" AND sp.extensions = '%s' ";

	int		nRes = 1;
	int		nSiteid = -1;
	int		grouptype = -1;
	int		acd_enabled = -1;
	int		ngroupcode = -1;
	int		extendedgroup = -1 ;
	int		nenableoperator = -1;
	int		nDcid = 0, nRouteType = 0;
	char	wsgroupcode[64] = "";
	char	fullquery[1024] = "", buffer[128] = "", cSpamBuffer[128] = "";
	db_row_t row ;
	db_res_t *result = NULL;

	if (!msg && !wsuri){
		LOG(L_ERR,"[ws_grp_ext_call_process] NULL sip msg  \n");
		return nRes;
	}
	memset(fullquery,0,sizeof(fullquery));
	if(nStreamletivr == 1){/*Fetching result based on group code*/
		snprintf(fullquery,sizeof(fullquery)-1,SELECT_IVR_DETAILS,wsuri->context, wsuri->command, wsuri->command );
	}else{
		snprintf(fullquery,sizeof(fullquery)-1,Check_group_status,wsuri->context, wsuri->command );
	}
	ws_dbf.raw_query (h, fullquery, &result);
	if(result==NULL){
		LOG(L_ERR,"[ws_grp_ext_call_process]'result' is NULL");
		return nRes;
	}
	if(RES_ROW_N(result) > 0){
		row = RES_ROWS(result)[0];
		ngroupcode = row.values[0].val.int_val;
		acd_enabled = row.values[1].val.int_val;
		extendedgroup = row.values[2].val.int_val;
		nenableoperator = row.values[3].val.int_val; 
		nSiteid =  row.values[4].val.int_val;
		grouptype =  row.values[5].val.int_val;
		nDcid =  row.values[6].val.int_val;
		nRouteType = row.values[7].val.int_val;
		ws_dbf.free_result (h,result);

		/* Added for AA + know group ext cases. skip 'AA' as we should not fallow sapm settings from 'AA' */
		if(!extendedgroup && IsSpamCall(msg) && nRouteType != STIR_ROUTE_WITH_SPAM && (prepare_contact_spam_route_fail(msg, nRouteType) == 1)) {
			if(wsuri && wsuri->command != NULL && wsuri->group != NULL && wsuri->context != NULL &&  wsuri->token != NULL) {
				memset(buffer, 0, sizeof(buffer));
				snprintf(buffer, sizeof(buffer)-1, "ext-%s-%s-%s-%s", wsuri->command, wsuri->group, wsuri->context, wsuri->token);
				snprintf(cSpamBuffer, sizeof(cSpamBuffer)-1, "SPAM#%d", nRouteType);
				racc_missed_call_entry(msg, buffer, cSpamBuffer);
			}
			send_reply(msg, 302, "Moved temporarily");
			_nNoRelay = 1;
			return 1;
		}

		if(grouptype ==3 ){
			wsuri->type =  "bla";
			snprintf(wsgroupcode , sizeof(wsgroupcode)-1 , "%d" , ngroupcode );
			wsuri->command = wsgroupcode;
			nRes = process_bla_call (msg, priority, wsuri, 0);
		}else if(!grouptype && !acd_enabled && !extendedgroup ) {
			nRes = process_group_ext_call( msg , priority , wsuri , nFlag , extcall, rpid_flag);
		}else{
			nRes = get_group_plan(msg,priority,ngroupcode,nSiteid,wsuri->context,"",wsuri->token,IVR_Command, nDcid, wsuri->command);
		}
	}else{
		ws_dbf.free_result (h,result);
	}
	return nRes;
}

/*For Group EXT CALLS - DATE-- NOV 8, 2011*/
int process_group_ext_call(struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag, int extcall, int rpid_flag) 
{
	char * formatstring =" "
		" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs,  sp.title,sp.code, 0 "
		" FROM  siteinfo_new s, special sp "
		" WHERE (sp.extensions = '%s' OR sp.code = '%s') "
		" AND s.siteid = sp.siteid "
		" AND sp.disasterflag = 1 "
		" AND s.sitename = REPLACE('%s','-','.')  and acctype!=2 and acctype!=4 "
		" AND sp.disasterphone IS NOT NULL "
		" UNION "
		" SELECT ph.phone,ph.phonetype, ph.ringsecs,sp.title ,  p.agentid, p.wscallfwdcname "
		" FROM pbxusers p, phones ph, special sp, agentspecial a,  weeklyplans w, siteinfo_new s "
		" WHERE (sp.extensions = %s OR sp.code = '%s')"
		" AND a.siteid = sp.siteid "
		" AND p.siteid = s.siteid  "
		" AND a.siteid = s.siteid "
		" AND a.agentid = p.agentid "
		" AND a.code = sp.code "
		" AND p.id = ph.pbxid "
		" AND ph.dialplanid = w.dialplanid "
		" AND sp.disasterflag = 0 "
		" AND sp.disablecallroute = 0 "
		" AND ph.groupactive = 1 "
		" AND ph.seq = '%d' "
		" AND sp.siteid = s.siteid "
		" AND s.sitename = REPLACE('%s','-','.') "
		" AND a.siteid = p.siteid "
		" AND w.pbxid "
		" NOT IN (SELECT dm.pbxid FROM dialplanmasks dm "
		" WHERE CURRENT_TIMESTAMP between dm.localfrom "
		" AND dm.localto) "
		" AND w.weekid = ( SELECT  dayofweek(convert_tz(now(), '-07:00', tdiff)) "
		" FROM pbxusers pu WHERE pu.id =  ph.pbxid ) "
		" AND ((localfromtime > localtotime "
		" AND ( time(now()) BETWEEN  localfromtime "
		" AND time('23:59:59')   OR time(now()) BETWEEN '00:00:00' "
		" AND  localtotime))   OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) and acctype!=2 and acctype!=4 "
//		" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)"
		" UNION "
		" SELECT ph.phone,ph.phonetype,ph.ringsecs, sp.title , p.agentid, p.wscallfwdcname "
		" FROM pbxusers p,phones ph, agentspecial a,dialplanmasks dm,siteinfo_new s, special sp "
		" WHERE s.siteid = a.siteid "
		" AND a.agentid = p.agentid "
		" AND s.sitename = REPLACE('%s','-','.') "
		" AND sp.disasterflag = 0 "
		" AND sp.disablecallroute = 0 "
		" AND a.siteid = p.siteid "
		" AND ph.groupactive = 1 "
		" AND p.id = ph.pbxid "
		" AND ph.dialplanid = dm.dialplanid " 
		" AND ph.pbxid = dm.pbxid "
		" AND sp.siteid = s.siteid "
		" AND ph.seq = %d "
		" AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
		" OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
		" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
//		" AND now() BETWEEN date(dm.localfrom) AND date(dm.localto) "
//		" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)"
		" AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto "
		" AND a.code=sp.code AND (sp.extensions = '%s' OR sp.code = '%s') and acctype!=2 and acctype!=4 LIMIT %d ";

       char  * disasterquery = ""
		" SELECT STRAIGHT_JOIN  sp.disasterphone, 6, 90, sp.title, sp.code, 0 "
		" FROM siteinfo_new s, special sp"
//		" WHERE  REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
		" WHERE  s.sitename = replace('%s','-','.') and acctype!=2 and acctype!=4 "
		" AND sp.siteid = s.siteid "
		" AND (sp.extensions = '%s' OR sp.code = '%s') "
		" AND sp.disasterphone IS NOT NULL";

       char * spamformatstring = "SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs,  sp.title,sp.code, 0 "
        " FROM  siteinfo_new s, special sp "
        " WHERE (sp.extensions = '%s' OR sp.code = '%s') "
        " AND s.siteid = sp.siteid  AND sp.disasterflag = 1 "
        " AND s.sitename = REPLACE('%s','-','.')  "
        " AND acctype!=2 and acctype!=4  AND sp.disasterphone IS NOT NULL  "
        " UNION "
        "SELECT ph.phone,ph.phonetype, ph.ringsecs,sp.title ,  p.agentid, p.wscallfwdcname "
        " FROM pbxusers p, phones ph, special sp, agentspecial a,  weeklyplans w, siteinfo_new s "
        " WHERE (sp.extensions = '%s' OR sp.code = '%s') AND a.siteid = sp.siteid  AND p.siteid = s.siteid "
        " AND a.siteid = s.siteid  AND a.agentid = p.agentid  AND a.code = sp.code  AND p.id = ph.pbxid "
        " AND ph.dialplanid = w.dialplanid  AND sp.disasterflag = 0  AND sp.disablecallroute = 0 "
        " AND ph.groupactive = 1  AND ph.seq = '%d'  AND sp.siteid = s.siteid  AND s.sitename = REPLACE('%s','-','.') "
        " AND a.siteid = p.siteid  AND w.pbxid  NOT IN (SELECT dm.pbxid FROM dialplanmasks dm "
        " WHERE CURRENT_TIMESTAMP between dm.localfrom  AND dm.localto) "
        " AND w.weekid = ( SELECT  dayofweek(convert_tz(now(), '-07:00', tdiff))  FROM pbxusers pu WHERE pu.id =  ph.pbxid ) "
        " AND ((localfromtime >localtotime  AND ( time(now()) BETWEEN  localfromtime "
        " AND time('23:59:59')   OR time(now()) BETWEEN '00:00:00'  AND  localtotime)) "
        " OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) and acctype!=2 and acctype!=4 "
        " AND ((sp.permit_user_spam = 0 AND s.stir_route = 3) OR (p.stir_enable = 1 AND p.stir_route = 3) "
        " OR (p.stir_enable = 0 AND s.stir_route = 3))"
        " UNION "
        "SELECT ph.phone,ph.phonetype,ph.ringsecs, sp.title , p.agentid, p.wscallfwdcname "
        " FROM pbxusers p,phones ph, agentspecial a,dialplanmasks dm,siteinfo_new s, special sp "
        " WHERE s.siteid = a.siteid  AND a.agentid = p.agentid  AND s.sitename = REPLACE('%s','-','.') "
        " AND sp.disasterflag = 0  AND sp.disablecallroute = 0  AND a.siteid = p.siteid  AND ph.groupactive = 1 "
        " AND p.id = ph.pbxid  AND ph.dialplanid = dm.dialplanid  AND ph.pbxid = dm.pbxid  AND sp.siteid = s.siteid "
        " AND ph.seq = %d  AND ( (localfromtime >localtotime AND ( time(now()) BETWEEN localfromtime "
        " AND time('23:59:59')  OR time(now()) BETWEEN '00:00:00' AND localtotime))  OR ( time(now()) BETWEEN ph.localfromtime "
        " AND ph.localtotime ) )  AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto "
        " AND a.code=sp.code AND (sp.extensions = '%s' OR sp.code = '%s') and acctype!=2 and acctype!=4 LIMIT %d ";

	int nRes = -1,nRows = 0;
	char buffer[512] = "";
	int nSeq = -1, is_fork_group = 0,nRet=0;
	char groupname[128] = "";
	char fullquery[10240] = "";

	db_res_t *result = NULL;

	if (!msg && !wsuri){
		LOG(L_ERR,"[process_group_ext_call] NULL sip msg  \n");
		return -1;
	}

	nSeq = atoi(priority);
	
	for (; nSeq <= 4; nSeq++ ) {
		if(WSState->nDisasterMode == DISABLED ) {
		memset(fullquery,0,sizeof(fullquery));
		if(IsSpamCall(msg)){
				snprintf(fullquery, sizeof(fullquery)-1, spamformatstring, wsuri->command,wsuri->command, wsuri->context, wsuri->command,wsuri->command , nSeq, wsuri->context, wsuri->context, nSeq, wsuri->command,wsuri->command,fgrphuntgrouplimit);
		}else{
				snprintf(fullquery, sizeof(fullquery)-1, formatstring, wsuri->command,wsuri->command, wsuri->context, wsuri->command,wsuri->command , nSeq, wsuri->context, wsuri->context, nSeq, wsuri->command,wsuri->command,fgrphuntgrouplimit);
		}
		}else{
			memset(fullquery,0,sizeof(fullquery));
			snprintf(fullquery, sizeof(fullquery)-1, disasterquery , wsuri->context, wsuri->command,wsuri->command);
		}

		//LOG(L_WARN,"[process_group_ext_call] The fullquery is = '%s' \n", fullquery );
		ws_dbf.raw_query (h, fullquery, &result);
		if(result==NULL){
			LOG(L_WARN,"[process_group_ext_call]'result' is NULL");
			return -1;
		}
		if ((nRows = RES_ROW_N(result)) > 0) {
			if(RES_ROWS (result)[0].values[3].type == DB_STRING ){
			LOG(L_WARN,"[process_group_ext_call] Groupname for Remote Party ID %s\n",RES_ROWS(result)[0].values[3].val.string_val);
			memset(groupname, 0, sizeof(groupname));
			strncpy( groupname , RES_ROWS(result)[0].values[3].val.string_val, sizeof(groupname)-1);
			}
	
			LOG(L_WARN,"[process_group_ext_call]  agent id = '%s' ", wsuri->group);
			if(is_nested_group && !(!strncmp(groupname,"Default",7) && strlen(groupname) == 7) && nRows>MAX_BRANCHES ){
				if((nRes = process_nested_forking(msg,result,wsuri->token,wsuri->context,nRows)) == 1){
					is_fork_group = 1;
				}
			}
			if(!is_fork_group){
				nRes = create_branches(msg, result, 1, wsuri);
			}
			LOG(L_WARN,"[process_group_ext_call] 1.nRes vale:%d",nRes);
			if (result)
				ws_dbf.free_result (h,result);
			if ( nRes < 0) {
				continue;
			}
			if (_nOrigSeq == 1) {
				if (strlen(groupname)) {
					ws_set_group_name(msg, groupname);
				}
                if(!rpid_flag)
				    ws_set_rpid(msg, registrar_domain,NULL,NULL,0,NULL,0,0,1, NULL);
			}
			if ( t_on_failure ) {
				if(nRes == 2 ){
					nSeq = 4;
				}
				t_on_failure (msg, (char *)(nSeq + 1), NULL);
				LOG(L_ERR, "On failure : %d\n", nSeq + 1 );
			}
			break;
		}
		if (result){
			ws_dbf.free_result (h, result);
		} 
	}
	if ( nRes < 0 ) {
		LOG(L_WARN,"[process_group_ext_call] RES_ROW_N(res) < 0 \n");
		if (nFlag) {
			LOG(L_ERR, "[process_group_ext_call] Route him to IVR\n");
			nRet = worksmart_route_to_ivr2(msg,1);
			if(nRet){
				return 1;
			}
		}

		char newuri[512];
		int nRespCode = get_resp_code(msg);

		memset(fullquery,0,sizeof(fullquery));
		snprintf(fullquery, sizeof (fullquery)-1,"SELECT s.code,s.title "
			"FROM special s, siteinfo_new si "
			"WHERE si.sitename = REPLACE('%s','-','.') "
			"AND si.siteid = s.siteid "
			"AND (s.extensions = '%s' OR s.code = '%s') and acctype!=2 and acctype!=4",wsuri->context,wsuri->command,wsuri->command);
		ws_dbf.raw_query(h, fullquery, &result);
		if(result==NULL || RES_ROW_N(result) <= 0 ) {
			LOG(L_ERR,"[process_group_ext_call] 'result' is NULL");
			if(result) {
				ws_dbf.free_result (h, result);
			}
			return -1;
		}
		int nCode = 0;
		if (RES_ROW_N(result) > 0) {
			nCode = get_db_int(RES_ROWS(result)[0].values[0]);
		}
		if (result)
			ws_dbf.free_result (h, result);
			LOG(L_WARN, "[process_group_ext_call] response code '%d' , '%s'\n", nRespCode,buffer );
			memset(newuri,0,sizeof(newuri));
			snprintf (newuri, sizeof (newuri)-1, "vml-%d-%d-%s-%s", nCode , nRespCode, wsuri->context, wsuri->token);
			LOG(L_WARN, "[process_group_ext_call]*=*=*=*=*=*=* uri is '%s'\n", newuri);
			return route_to_media_server(msg, newuri,"");
	}
	return 1;
}

/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
int process_ext_call2 (struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag, int extcall,int rpid_flag) {
	char * nonbla_formatstring = ""
		"SELECT phone, phonetype, ringsecs,p.agentid, p.agentid, p.wscallfwdcname"
		"  FROM pbxusers p, dialplantypes d, weeklyplans w, phones ph, siteinfo_new s "
		" WHERE ph.pbxid = p.id "
		"   AND p.id = d.pbxid  "
		"   AND d.id = w.dialplanid "
		"   AND s.siteid = p.siteid  "
		"   AND w.pbxid = p.id"
		"   AND weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
		"       FROM pbxusers pu "
		"      WHERE pu.id = p.id ) "
		"   AND w.dialplanid = ph.dialplanid "
		"   AND extensions = '%s' "
		"   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
		"   AND ph.seq = %d "
		"   AND w.pbxid NOT IN ( "
		"        SELECT dm.pbxid "
		"          FROM dialplanmasks dm "
		"         WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto "
		"   ) "
		"   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
		"    OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
		"	 OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
		"   AND p.disasterflag=0 "
		"UNION "
		"SELECT phone, phonetype, ringsecs,p.agentid, p.agentid, p.wscallfwdcname "
		"  FROM pbxusers p, dialplantypes d, dialplanmasks dm, phones ph, siteinfo_new s  "
		" WHERE ph.pbxid = p.id "
		"   AND p.id = d.pbxid "
		"   AND d.id = dm.dialplanid "
		"   AND dm.dialplanid = ph.dialplanid "
		"   AND extensions = '%s' "
		"   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
		"   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
		"	OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
		"	 OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
		"   AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto "
		"   AND s.siteid = p.siteid  "
		"   AND ph.seq = %d "
		"   AND p.disasterflag=0 "
		" UNION "
		"SELECT p.disasterphone as phone, 6 , 90,p.agentid , p.agentid, p.wscallfwdcname"
		"   FROM pbxusers p, siteinfo_new s "
		"  WHERE REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' " 
		"   AND p.siteid = s.siteid " 
		"	AND p.extensions='%s' "
		"	AND p.disasterflag=1 "
		"	AND disasterphone IS NOT NULL ";

	char * formatstring = ""
		"SELECT phone, phonetype, ringsecs,p.agentid, p.agentid, p.wscallfwdcname "
		"  FROM pbxusers p, dialplantypes d, weeklyplans w, phones ph, siteinfo_new s "
		" WHERE ph.pbxid = p.id "
		"   AND p.id = d.pbxid  "
		"   AND d.id = w.dialplanid "
		"   AND s.siteid = p.siteid  "
		"   AND w.pbxid = p.id"
		"   AND weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
		"       FROM pbxusers pu "
		"      WHERE pu.id = p.id ) "
		"   AND w.dialplanid = ph.dialplanid "
		"   AND extensions = '%s' "
		"   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
		"   AND ph.seq = %d "
		"   AND w.pbxid NOT IN ( "
		"        SELECT dm.pbxid "
		"          FROM dialplanmasks dm "
		"         WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto "
		"   ) "
		"   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
		"    OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
		"	 OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
		"   AND p.disasterflag=0 "
//		"   AND ( (ph.phone NOT IN (select username from pbxblausers WHERE 	agentid=p.agentid)) "
//		"   	OR (ph.phone IN (select username from pbxblausers WHERE agentid=p.agentid) "
//		"   	AND ph.phone NOT IN (select sharedid from appearance_info)) "
//		"   	OR (ph.phone IN (select username from pbxblausers WHERE agentid=p.agentid) "
//		"   	AND ph.phone IN (select sharedid from appearance_info WHERE appearancestate = 'FREE'))) "
		"UNION "
		"SELECT phone, phonetype, ringsecs,p.agentid, p.agentid, p.wscallfwdcname "
		"  FROM pbxusers p, dialplantypes d, dialplanmasks dm, phones ph, siteinfo_new s  "
		" WHERE ph.pbxid = p.id "
		"   AND p.id = d.pbxid "
		"   AND d.id = dm.dialplanid "
		"   AND dm.dialplanid = ph.dialplanid "
		"   AND extensions = '%s' "
		"   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
		"   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
		"	OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
		"	 OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
		"   AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto "
		"   AND s.siteid = p.siteid  "
		"   AND ph.seq = %d "
		"   AND p.disasterflag=0 "
//		"   AND ( (ph.phone NOT IN (select username from pbxblausers WHERE 	agentid=p.agentid)) "
//		"   	OR (ph.phone IN (select username from pbxblausers WHERE agentid=p.agentid) "
//		"   	AND ph.phone NOT IN (select sharedid from appearance_info)) "
//		"   	OR (ph.phone IN (select username from pbxblausers WHERE agentid=p.agentid) "
//		"   	AND ph.phone IN (select sharedid from appearance_info WHERE appearancestate = 'FREE'))) "
		" UNION "
		"SELECT p.disasterphone as phone, 6 , 90,p.agentid, p.agentid, p.wscallfwdcname "
		"   FROM pbxusers p, siteinfo_new s "
		"  WHERE REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' " 
		"   AND p.siteid = s.siteid " 
		"	AND p.extensions='%s' "
		"	AND p.disasterflag=1 "
		"	AND disasterphone IS NOT NULL ";

	char  * disasterquery = ""
			"SELECT p.disasterphone as  phone, 6 as phonetype, 90 as ringsecs,p.agentid, p.agentid,p.wscallfwdcname "
			"  FROM pbxusers p, siteinfo_new s "
			"  WHERE REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			"  	AND p.siteid = s.siteid " 
			"	AND p.extensions='%s' "
			"	AND disasterphone IS NOT NULL";
					
	db_res_t *res=NULL;
	db_row_t row;
	int nRes = -1,nRet=0;
	char query[5120];
	int nSeq = atoi(priority);
	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group ) {
	    return -1;
	}
    
	LOG(L_ERR, "[process_ext_call2] priority 1  = '%d' \n", nSeq);
	clear_branches();

	for (; nSeq <= 4; nSeq++ ) {
		LOG(L_WARN, "[process_ext_call2] enetere into for loop\n");	
		if(!strncmp(wsuri->type, "icom" , 4) && nSeq > 1 ){
				/* Here we are return cause this is an intercom call and no one is accepted it */
				LOG(L_ERR, "[process_ext_call2] Intercom  IN FOR LOOP \n");
				return -1;
		}

		if( WSState->nDisasterMode == ENABLED ){
			LOG(L_WARN, "[process_ext_call2] disasterquery \n");
			memset(query,0,sizeof(query));
			snprintf(query, sizeof(query)-1, disasterquery,wsuri->context, wsuri->command);
		}
		else{
			if(_nSupSeq < 6){
				LOG(L_ERR, "[process_ext_call2] formatstring \n");
				memset(query,0,sizeof(query));
				snprintf(query, sizeof(query)-1, formatstring,servertz, wsuri->command, wsuri->context, nSeq,
						wsuri->command, wsuri->context, nSeq,
						wsuri->context,wsuri->command);
			}else{
				LOG(L_WARN, "[process_ext_call2]  nonbla_formatstring \n");
				memset(query,0,sizeof(query));
				snprintf(query, sizeof(query)-1, nonbla_formatstring,servertz, wsuri->command, wsuri->context, nSeq,
						wsuri->command, wsuri->context, nSeq,
						wsuri->context,wsuri->command);
			}
		}
    	//LOG(L_ERR, "[process_ext_call2] \n %s\n", query);
	    ws_dbf.raw_query (h, query, &res);
		if(res==NULL){
			LOG(L_ERR,"[process_ext_call2]'res' is NULL");
			return -1;
		}

		if (RES_ROW_N(res) > 0) {
	    		LOG(L_WARN,"[process_ext_call2] RES_ROW_N(res) > 0 \n");
			
			if(nSeq > 1){
				row = RES_ROWS (res)[0];
				if(get_db_int(row.values[1]) == 6)
				{
					nRes = -1;
					break;
				}
				
			}
			nRes = create_branches(msg, res, 1, wsuri);

			if (res)
			    ws_dbf.free_result (h,res);

			
			if ( nRes < 0) {
				continue;
			}
			if (_nOrigSeq == 1 && !rpid_flag) {
				/* Set the Remote-Party-ID only for the first priority -Ramu 04/04/06*/
				ws_set_rpid(msg, registrar_domain ,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
			}
			if (nRes != 3 &&  t_on_failure ) {
				if(nRes == 2){
					nSeq = 4;
				}else{
				/*Added by tulsi for fix the media server pool. But here i am expecting some bug..*/
					if(_nSupSeq > 5){
						LOG(L_WARN,"[process_ext_call2]_nSupSeq Value:%d\n",_nSupSeq);
						nSeq = 24 + nSeq;
					}
				}
				
				t_on_failure (msg, (char *)(nSeq + 1), NULL);
				LOG(L_ERR, "[process_ext_call2]On failure : %d\n", nSeq  );
			}
			break;
		}
		if (res)
	    	ws_dbf.free_result (h,res);
	}

	//LOG(L_ERR, "[process_ext_call2]mysql query res ='%d'\n",nRes);
	if ( nRes < 0 ) {
		LOG(L_WARN,"[process_ext_call2] RES_ROW_N(res) < 0 \n");
		if(!strncmp(wsuri->type, "icom",4)){
			/* No voicemail for intercom calls */
			LOG(L_ERR, "[process_ext_call2] Intercom failed hanging up the call \n");
			return -1;
		}
		if (nFlag) {
			LOG(L_ERR, "[process_ext_call2] Route him to IVR\n");
			nRet = worksmart_route_to_ivr2(msg,1);
			if(nRet){
				return nRet;
			}
		}
		int nRespCode = get_resp_code(msg);
		char newuri[512];
		memset(newuri,0,sizeof(newuri));
		if(msg->nRejectRelay > 0){

			if(msg->nRejectRelay > 0 && strlen(msg->cRejectReason) > 0){
				memset(newuri, 0, sizeof(newuri));
				snprintf(newuri, sizeof(newuri)-1, "X-Reject-Reason: %d;%s\r\n", msg->nRejectRelay, msg->cRejectReason);
				add_lump_rpl(msg, newuri, strlen(newuri), LUMP_RPL_HDR);
			}

			memset(newuri, 0, sizeof(newuri));
			snprintf (newuri, sizeof (newuri)-1, "tranfail-%d-0-0-0", msg->nRejectRelay);//uri to play robocall detected/auth failed message and disconnect
			LOG(L_ERR,"Sending 603 here...!\n");
			ws_t_reply(msg,(char *)603,"Declined");	
			return 1;
		}
		snprintf (newuri, sizeof (newuri)-1, "vml-%s-%d-%s-%s", wsuri->command, nRespCode, wsuri->context, wsuri->token);
		LOG(L_WARN, "[process_ext_call2]*=*=*=*=*=*=* uri is '%s'\n", newuri);

		return route_to_media_server(msg, newuri,"");
	}

	if(!strncmp(wsuri->type, "icom",4)){
		char newheader[256]="";
		strncpy(newheader,"Alert-Info: Ring Answer\r\n",sizeof(newheader)-1);
		str cmd={newheader,strlen(newheader)};
		if (append_hf) {
			if(append_hf(msg, (char *)&cmd,0)!=1)
				LOG(L_ERR,"[process_ext_call2]header 1 append_hf failure\n");
		}
		memset(newheader,0x00,sizeof(newheader));
		snprintf(newheader,sizeof(newheader)-1,"Call-Info:<sip:%s>\\;answer-after=0\r\n",proxy_domain);
		str cmd1={newheader,strlen(newheader)};
		if (append_hf) {
			if(append_hf(msg, (char *)&cmd1,0)!=1)
				LOG(L_ERR,"[process_ext_call2] header2 append_hf failure\n");
		}
	}

	return 1;
}

int process_ob_ext_call2 (struct sip_msg *msg, char *priority, struct ws_uri *wsuri) {
		char * formatstring = ""
	        "SELECT STRAIGHT_JOIN phone, phonetype, ringsecs ,p.agentid, p.agentid, 0"
	        "  FROM pbxusers p, dialplantypes d, weeklyplans w, phones ph, siteinfo_new s "
	        " WHERE ph.pbxid = p.id "
	        "   AND p.id = d.pbxid  "
	        "   AND d.id = w.dialplanid "
	        "   AND s.siteid = p.siteid  "
	        "   AND w.pbxid = p.id"
		    "   AND weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
		    "       FROM pbxusers pu "
	        "      WHERE pu.id = p.id ) "
	        "   AND w.dialplanid = ph.dialplanid "
	        "   AND extensions = '%s' "
	        "   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
	       	"   AND ph.seq = %d "
	      	"   AND w.pbxid NOT IN ( "
	        "        SELECT dm.pbxid "
	        "          FROM dialplanmasks dm "
	       	"         WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto "
	       	"   ) "
	        "   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
	        "    OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
	       	"    OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
	        "   AND p.disasterflag=0 "
	        "UNION "
	        "SELECT STRAIGHT_JOIN phone, phonetype, ringsecs, p.agentid, p.agentid, 0 "
	       	"  FROM pbxusers p, dialplantypes d, dialplanmasks dm, phones ph, siteinfo_new s  "
	        " WHERE ph.pbxid = p.id "
	        "   AND p.id = d.pbxid "
	        "   AND d.id = dm.dialplanid "
	        "   AND dm.dialplanid = ph.dialplanid "
	       	"   AND extensions = '%s' "
	        "   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
	        "   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
	        "   OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
	        "    OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
	      	"   AND CURRENT_TIMESTAMP between dm.localfrom AND dm.localto "
		    "   AND s.siteid = p.siteid  "
		    "   AND ph.seq = %d "
		    "   AND p.disasterflag=0 "
		    " UNION "
		 	"SELECT STRAIGHT_JOIN p.disasterphone as phone, 6, 90, p.agentid, p.agentid, 0 "
			"   FROM pbxusers p, siteinfo_new s "
			"  WHERE REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			"   AND p.siteid = s.siteid "
			"   AND p.extensions='%s' "
			"   AND p.disasterflag=1 "
			"   AND disasterphone IS NOT NULL";
		
			char  * disasterquery = ""
			"SELECT STRAIGHT_JOIN p.disasterphone as  phone, 6 as phonetype, 90 as ringsecs, p.agentid, p.agentid, 0 "
		    "  FROM pbxusers p, siteinfo_new s "
		    "  WHERE REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
		    "   AND p.siteid = s.siteid "
		    "   AND p.extensions='%s' "
			"   AND disasterphone IS NOT NULL";


		    db_res_t *res=NULL;
		    int nRes = -1;
		    char query[5120];
		    int nSeq = atoi(priority);
		    if ( !wsuri->context || !wsuri->type || !wsuri->command ) {
				LOG(L_ERR,"[process_ob_ext_call2] Invalid WSURI\n");
				return -1;
		    }

		    clear_branches();
		    for (; nSeq <= 4; nSeq++ ) {
				if( WSState->nDisasterMode == ENABLED ){
					memset(query,0,sizeof(query));
					snprintf(query, sizeof(query)-1, disasterquery,wsuri->context, wsuri->command);
				}
				else{
					memset(query,0,sizeof(query));
				    snprintf(query, sizeof(query)-1, formatstring,servertz, wsuri->command, wsuri->context, nSeq,wsuri->command, wsuri->context, nSeq,wsuri->context,wsuri->command);
				}
				/*LOG(L_ERR, "[process_ob_ext_call2]\n %s\n", query);*/
				ws_dbf.raw_query (h, query, &res);
				if(res==NULL){
					LOG(L_ERR,"[process_ob_ext_call2]'res' is NULL");
					return -1;
				}
				if (RES_ROW_N(res) > 0) {
				    nRes = create_branches(msg, res, 1, wsuri);
				    if (res)
						ws_dbf.free_result (h,res);
				    if ( nRes < 0) {
						continue;
				    }
				    if ( t_on_failure ) {
						if(nRes == 2){
						    nSeq = 4;
						}
						t_on_failure (msg, (char *)(nSeq + 1), NULL);
						LOG(L_ERR, "On failure : process_ob_ext_call2 %d\n", nSeq  );
				    }
				    break;
				}
				if (res)
					ws_dbf.free_result (h,res);
			}
		    if ( nRes < 0 ) {
				char newuri[512];
				int nRespCode = get_resp_code(msg);
				memset(newuri,0,sizeof(newuri));
				snprintf (newuri, sizeof (newuri)-1, "vml-%s-%d-%s-%s", wsuri->command, nRespCode, wsuri->context, wsuri->token);
				LOG(L_WARN, "[process_ob_ext_call2] new uri:%s\n",newuri);
				route_to_media_server(msg,newuri,"");
			}
		    return 1;
}

int get_extended_plan(struct sip_msg * msg, char * prio, struct ws_uri * wsuri)
{
	db_res_t *res=NULL;
	db_row_t row;
	int nCode= 0;
	int nACDEnabled = 0;
	int nExtendedGroup = 0;
	int nEnableOperator = 0;
	int nSiteid = 0;
	int nGroupType = 0;
	char cGroupTitle[256] = "";
	int nDcid = 0;
	char cExt[32] = "";
	char * format_string = ""
		 " SELECT code, acdenabled, extendedgroup, enableoperator, s.siteid, grouptype, sp.title, s.dcid, sp.extensions "
		 "  FROM special sp, extendedpbxivrcommand e, siteinfo_new s "
		 " WHERE sp.siteid = e.siteid "
		 "   AND e.siteid = s.siteid"
		 "   AND REPLACE(REPLACE(s.sitename, '.', '-'), '_', '-') = '%s'"
		 "   AND e.groupcode = %s"
		 "   AND sp.code = e.extensiongroupcode"
		 "   AND command = %s";
	char query[1024];
	bzero (query, sizeof (query));
	snprintf(query, sizeof(query)-1, format_string, wsuri->context,wsuri->group, wsuri->command);
	LOG(L_ERR, "[get_extended_plan]\n %s\n", query);
	ws_dbf.raw_query (h, query, &res);
	if(!res || RES_ROW_N(res) <= 0){
		LOG(L_WARN,"[get_extended_plan]'res' is NULL");
		if (res){
			LOG(L_WARN,"[get_extended_plan]No result is found\n");
			ws_dbf.free_result (h,res);
		}
		return -1;
	}

	row = RES_ROWS(res)[0];
	nCode = get_db_int(row.values[0]);
	nACDEnabled = get_db_int(row.values[1]);
	nExtendedGroup = get_db_int(row.values[2]);
	nEnableOperator = get_db_int(row.values[3]);
	nSiteid = get_db_int(row.values[4]);
	nGroupType = get_db_int(row.values[5]);
	if(RES_ROWS(res)[0].values[6].val.string_val){
		strncpy(cGroupTitle,RES_ROWS(res)[0].values[6].val.string_val, sizeof(cGroupTitle)-1);
	}
	nDcid = get_db_int(row.values[7]);
	memset(cExt, 0, sizeof(cExt));
	if(RES_ROWS(res)[0].values[8].val.int_val){
		snprintf(cExt,sizeof(cExt)-1,"%d", RES_ROWS(res)[0].values[8].val.int_val);
	}

	LOG(L_WARN,"[get_extended_plan]nGroupType:%d\n",nGroupType);
	if (res){
		ws_dbf.free_result (h,res);
	}

	if(nGroupType == 3){ //Identfying BLA group .... 
		return 3;
	}
	if(nACDEnabled==1 || nExtendedGroup==1){
		return get_group_plan (msg,  prio, nCode, nSiteid, wsuri->context,"",wsuri->token, IVR_Command, nDcid, cExt);
	}
	return -1;
}

int process_vmopr_call (struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag ) 
{
	int nRes = -1,nRows = 0;
	char fullquery[5120] = "";
	char groupname[128] = "";
	db_res_t *res=NULL;
	int blagroup = 0, is_fork_group = 0;
	int nSeq = atoi(priority);
	
	char  * disasterquery = ""
			" SELECT STRAIGHT_JOIN  sp.disasterphone as phone, 6 as phonetype, 90 as ringsecs, sp.title, sp.code, 0 "
			" FROM   siteinfo_new s, special sp"
			" WHERE  REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s'  "
			" AND sp.siteid = s.siteid  "
			" AND sp.code = '%s'  "
			" AND sp.disasterflag = 1  "
			" AND sp.disasterphone IS NOT NULL";

	char * formatstring =" "
			" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs, sp.title ,sp.code, 0"
			" FROM   siteinfo_new s, special sp "
			" WHERE sp.code = %s AND sp.disasterflag = 1 "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND s.siteid = sp.siteid "
			" AND sp.disasterphone IS NOT NULL "
			" UNION "
			" SELECT ph.phone,ph.phonetype, ph.ringsecs, sp.title,p.agentid, p.wscallfwdcname  FROM  pbxusers p, phones ph, special sp, agentspecial a,  weeklyplans w "
			" WHERE a.siteid = ( SELECT siteid from siteinfo_new where  REPLACE(REPLACE(sitename,'.','-'),'_','-') = '%s' limit 1 ) "
			" AND a.siteid = p.siteid  "
			" AND sp.siteid = a.siteid "
			" AND a.code=%s "
			" AND a.agentid = p.agentid "
			" AND p.id = ph.pbxid  "
			" AND ph.dialplanid = w.dialplanid "
			" AND ph.groupactive = 1 "
			" AND sp.disasterflag = 0 " 
			" AND sp.disablecallroute = 0 "
			" AND ph.seq = %d "
			" AND sp.code = a.code "
			" AND w.pbxid NOT IN (SELECT dm.pbxid FROM dialplanmasks dm WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto) "
			" AND w.weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
			" FROM pbxusers pu WHERE pu.id = ph.pbxid ) AND ( (localfromtime > localtotime "
			" AND ( time(now()) BETWEEN localfromtime AND time('23:59:59')  OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
			" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime )) "
//			" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)) "
			" UNION "
			" (SELECT ph.phone,ph.phonetype,ph.ringsecs, sp.title,p.agentid, p.wscallfwdcname  FROM    pbxusers p, phones ph, agentspecial a, dialplanmasks dm, special sp "
			" WHERE  a.siteid = ( SELECT siteid from siteinfo_new where  REPLACE(REPLACE(sitename,'.','-'),'_','-') = '%s' limit 1 ) "
			" AND p.siteid = a.siteid "
			" AND sp.siteid = a.siteid "
			" AND a.code = %s "
			" AND sp.code = a.code "
			" AND a.agentid = p.agentid "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND ph.groupactive = 1 "
			" AND p.id = ph.pbxid "
			" AND ph.dialplanid = dm.dialplanid "
			" AND ph.pbxid = dm.pbxid "
			" AND ph.seq = %d "
			" AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59')  "
			" OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
			" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) )"
			" AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto ) LIMIT %d";
//			" AND now() BETWEEN date(dm.localfrom) AND date(dm.localto) "
//			" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)) "; 
	
	char * bla_formatstring =" "
			" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs, sp.title,sp.code,0 "
			" FROM   siteinfo_new s, special sp "
			" WHERE sp.code = %s "
			" AND sp.disasterflag = 1 "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND sp.disasterphone IS NOT NULL "
			" UNION "
			" (SELECT ph.username,2, ph.ringsecs, sp.title ,p.agentid, p.wscallfwdcname"
			" FROM  pbxusers p,special sp,pbxblausers ph,agentspecial a,siteinfo_new s,appearance_info app "
			" WHERE a.siteid = ( SELECT siteid from siteinfo_new where  REPLACE(REPLACE(sitename,'.','-'),'_','-') = '%s' limit 1 ) "
			" AND a.siteid = p.siteid "
			" AND sp.siteid = a.siteid "
			" AND a.code=%s "
			" AND a.agentid = p.agentid "
			" AND p.agentid = ph.agentid "
			" AND ph.siteid = s.siteid "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND sp.code = %s"
			" AND sp.siteid = s.siteid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND a.siteid = p.siteid "
			" AND a.seq_num = %d  "
			" AND app.sharedid = ph.username "
			" AND app.appearancestate = 'FREE' "
			" ORDER BY a.seq_num "
			" limit 1) LIMIT %d" ;

	if ( !wsuri->type || (strcmp(wsuri->type, "vmo"))){
			LOG(L_ERR, "[process_vmopr_call]Invalid URI for process_ivr_vmd: Level1 error");
			return -1;
	}
			
	if ( !wsuri->command || !wsuri->group || !wsuri->context ) {
		LOG(L_ERR, "[process_vmopr_call]Invalid URI for process_ivr_command: Level2 error");
		return -1;
	}
	nRes = get_extended_plan(msg, priority, wsuri);

	if(nRes == 1){
		/* Use rtp proxy */
		return WS_USE_RTP_PROXY;
	}
	else if(nRes == 2) {
		/* dont use rtp proxy */
		return WS_DONT_USE_RTP_PROXY;
	}
	else if(nRes == 3){//BLA Group Call
		LOG(L_WARN,"[process_vmopr_call]Got  the BLA Group\n");
		blagroup = 1;
		nRes = -1;
	}
	
	clear_branches();

	if(blagroup){
		/* Added bla sequential routing for brguest fixes --kimo/vravi */
		LOG (L_ERR, "[blagroup::process_vmopr_call] MAX_BLA_LINES  : %d\n", max_bla_lines);
		if( _nSupSeq != 4){  /* _nSupSeq == 4 means Got response other than 486 for first sequence for BLA call --vravi */
			for (; nSeq <= max_bla_lines; nSeq++ ) {
				if(WSState->nDisasterMode == DISABLED ) {
					memset(fullquery,0,sizeof(fullquery));
					snprintf(fullquery, sizeof(fullquery)-1, bla_formatstring, wsuri->group,wsuri->context, wsuri->context,wsuri->group,wsuri->group,wsuri->context,nSeq,fgrphuntgrouplimit);
				}else{
					 memset(fullquery,0,sizeof(fullquery));
			 		snprintf(fullquery, sizeof(fullquery)-1, disasterquery , wsuri->context, wsuri->group);
				}
			
				LOG (L_ERR, "[blagroup::process_vmopr_call] fullquery : %s\n", fullquery);
				ws_dbf.raw_query (h, fullquery, &res);
				if(res==NULL){
					LOG(L_ERR,"[blagroup::process_vmopr_call] 'res' is NULL ... route to vml");
					nRes = -1; //Need to route to vml
					break;
				}
				if ((nRows = RES_ROW_N(res)) <= 0) {
					LOG(L_ERR,"[blagroup::process_vmopr_call] result set is not found .." );
					ws_dbf.free_result (h, res);
					continue;
				}
				if(RES_ROWS (res)[0].values[3].type == DB_STRING ){
					memset(groupname, 0, sizeof(groupname));
					strncpy( groupname ,   RES_ROWS(res)[0].values[3].val.string_val, sizeof(groupname)-1);
				}
				LOG(L_WARN,"[blagroup::process_vmopr_call] Creating the branches ...");
				if(is_nested_group && !(!strncmp(groupname,"Default",7) && strlen(groupname) == 7)  &&  nRows>MAX_BRANCHES ){
					if((nRes = process_nested_forking(msg,res,wsuri->token,wsuri->context,nRows)) == 1){
						is_fork_group = 1;
					}
				}
				if(!is_fork_group){
					if ((nRes = create_branches(msg, res, 1, wsuri)) < 0 ){
						LOG(L_WARN,"[blagroup::process_vmopr_call] nRes vale:%d,creating branches failed continuing ..",nRes);
						ws_dbf.free_result (h, res);
						continue;
					}
				}
				LOG(L_WARN,"[blagroup::process_vmopr_call] nRes vale:%d",nRes);
				
				/* Set the Remote-Party-ID only for first priority -Ramu 04/04/06*/
				if (strlen(groupname)) {
					LOG(L_WARN,"[blagroup::process_vmopr_call] Setting Group name as : %s\n", groupname);
					ws_set_group_name(msg, groupname);	
				}
				ws_set_rpid(msg, registrar_domain,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
				if ( t_on_failure ) {
					t_on_failure (msg, (char *)(nSeq + 1), NULL);
					LOG(L_WARN, "[blagroup::process_vmopr_call]On failure : %d\n",nSeq + 1);
				}
				if (res){
					ws_dbf.free_result (h, res);
				}
				break;
			}
			LOG(L_WARN, "[blagroup::process_vmopr_call] On failure route : %d , nRes : %d\n", nSeq + 1,nRes);
		}else {
			LOG (L_ERR, "[blagroup::process_vmopr_call] BLA ROUTING TO VOICEMAIL AS WE DID'NT GOT 486 \n");
		}

	}else{
		for (; nSeq <= 4; nSeq++ ) {
			if(WSState->nDisasterMode == DISABLED ) {
				memset(fullquery,0,sizeof(fullquery));
				snprintf(fullquery, sizeof(fullquery)-1, formatstring, wsuri->group,wsuri->context, wsuri->context,wsuri->group,nSeq,servertz,wsuri->context, wsuri->group,nSeq,fgrphuntgrouplimit);
			}
			else{
				memset(fullquery,0,sizeof(fullquery));
				snprintf(fullquery, sizeof(fullquery)-1, disasterquery , wsuri->context, wsuri->group); 			 
			}
				
			LOG (L_WARN, "[process_vmopr_call]fullquery: %s\n", fullquery);
			ws_dbf.raw_query (h, fullquery, &res);
			if(res==NULL){
				LOG(L_WARN,"[process_vmopr_call]'res' is NULL");
				return -1;
			}
			
			if ((nRows = RES_ROW_N(res)) > 0){ 
				if(RES_ROWS (res)[0].values[3].type == DB_STRING ){
					LOG(L_WARN , "[process_vmopr_call]Groupname  for Remote Party ID %s\n", RES_ROWS(res)[0].values[3].val.string_val);
					memset(groupname, 0, sizeof(groupname));
					strncpy( groupname ,   RES_ROWS(res)[0].values[3].val.string_val, sizeof(groupname)-1);
				}
				if(is_nested_group && !(!strncmp(groupname,"Default",7) && strlen(groupname) == 7)  &&  nRows>MAX_BRANCHES ){
					if((nRes = process_nested_forking(msg,res,wsuri->token,wsuri->context,nRows)) == 1){
						is_fork_group = 1;
					}
				}
				if(!is_fork_group){
					nRes = create_branches(msg, res, 1, wsuri);
				}
				LOG(L_WARN,"[process_vmopr_call]1.nRes vale:%d",nRes);
				if (res)
					ws_dbf.free_result (h,res);
				if ( nRes < 0) {
					continue;
				}
				if (_nOrigSeq == 1) {
					/* Set the Remote-Party-ID only for first priority -Ramu 04/04/06*/
					if (strlen(groupname)) {
						ws_set_group_name(msg, groupname);	
					}
					ws_set_rpid(msg, registrar_domain,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
				}
				if ( t_on_failure ) {
					if(nRes == 2 ){
						nSeq = 4;	
					}
					t_on_failure (msg, (char *)(nSeq + 1), NULL);
					LOG(L_WARN, "[process_vmopr_call]On failure : %d\n", nSeq + 1 );
				}
				break;
			}
			if (res){
				ws_dbf.free_result (h, res);
			}
			LOG(L_WARN,"[process_vmopr_call]nSeq:%d",nSeq);
		}
	}
	
	LOG(L_WARN,"[process_vmopr_call] nRes value:%d",nRes);

	if ( (nRes < 0 )) {
		if (nFlag) {
			LOG(L_ERR,"[process_vmopr_call]nFlag vale:%d",nFlag);
			return worksmart_route_to_ivr2(msg,0);
		}
		//Route him to voicemail
		int nCode;
		memset(fullquery,0,sizeof(fullquery));
		snprintf(fullquery, sizeof (fullquery)-1, "SELECT s.code,s.title "
						" FROM special s, siteinfo_new si "
						" WHERE REPLACE(REPLACE(si.sitename,'.','-'),'_','-') = '%s' "
						" AND s.siteid = si.siteid "
						" AND s.code=%s", wsuri->context, wsuri->group);

		LOG (L_WARN, "[process_vmopr_call]fullquery: %s\n", fullquery);
		ws_dbf.raw_query(h, fullquery, &res);
		if(res==NULL){
			LOG(L_ERR,"[process_vmopr_call]'res' is NULL");
			return -1;
		}
		int nRespCode = get_resp_code(msg);
		if ( RES_ROW_N(res) == 0 ) {
			// Play the invalid Extension  26/12/05 Ramu
			memset(fullquery,0,sizeof(fullquery));
			snprintf (fullquery, sizeof (fullquery)-1, "vml-00-00-%s-%s", wsuri->context, wsuri->token);
			route_to_media_server(msg, fullquery,"");
		} else {
			nCode = get_db_int(RES_ROWS(res)[0].values[0]);
			memset(fullquery,0,sizeof(fullquery));
			snprintf (fullquery, sizeof (fullquery)-1, "vml-%d-%d-%s-%s", nCode, nRespCode, wsuri->context, wsuri->token);
			route_to_media_server(msg, fullquery,"");
		}
		if (res)
			ws_dbf.free_result (h, res);
		return 2;
	}
	return 1;
}


int process_ivr_command2 (struct sip_msg *msg, char *priority, struct ws_uri *wsuri, int nFlag ) {
	int nRes = -1,nRows = 0;
	char fullquery[5120] = "";
	char groupname[100] = "";
	db_res_t *res=NULL;
	int blagroup = 0, is_fork_group = 0;
	int nSeq = atoi(priority);

	/*This took only 0.02sec for getting output*/
	char * formatstring =" "
			" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs, sp.title,sp.code, 0 "
			" FROM   extendedpbxivrcommand e, siteinfo_new s, special sp"
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND sp.siteid = e.siteid "
			" AND sp.code = e.extensiongroupcode "
			" AND sp.disasterflag = 1 "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND sp.disasterphone IS NOT NULL "
			" UNION "
			" SELECT ph.phone,ph.phonetype, ph.ringsecs, sp.title ,p.agentid , p.wscallfwdcname"
			" FROM  extendedpbxivrcommand e, pbxusers p, phones ph, special sp, agentspecial a,  weeklyplans w,   siteinfo_new s "
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND e.siteid = a.siteid "
			" AND e.extensiongroupcode = a.code "
			" AND a.agentid = p.agentid "
			" AND p.id = ph.pbxid "
			" AND ph.dialplanid = w.dialplanid "
			" AND ph.groupactive = 1 "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND ph.seq = %d "
			" AND sp.code = e.extensiongroupcode "
			" AND sp.siteid = s.siteid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND a.siteid = p.siteid "
			" AND w.pbxid NOT IN (SELECT dm.pbxid FROM dialplanmasks dm WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto) "
			" AND w.weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
			" FROM pbxusers pu WHERE pu.id = ph.pbxid ) "
			" AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
			" OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
			" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
//			" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)"
			" UNION "
			" SELECT ph.phone,ph.phonetype,ph.ringsecs, sp.title,p.agentid , p.wscallfwdcname"
			" FROM   extendedpbxivrcommand e, pbxusers p, phones ph, agentspecial a, dialplanmasks dm, siteinfo_new s, special sp "
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND e.siteid = a.siteid "
			" AND sp.code = e.extensiongroupcode "
			" AND e.extensiongroupcode = a.code "
			" AND a.agentid = p.agentid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND a.siteid = p.siteid "
			" AND ph.groupactive = 1 "
			" AND p.id = ph.pbxid "
			" AND ph.dialplanid = dm.dialplanid " 
			" AND ph.pbxid = dm.pbxid "
			" AND sp.siteid = s.siteid "
			" AND ph.seq = %d "
			" AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
			" OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
			" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
			" AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto LIMIT %d ";
//			" AND now() BETWEEN date(dm.localfrom) AND date(dm.localto) "
//			" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)";

	char * spamformatstring =" "
			" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs, sp.title,sp.code, 0 "
			" FROM   extendedpbxivrcommand e, siteinfo_new s, special sp"
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND sp.siteid = e.siteid "
			" AND sp.code = e.extensiongroupcode "
			" AND sp.disasterflag = 1 "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND sp.disasterphone IS NOT NULL "
			" UNION "
			" SELECT ph.phone,ph.phonetype, ph.ringsecs, sp.title ,p.agentid , p.wscallfwdcname"
			" FROM  extendedpbxivrcommand e, pbxusers p, phones ph, special sp, agentspecial a,  weeklyplans w,   siteinfo_new s "
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND e.siteid = a.siteid "
			" AND e.extensiongroupcode = a.code "
			" AND a.agentid = p.agentid "
			" AND p.id = ph.pbxid "
			" AND ph.dialplanid = w.dialplanid "
			" AND ph.groupactive = 1 "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND ph.seq = %d "
			" AND sp.code = e.extensiongroupcode "
			" AND sp.siteid = s.siteid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND a.siteid = p.siteid "
			" AND w.pbxid NOT IN (SELECT dm.pbxid FROM dialplanmasks dm WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto) "
			" AND w.weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
			" FROM pbxusers pu WHERE pu.id = ph.pbxid ) "
			" AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
			" OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
			" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
//			" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)"
			" AND ((sp.permit_user_spam = 0 AND s.stir_route = 3) OR (p.stir_enable = 1 AND p.stir_route = 3) "
	        " OR (p.stir_enable = 0 AND s.stir_route = 3))"
			" UNION "
			" SELECT ph.phone,ph.phonetype,ph.ringsecs, sp.title,p.agentid , p.wscallfwdcname"
			" FROM   extendedpbxivrcommand e, pbxusers p, phones ph, agentspecial a, dialplanmasks dm, siteinfo_new s, special sp "
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND e.siteid = a.siteid "
			" AND sp.code = e.extensiongroupcode "
			" AND e.extensiongroupcode = a.code "
			" AND a.agentid = p.agentid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND a.siteid = p.siteid "
			" AND ph.groupactive = 1 "
			" AND p.id = ph.pbxid "
			" AND ph.dialplanid = dm.dialplanid " 
			" AND ph.pbxid = dm.pbxid "
			" AND sp.siteid = s.siteid "
			" AND ph.seq = %d "
			" AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
			" OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
			" OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
			" AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto LIMIT %d ";
//			" AND now() BETWEEN date(dm.localfrom) AND date(dm.localto) "
//			" AND ph.phone NOT IN (select username from pbxblausers WHERE agentid=p.agentid)";

	char  * disasterquery = ""
			"SELECT STRAIGHT_JOIN  sp.disasterphone, 6, 90, sp.title, sp.code, 0 "
			"  FROM   extendedpbxivrcommand e, siteinfo_new s, special sp"
			" WHERE  REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			"   AND e.siteid = s.siteid "
			"   AND sp.siteid = e.siteid "
		    	"   AND sp.code = e.extensiongroupcode "
			"   AND e.groupcode = %s "
			"   AND e.command = %s "
			"   AND sp.disasterphone IS NOT NULL";

char * bla_formatstring =" "
			" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs, sp.title,sp.code, 0 "
			" FROM   extendedpbxivrcommand e, siteinfo_new s, special sp "
			" WHERE e.groupcode = %s "
			" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND sp.siteid = e.siteid "
			" AND sp.code = e.extensiongroupcode "
			" AND sp.disasterflag = 1 "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND sp.disasterphone IS NOT NULL "
			" UNION "
			" (SELECT ph.username,2, ph.ringsecs, sp.title ,p.agentid, p.wscallfwdcname"
			" FROM  extendedpbxivrcommand e,pbxusers p,special sp,pbxblausers ph,agentspecial a,siteinfo_new s,appearance_info app "
			" WHERE e.groupcode = %s " 
	  		" AND e.command = %s "
			" AND e.siteid = s.siteid "
			" AND e.siteid = a.siteid "
			" AND e.extensiongroupcode = a.code "
			" AND a.agentid = p.agentid "
			" AND p.agentid = ph.agentid "
			" AND ph.siteid = s.siteid "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND sp.code = e.extensiongroupcode "
			" AND sp.siteid = s.siteid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
			" AND a.siteid = p.siteid "
			" AND a.seq_num = %d  "
			" AND app.sharedid = ph.username "
			" AND app.appearancestate = 'FREE' "
			" ORDER BY a.seq_num "
			" limit 1) LIMIT %d" ;

	if ( !wsuri->type || (strcmp(wsuri->type, "cmd"))){
		if ( !wsuri->type || (strcmp(wsuri->type, "vmd"))){
			LOG(L_ERR, "Invalid URI for process_ivr_vmd: Level1 error");
			return -1;
		}
	}
			
	if ( !wsuri->command || !wsuri->group || !wsuri->context ) {
		LOG(L_ERR, "Invalid URI for process_ivr_command: Level2 error");
		return -1;
	}
	nRes = get_extended_plan(msg, priority, wsuri);

	if(nRes == 1){
		/* Use rtp proxy */
		return WS_USE_RTP_PROXY;
	}
	else if(nRes == 2) {
		/* dont use rtp proxy */
		return WS_DONT_USE_RTP_PROXY;
	}
	else if(nRes == 3){//BLA Group Call
		LOG(L_WARN,"Got  the BLA Group\n");
		blagroup = 1;
		nRes = -1;
	}
	
	clear_branches();

	if(blagroup){
				
		LOG (L_ERR, "[blagroup::process_ivr_command2] MAX_BLA_LINES  : %d\n", max_bla_lines);
	/*****Added bla sequential routing for brguest fixes --kimo/vravi ****/
		if( _nSupSeq != 4){  /* _nSupSeq == 4 means Got response other than 486 for first sequence for BLA call --vravi */
			for (; nSeq <= max_bla_lines; nSeq++ ) {
				if(WSState->nDisasterMode == DISABLED ) {
					memset(fullquery,0,sizeof(fullquery));
					snprintf(fullquery, sizeof(fullquery)-1, bla_formatstring, wsuri->group,wsuri->command,wsuri->context, wsuri->group,wsuri->command,wsuri->context,nSeq,fgrphuntgrouplimit);
				}else{
					memset(fullquery,0,sizeof(fullquery));
					snprintf(fullquery, sizeof(fullquery)-1, disasterquery , wsuri->context, wsuri->group, wsuri->command);
				}
				LOG (L_ERR, "[blagroup::process_ivr_command2] fullquery : %s\n", fullquery);
				ws_dbf.raw_query (h, fullquery, &res);
				if(res==NULL){
					LOG(L_ERR,"[blagroup::process_ivr_command2] 'res' is NULL ... route to vml");
					nRes = -1; //Need to route to vml
					break;
				}
				if ((nRows = RES_ROW_N(res)) <= 0) {
					LOG(L_ERR,"[blagroup::process_ivr_command2] result set is not found .." );
					ws_dbf.free_result (h, res);
					continue;
				}
				if(RES_ROWS (res)[0].values[3].type == DB_STRING ){
					memset(groupname, 0, sizeof(groupname));
					strncpy( groupname ,   RES_ROWS(res)[0].values[3].val.string_val, sizeof(groupname)-1);
				}
				LOG(L_WARN,"[blagroup::process_ivr_command2] Creating the branches ...");
				if(is_nested_group && !(!strncmp(groupname,"Default",7) && strlen(groupname) == 7)  &&  nRows>MAX_BRANCHES ){
					if((nRes = process_nested_forking(msg,res,wsuri->token,wsuri->context,nRows)) == 1){
						is_fork_group = 1;
					}
				}
				if(!is_fork_group){
					if ((nRes = create_branches(msg, res, 1, wsuri)) < 0){
						LOG(L_WARN,"[blagroup::process_ivr_command2] nRes vale:%d , creating branches failed ,continuing ...",nRes);
						ws_dbf.free_result (h, res);
						continue;
					}
				}
				LOG(L_WARN,"[blagroup::process_ivr_command2] nRes vale:%d",nRes);
				
				/* Set the Remote-Party-ID only for first priority -Ramu 04/04/06*/
				if (strlen(groupname)) {
					LOG(L_WARN,"[blagroup::process_ivr_command2] Setting Group name as : %s\n", groupname);
					ws_set_group_name(msg, groupname);	
				}
				ws_set_rpid(msg, registrar_domain,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
				if ( t_on_failure ) {
					t_on_failure (msg, (char *)(nSeq + 1), NULL);
					LOG(L_WARN, "[blagroup::process_ivr_command2]On failure : %d\n",nSeq + 1);
				}
				if (res){
					ws_dbf.free_result (h, res);
				}
				break;
			}
			LOG(L_WARN, "[blagroup::process_ivr_command2] On failure route : %d , nRes : %d\n", nSeq + 1,nRes);
		}else{
			LOG(L_WARN, "[blagroup::process_ivr_command2] BLA ROUTING TO VOICEMAIL AS WE DIDN'T GOT 486 ");
		}

	}else{

		for (; nSeq <= 4; nSeq++ ) {
			if(WSState->nDisasterMode == DISABLED ) {
				memset(fullquery,0,sizeof(fullquery));
				if(IsSpamCall(msg)){
					snprintf(fullquery, sizeof(fullquery)-1, spamformatstring, wsuri->group,wsuri->command,wsuri->context, wsuri->group,wsuri->command,nSeq,wsuri->context,servertz, wsuri->group, wsuri->command,wsuri->context, nSeq,fgrphuntgrouplimit);
				}else{
					snprintf(fullquery, sizeof(fullquery)-1, formatstring, wsuri->group,wsuri->command,wsuri->context, wsuri->group,wsuri->command,nSeq,wsuri->context,servertz, wsuri->group, wsuri->command,wsuri->context, nSeq,fgrphuntgrouplimit);
				}
			}
			else{
				memset(fullquery,0,sizeof(fullquery));
				snprintf(fullquery, sizeof(fullquery)-1, disasterquery , wsuri->context, wsuri->group, wsuri->command); 			 }
			
				
			LOG (L_WARN, "fullquery: %s\n", fullquery);
			ws_dbf.raw_query (h, fullquery, &res);
			if(res==NULL){
				LOG(L_WARN,"[process_ivr_command2]'res' is NULL");
				return -1;
			}

			if ((nRows = RES_ROW_N(res)) > 0){ 
				if(RES_ROWS (res)[0].values[3].type == DB_STRING ){
					LOG(L_WARN , "Groupname  for Remote Party ID %s\n", RES_ROWS(res)[0].values[3].val.string_val);
					memset(groupname, 0, sizeof(groupname));
					strncpy( groupname ,   RES_ROWS(res)[0].values[3].val.string_val, sizeof(groupname)-1);
				}
				if(is_nested_group && !(!strncmp(groupname,"Default",7) && strlen(groupname) == 7) && nRows>MAX_BRANCHES ){
					if((nRes = process_nested_forking(msg,res,wsuri->token,wsuri->context,nRows)) == 1){
						is_fork_group = 1;
					}
				}
				if(!is_fork_group){
					nRes = create_branches(msg, res, 1, wsuri);
				}
				LOG(L_WARN,"[process_ivr_command2]1.nRes vale:%d",nRes);
				if (res)
					ws_dbf.free_result (h,res);
				if ( nRes < 0) {
					continue;
				}
				if (_nOrigSeq == 1) {
					/* Set the Remote-Party-ID only for first priority -Ramu 04/04/06*/
					if (strlen(groupname)) {
						ws_set_group_name(msg, groupname);	
					}
					ws_set_rpid(msg, registrar_domain,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
				}
				if ( t_on_failure ) {
					if(nRes == 2 ){
						nSeq = 4;	
					}
					t_on_failure (msg, (char *)(nSeq + 1), NULL);
					LOG(L_ERR, "On failure : %d\n", nSeq + 1 );
				}
				break;
			}
			if (res){
				ws_dbf.free_result (h, res);
				res=NULL;
			}
			LOG(L_WARN,"nSeq:%d",nSeq);
		}
	}
	
	LOG(L_WARN,"[process_ivr_command2] nRes value:%d",nRes);

	if ( (nRes < 0 )) {
		if (nFlag) {
			LOG(L_WARN,"nFlag vale:%d",nFlag);
			return worksmart_route_to_ivr2(msg,0);
		}
		//Route him to voicemail
		int nCode;
		memset(fullquery,0,sizeof(fullquery));
		snprintf(fullquery, sizeof (fullquery)-1, "SELECT s.code,s.title"
						"  FROM special s, extendedpbxivrcommand e, siteinfo_new si"
						" WHERE REPLACE(REPLACE(si.sitename,'.','-'),'_','-') = '%s'"
						"   AND si.siteid = s.siteid"
						"   AND s.siteid = e.siteid"
						"   AND e.extensiongroupcode = s.code"
						"   AND e.groupcode = %s"
						"   AND e.command = %s", wsuri->context, wsuri->group, wsuri->command);

		LOG (L_WARN, "fullquery: %s\n", fullquery);
		res = NULL;
		ws_dbf.raw_query(h, fullquery, &res);
		if(res==NULL){
			LOG(L_ERR,"[process_ivr_command2]'res' is NULL");
			return -1;
		}
		int nRespCode = get_resp_code(msg);
		if ( RES_ROW_N(res) == 0 ) {
			// Play the invalid Extension  26/12/05 Ramu
			memset(fullquery,0,sizeof(fullquery));
			snprintf (fullquery, sizeof (fullquery)-1, "vml-00-00-%s-%s", wsuri->context, wsuri->token);
			route_to_media_server(msg, fullquery, "");
		} else {
			nCode = get_db_int(RES_ROWS(res)[0].values[0]);
			memset(fullquery,0,sizeof(fullquery));
			snprintf (fullquery, sizeof (fullquery)-1, "vml-%d-%d-%s-%s", nCode, nRespCode, wsuri->context, wsuri->token);
			route_to_media_server(msg, fullquery, "");
		}
		if (res)
			ws_dbf.free_result (h, res);
		return 2;
	}
	return 1;
}

/* Establish connection with the database. This *SHOULD* be called first */
static int connect_db ()
{
	/* ??? Check the syntax of 'bind_dbmode' with the version you are using... */
	if (bind_dbmod (db_url, &ws_dbf) < 0 ){
    	// fprintf (stderr, "Error! Did you forget to load the database module?\n");
       	return -1;
    }
    else {
		// fprintf (stderr, "Successfully bound to database module\n");
	}
	h = ws_dbf.init (db_url); 
    if (!h )    {
    	LOG(L_ERR, "Error while initializing database connection\n");
       	return -1;
    }
    else    {
		LOG(L_WARN, "Connected to database\n");
	}

	return 0;
}
static int  init_shm (int _rank)
{
	FILE  * fp ;
	/*if (( shmid = shmget(SHM_KEY, sizeof(WSServerState), 0666|IPC_CREAT)) < 0) {
	        LOG(L_ERR , "Cann't create Shared Memory  %s\n\n" , strerror(errno));
			return -1;
	}  */
	if ((WSState  = (WSServerState *)shmat (shmid, (char *) 0, 0)) == ( WSServerState *) -1) {
		LOG(L_ERR , "Cann't attach Shared Memory\n\n");
		return  -1;
		
	}
	WSState->nDisasterMode = DISABLED ;
	WSState->nMediaServerType = PRIMARY;
	WSState->nMediaServerType = PRIMARY;
	WSState->nMediaServerType = PRIMARY;
    	if(_rank == 2){
        	fp = fopen("/usr/local/etc/wsmsicp/shm.id",  "w");
        	if( fp != NULL){
            		fprintf(fp , "%d", shmid);
            		fclose(fp);
        	}
    	}
	return 0;
}

static int  init_wsicpshm (int _rank)
{
	FILE  * fp ;

	if (( shmwsicpid = shmget(SHMWSICP_KEY, sizeof(WSServerState), 0666|IPC_CREAT)) < 0) {
		LOG (L_ERR,"Cann't create Shared Memory for Wsicp servers\n");
		return -1;
	}

	if ((WSicpState  = (WSicpServerState *)shmat (shmwsicpid, (char *) 0, 0)) == ( WSicpServerState *) -1) {
		LOG(L_ERR , "Cann't WSICP attach Shared Memory\n\n");
		return  -1;
		
	}
	WSicpState->nDisasterMode = DISABLED ;
	WSicpState->nICPServerType = PRIMARY;
    	if(_rank == 2){
        	fp = fopen("/usr/local/etc/wsmsicp/shmwsicp.id",  "w");
        	if( fp != NULL){
            		fprintf(fp , "%d", shmwsicpid);
            		fclose(fp);
        	}
    	}
	return 0;
}

/* Close database: This is the *LAST* function to be called */
static void mod_destroy (void)
{
    printf ("Destroying module\n");
	shmctl(shmid,IPC_RMID,NULL);
	shmdt(&shmid);
	shmctl(shmwsicpid,IPC_RMID,NULL);
	shmdt(&shmwsicpid);
}

int disconnect_db() {
	db_con_t * c;
	if ( ( c = h) != NULL ) {
		h = NULL;
		ws_dbf.close(c);
	}
	c=NULL;
	return 0;
}

static int child_init(int _rank ) {
	fprintf(stderr, "Child Init called : %d\n", _rank);
	generate_unique_process_range(_rank);
	if(init_shm(_rank))  return -1;
	if(init_wsicpshm(_rank))  return -1;
	return connect_db();
}

static inline int is_number(char *tocall, int len){
	int i=0;
	if(tocall[0]=='+'){
		i=1;
	}
	for(;i<len;i++){
		if(!isdigit(tocall[i])){
			return 0;
		}
	}
	return 1;
}

int process_thirdparty_phones(struct sip_msg   * msg ,char* phno , struct ws_uri * wsuri, int *branch,char *moc_agent, int fwdflag,int phno_len,int (*fptr)(int *),int *is_forked){
	char *domain, uri[256];
	int len, teluri = 0, nDcidHeader = 0;
	char buffer[DIDSIZE]="";
	char cnamfwdheader[1024] = "";
	if(phno==NULL){
		return 0;
	}
	LOG(L_WARN ,  "[process_thirdparty_phones] phone no. is '%s'\n",phno);
	domain = strchr( phno, '@');
	if (domain != NULL) {
		*domain = '\0';
		domain++;
	}
	len = strlen(phno);
	teluri = is_number(phno, len);
	LOG(L_WARN ,  "[process_thirdparty_phones] '%s' teluri %d\n",phno, teluri);
	if(domain==NULL || strlen(domain)==0) {
		/*Seems there is no domain name in the uri*/
		if(teluri ) {
			memset(uri, 0 , sizeof(uri));
			if(len > 5){
				if(moc_agent == NULL)
					moc_agent = wsuri->group;
				memset(uri,0,sizeof(uri));
				if(fwdflag == 1){
					len = snprintf(uri, sizeof(uri)-1, "sip:oms-%s-%s-%s-%s_%d@%s", phno, moc_agent,wsuri->context,wsuri->token,fwdflag,umslb_proxy);
				}else{
					len = snprintf(uri, sizeof(uri)-1, "sip:oms-%s-%s-%s-%s@%s", phno, moc_agent,wsuri->context,wsuri->token,umslb_proxy);
				}
				if(len > sizeof(uri)){
					LOG(L_WARN,"[Worksmart][process_thirdparty_phones] URI creation failed for '%s'. Cause:Buffer Overflow\n", phno);
					return 0;
				}
			}else {
				/*user extension it seems. Any way send this to proxy_domain  XXX Not sure what will happen  XXX*/
				memset(uri,0,sizeof(uri));
				len = snprintf(uri , sizeof(uri)-1, "sip:%s@%s", phno, proxy_domain);
				/*--- Callerid changes ------------*/
				if(fwdflag == 1){
					snprintf(cnamfwdheader, sizeof(cnamfwdheader)-1,"FwdCnam: %s \r\n",moc_agent);
					str cmd={cnamfwdheader,strlen(cnamfwdheader)};
					if (append_hf) {
						if(append_hf(msg, (char *)&cmd,0)==1)
							LOG(L_WARN,"append_hf success\n");
						else
							LOG(L_ERR,"append_hf failure\n");
					}
				}
				/*------------------------------*/

				if(len > sizeof(uri)){
					LOG(L_WARN,"[Worksmart][process_thirdparty_phones] URI creation failed for '%s'. Cause:Buffer Overflow\n", phno);
					return 0;
				}
			}
			

			if ( *branch  == 0 ) {
				(*branch)++;
				return  set_uri2(msg, uri );
			} else {
				(*branch)++;
				return  append_branch (msg, uri, len, 0, 0, 0);
			}

		}else{
			/*As there is no domain name, we are sending the call to all registrar available in list ------------*/
			get_uri_contact(msg , phno, branch, fptr, &nDcidHeader);
		}
		return 1;
	} else {
		/*There is domain name-- check whether it is our domain name */
		if((!strncmp( domain, registrar_domain,strlen(domain))) &&  (teluri == 0) ) {
			LOG(L_WARN , "[thirdparty ] '%s'  %d %s\n", registrar_domain, teluri, phno);	
			get_uri_contact(msg , phno, branch,fptr, &nDcidHeader);
		}else{
			/*This is what is third party uri  :)*/
			/*Added by Tulsi for fix bugid:5016  11-12-08*/
			if(teluri == 1)
			{
					LOG(L_WARN , "[Afternormalize_Thirdparty_Number ] Before normalize :%s\n",phno);
					memset(buffer, 0 , sizeof(buffer));
					normalize(phno, buffer, sizeof(buffer),phno_len);
					LOG(L_WARN , "[Afternormalize_Thirdparty_Number ] After normalize :%s\n",buffer);
					memset(uri,0,sizeof(uri));
					/*--- Callerid changes ------------*/
					if(fwdflag == 1){
						snprintf(cnamfwdheader, sizeof(cnamfwdheader)-1,"FwdCnam: %s \r\n",moc_agent);
						str cmd={cnamfwdheader,strlen(cnamfwdheader)};
						if (append_hf) {
							if(append_hf(msg, (char *)&cmd,0)==1)
								LOG(L_WARN,"append_hf success\n");
							else
								LOG(L_ERR,"append_hf failure\n");
						}
					}
					/*------------------------------*/
					len = snprintf(uri, sizeof(uri)-1, "sip:%s@%s",  buffer, domain);
			}else if(phno != NULL){
					memset(uri,0,sizeof(uri));
					len = snprintf(uri, sizeof(uri)-1, "sip:%s@%s",  phno, domain);
			}else{
					LOG(L_WARN, "[Worksmart][process_thirdparty_phones] OOPS Phno is NULL...:%s\n", phno);
					return 0;
			}
			if( len > sizeof(uri)){
				LOG(L_WARN , "[Worksmart][process_thirdparty_phones] URI creation failed for '%s'. Cause:Buffer Overflow\n", phno);
				/*We dont want to proceed -- Same error will come again */
				return 0;
			}
			if(is_forked){
				*is_forked = 1;
			}
			
			if ( *branch  == 0 ) {
				(*branch)++;
				return  set_uri2(msg, uri );
			} else {
				(*branch)++;
				return  append_branch (msg, uri, len, 0, 0, 0);
			}
		}
	}
	return 1;
}

int normalize_thirdparty_uri(char* phno , char  * uri , struct ws_uri * wsuri,int uri_len){
	char *domain;
	int len, isNumber;
	
	if((phno==NULL) || (uri == NULL)){
			return 0;
	}
	len = strlen(phno);
	isNumber = is_number(phno, len);
	domain = strchr( phno, '@');
	if(domain==NULL) {
		/*Seems there is no domain name in the uri*/
		if(isNumber &&  len > 5){
			snprintf(uri,uri_len-1, "sip:oms-%s-%s-%s-%s@%s", phno, wsuri->group,wsuri->context,wsuri->token,umslb_proxy);
		}else{
			snprintf(uri,uri_len-1, "sip:%s@%s", phno,proxy_domain );
		}
		return 1;
	} else {
		snprintf(uri,uri_len-1, "sip:%s", phno);	
	}
	return 1;
}

int get_uri_contact(struct sip_msg * msg, char *phoneno, int *append, int (*fptr)(int *), int *nDcidHeader)
{
	char query[512];
	db_res_t *res=NULL;
	db_row_t row;
	char uri[1024];
	int row_count=0;
	int ret=0,dcid=0, nAddDataceneter_id = 0;
	char *ptr = NULL, temp_phone[256] = "", header_value[64] = "";
	struct lump* tmp, *t = NULL;

	bzero(query,sizeof(query));
	
	if(phoneno){
		strncpy(temp_phone, phoneno, sizeof(temp_phone)-1);	
		if((ptr = strchr(phoneno, '_'))){
			*ptr = '\0';
		}
	}
	snprintf(query,sizeof(query)-1,GET_CONTACT_QUERY,phoneno);
	
	LOG(L_WARN, "[get_uri_contact] %s\n", query);
	ws_dbf.raw_query(h, query, &res);
	
	if (res == NULL)
	{
		return 0;
	}
	
	row_count = RES_ROW_N(res);
	if (row_count <= 0)
	{
		ws_dbf.free_result(h, res);
		return 0;
	}
	if(fptr){
		fptr(&row_count);
	}
	while (row_count > 0)
	{
		row = RES_ROWS(res)[row_count-1];
		row_count--;
		if(!(VAL_NULL(&row.values[2])))
			dcid = get_db_int(row.values[2]);
		memset(uri,0,sizeof(uri));
		if(datacenter_id && dcid && dcid != datacenter_id && !(VAL_NULL(&row.values[1])) && strlen(row.values[1].val.string_val) > 0){ /*routing to other data center */
			if(engage_mediaanchor == 1 && nAddDataceneter_id == 0 && (nDcidHeader != NULL) && *nDcidHeader == 0){ /*'Datacenterid' header already added for this branch no need to add again ---Anik/Badree 05sep2019  */
				tmp=msg->add_rm;
				/* checking if 'Datacenterid' header already added from different call routed through msicp then removing it ---Anik/Badree 01oct2019  */
				if(tmp) {
					for(t=tmp->next;t;t=t->next) {

						if(t->before && t->before->len >0 && t->before->op == LUMP_ADD && t->before->type == 0) {
							if((t->before->u.value) && (strlen(t->before->u.value)>0) && !strncmp(t->before->u.value,"Datacenterid:", 13) ) {
								LOG(L_ERR,"[get_uri_contact] SETTING LUMP_DEL  PTR : %p   VALUE : %s and TYPE : %d  AND  LEN : %d !!!!!!!\n",t->before,t->before->u.value,t->before->type,t->before->len);
								t->before->op = LUMP_DEL;
							}
						}
					}
				}


				memset(header_value,0,sizeof(header_value));
				snprintf(header_value, sizeof(header_value)-1,"Datacenterid: %d\r\n",datacenter_id);
				add_header(msg,header_value,strlen(header_value));
				LOG(L_ERR,"[get_uri_contact] this is other data center call adding header 'Datacenterid'\n");
				nAddDataceneter_id = 1;
				*nDcidHeader = 1;
			}

			snprintf(uri, sizeof(uri)-1, "sip:%s@%s", temp_phone, row.values[1].val.string_val);
		} else if(!(VAL_NULL(&row.values[0])) && strlen(row.values[0].val.string_val) > 0 ){
			snprintf(uri, sizeof(uri)-1, "sip:%s@%s", temp_phone, row.values[0].val.string_val);
		}
		LOG(L_WARN, "[get_uri_contact] branch is %s\n", uri);
		if (*append == 0)
		{
			ret = set_uri2(msg, uri);
			
			if (ret < 0)
			{
				LOG(L_WARN, "[get_uri_contact] Failed to Create First Branch\n");
				ws_dbf.free_result(h,res);
				return 0;
			}
			else
			{
				(*append)++;
			}
		}
		else
		{
			ret = append_branch(msg, uri, strlen(uri), 0, 0, 0);
			if (ret < 0)
			{
				 LOG(L_WARN, "[get_uri_contact] Failed to Append Branch\n");
				 ws_dbf.free_result(h,res);
				 return 0;
			}
		}
	}
	ws_dbf.free_result(h, res);
	return 1;
}

/*Added by Tulsi for Virtual Seat implementation-Tulsi-21-07-09*/
int isInPhoneList(char *phoneno,int nSize,int count)
{
	int i=0, nFound = 0;
    /*Added Branch checks --kkmurthy/vijay/Tulsi 2009-10-01*/	
	while ((i < MAX_BRANCHES) && (phonelist[i][0] != 0)){
		if(((strlen(phonelist[i]) > 0) && (!strncmp(phonelist[i],phoneno,strlen(phonelist[i])))))
		{
			LOG (L_WARN, "[isInPhoneList]Which Row: %d. Already Routed for this phoneno**%s** == phonelist[%d]**%s**\n",count,phoneno,i,phonelist[i]);
			nFound = 1;
			break;
		}
		i++;
	}
    /*Added Branch checks --kkmurthy/Tulsi 2009-10-01*/	
	if ((count < MAX_BRANCHES) && (!nFound) ) {
		strncpy (phonelist[count],phoneno,sizeof(phonelist[count]));
	}
	return nFound;
}

/*Framing main fork group uri for hunt group calls --Anusha */
int process_nested_forking(struct sip_msg *msg,db_res_t * result,char *token,char *cSitename,int nRows){
	
	char cToken[64]="",fgroup_uri[256]="";
	int nRes = 0;

	if(!token || !cSitename || !result) {
		LOG(L_WARN, "[process_nested_forking] cToken or cSitename or result is NULL\n");
		return -1;
	}

	memset(cToken,0,sizeof(cToken));
	strncpy(cToken,token,sizeof(cToken)-1);
	if(strstr(cToken,"wspib")){
		strncpy(cToken,cToken+5,sizeof(cToken)-1);
	}

	if(shm_create(result,cToken,nRows) < 0)
			return -1;
	snprintf(fgroup_uri,sizeof(fgroup_uri)-1 , "sip:fgrp-%d-%d-%s-%s_%d@%s:%d", 0, nRows, cSitename, cToken,nRows,own_ip,port_no);

	if(_nOrigSeq == 1){
		nRes = set_uri2(msg, fgroup_uri);
	}else{
		nRes = append_branch(msg, fgroup_uri , strlen(fgroup_uri), 0,0,0);
	}
	return nRes;
}

/*Create the shared memory to hunt group branches --Anusha*/
int shm_create(db_res_t * result,char *cToken,int nRows)
{
	int i=0,key,flag = 0;
	db_row_t row;
	
	if(!cToken || !result) {
		LOG(L_WARN, "[shm_create] cToken or result is NULL\n");
		return -1;
	}
	typedef struct {
		int nToken;
		volatile int nMaxForksInGrp;
		int nCount;
		short nProcessed[nRows];
		int nRingSecs[nRows];
		int nPhoneType[nRows];
		int nFwdflag[nRows];
		char cAgentid[nRows][256];
		char cUserName[nRows][256];
		char cGrp_Title[nRows][256];
	} WSHGroup;

	WSHGroup *WSState1 = NULL;
	key=atoi(cToken);
	if ((shmforkgrpid = shmget(key, sizeof(WSHGroup), 0666|IPC_CREAT|IPC_EXCL)) < 0) {
		LOG(L_ERR , "[shm_create] Cann't get Shared Memory Key: %d Token: %s Error: %s \n\n",key,cToken,strerror(errno));
		return -1;
	}
	flag = select_shmid(shmforkgrpid);
	if(flag == 0){
		insert_shmid_shm(shmforkgrpid);
	}
	
	if ((WSState1  = (WSHGroup *) shmat (shmforkgrpid, NULL, 0)) == (WSHGroup *) -1) {
		LOG(L_ERR , "[shm_create] Cann't attach Shared Memory Key : %d Token: %s Error : %s \n\n",key,cToken,strerror(errno));
		return  -1;
	}
	memset(WSState1,0,sizeof(WSHGroup));
	WSState1->nToken = key;
	while (nRows>i)
	{
		row = RES_ROWS (result)[i];
		if (row.values[0].nul == FALSE) {
			snprintf(WSState1->cUserName[i] , sizeof(WSState1->cUserName[i])-1 , "%s",row.values[0].val.string_val);
		}
		WSState1->nPhoneType[i]=get_db_int(row.values[1]);
		WSState1->nRingSecs[i]=get_db_int(row.values[2]);
		if(RES_ROWS(result)[i].values[3].val.string_val){
			snprintf(WSState1->cGrp_Title[i],sizeof(WSState1->cGrp_Title[i])-1,"%s",RES_ROWS(result)[i].values[3].val.string_val);	
		}
		if(RES_ROWS(result)[i].values[4].val.string_val){
			snprintf(WSState1->cAgentid[i],sizeof(WSState1->cAgentid[i])-1,"%s",RES_ROWS(result)[i].values[4].val.string_val);	
		}
		if(RES_ROWS(result)[i].values[5].val.string_val){
			WSState1->nFwdflag[i] = get_db_int(RES_ROWS(result)[i].values[5]);
		}
		i++;
	}
	LOG(L_ERR,"[shm_create] Created SHM Node:%p id:%d SIZE : %ld TOKEN : %s:%d Rows: %d=%d \n", WSState1, shmforkgrpid,sizeof(WSHGroup),cToken, WSState1->nToken, i, nRows);
	shmdt(WSState1);
	
	return 0;
}

static int append_forkeduri(struct sip_msg * msg, char *uri, char *cToken, int **append)
{
	int ret=-1;
	char fgroup_uri[128]="";

	if(!msg || !uri || !cToken) {
		LOG(L_WARN, "[append_forkeduri] Any of above value is NULL\n");
		return -1;
	}

	snprintf(fgroup_uri , sizeof(fgroup_uri)-1 , "sip:%s@%s:%d",uri,own_ip,port_no);
	if (**append == 0 )
	{
		ret = set_uri2(msg, fgroup_uri);
		if (ret < 0)
		{
			LOG(L_WARN, "[append_forkeduri] Failed to create 1st branch\n");
			return 0;
		}
		else
		{
			(**append)++;
		}
	}
	else
	{
		if (append_branch(msg, fgroup_uri, strlen(fgroup_uri), 0,0,0) == -1){
			LOG(L_ERR, "[append_forkeduri] Error while appending a branch\n");
			return -1;
		}
	}
	return 0;
}

int UpdateShmCounter(char *cToken,char *cUri)
{
	int nKey = 0,row_count=0;
	char *ptr = NULL,fork_uri[128]="";
	if(!cToken || !cUri) {
		LOG(L_ERR, "[UpdateShmCounter]Received empty token value\n");
		return -1;
	}
	strncpy(fork_uri,cUri,sizeof(fork_uri)-1);
	if((ptr = strrchr(fork_uri,'_'))){
		*ptr='\0';
		ptr++;
		row_count = atoi(ptr);
	}

	typedef struct {
		int nToken;
		volatile int nMaxForksInGrp;
		int nCount;
		short nProcessed[row_count];
		int nRingSecs[row_count];
		int nPhoneType[row_count];
		int nFwdflag[row_count];
		char cAgentid[row_count][256];
		char cUserName[row_count][256];
		char cGrp_Title[row_count][256];
	} WSHGroup;

	WSHGroup * WSState1 = NULL;

	nKey = atoi(cToken);
	if ((shmforkgrpid = shmget(nKey, sizeof(WSHGroup), 0666)) < 0) {
		LOG(L_ERR , "[UpdateShmCounter] ERROR: Cann't get Shared Memory Token: %s:%d Error: %s \n\n",cToken,nKey,strerror(errno));
		return -1;
	}
	if ((WSState1  = (WSHGroup *) shmat (shmforkgrpid, NULL, 0)) == (WSHGroup *) -1) {
		LOG(L_ERR , "[UpdateShmCounter] ERROR: Cann't attach Shared Memory\n\n");
		return  -1;
	}
	WSState1->nCount++;
	LOG(L_ERR,"[UpdateShmCounter] Node:%p id:%d token:%s Count:%d \n", WSState1, shmforkgrpid, cToken, WSState1->nCount);
	shmdt(WSState1);
	return 1;
}

/*Framing fgrp uri's to hunt group calls --Anusha*/
int generate_forkgroup(int nStart, int nEnd, char * uri, struct sip_msg * msg, char *cSitename,char *cToken, int nPrevSize, int *nAppend,int nVal,int nRows,int uri_len)
{
	int nSize = nEnd  - nStart + 1;
	int nNewStart = nStart, nNewEnd = nStart - 1,ni=0;
	int nGroups = 0 , nDiff = 0;

	if(!msg || !cSitename || !cToken || !uri) {
		LOG(L_WARN, "[generate_forkgroup] Any of above value is NULL\n");
		return -1;
	}
	nDiff = nPrevSize - nSize;
	if ( nSize > MAX_FORK_BRANCHES ) {
		nGroups	= nSize/MAX_FORK_BRANCHES + (nSize%MAX_FORK_BRANCHES > 0 ?1:0);
		if((nDiff < MAX_FORK_BRANCHES) && strlen(uri) >0) {
			nVal =1;
			if((nEnd - nStart)<MAX_FORK_BRANCHES && strlen(uri)>0)
				UpdateShmCounter(cToken,uri);			
			append_forkeduri(msg,uri,cToken,&nAppend);
		}
		for ( ni = 0; (ni < MAX_FORK_BRANCHES && nNewEnd < nEnd); ni++ ) {
			nNewStart = nNewEnd + 1;
			if ( nGroups > MAX_FORK_BRANCHES ) {
				nNewEnd = nNewStart + nGroups -1; 
			} else {
				nNewEnd = nNewStart + MAX_FORK_BRANCHES - 1; 
			}
			if ( nNewEnd > nEnd ) {
				nNewEnd = nEnd;
			}
			if(nRows)
				snprintf(uri,uri_len-1, "fgrp-%d-%d-%s-%s_%d", nNewStart, nNewEnd, cSitename, cToken,nRows);
			else
				snprintf(uri,uri_len-1, "fgrp-%d-%d-%s-%s", nNewStart, nNewEnd, cSitename, cToken);
				generate_forkgroup(nNewStart, nNewEnd, uri, msg, cSitename,cToken,nPrevSize,nAppend,nVal,nRows,sizeof(uri));
		}
	}
	else
	{
		if(!nVal) {
			if((nEnd - nStart)<MAX_FORK_BRANCHES && strlen(uri)>0)
				UpdateShmCounter(cToken,uri);			
			append_forkeduri(msg,uri,cToken,&nAppend);
		}
	}
	return 1;
}

/*Branches are created for nested fork group uri's --Anusha*/
int create_fork_branches ( struct sip_msg * msg , struct ws_uri * wsuri )
{
	int nStart = 0, i,j,nRows=0;
	int nEnd = 0, nKey,ni=0,nSec=5,fwdflag=0, nCalledDisaster=0,is_forked = 0, nDcidHeader = 0;
	short invalid_phone_no=0;
	char buffer[32]="",*moc_agentid = NULL,agentid[128]="",*ptr = NULL,loc_phoneno[256]="",db_phone[256]="";

	int nRouteType = 0, nDcid = 0;
	struct ws_uri wsuri1 = {};
	char cNewUri[256] = "", cLocationIP[32] = "", grp_title[128] = "", cRequestUri[256] = "";

	str cmd1={"Record-Route",12};

	if(!wsuri || !msg) {
		LOG(L_ERR, "[create_fork_branches] Got empty wsuri structure...\n");
		return -1;
	}
	if(wsuri->token)
	{
		if((ptr = strchr(wsuri->token,'_'))){
			*ptr='\0';
			ptr++;
			nRows = atoi(ptr);
		}
	}

	typedef struct {
		int nToken;
		volatile int nMaxForksInGrp;
		int nCount;
		short nProcessed[nRows];
		int nRingSecs[nRows];
		int nPhoneType[nRows];
		int nFwdflag[nRows];
		char cAgentid[nRows][256];
		char cUserName[nRows][256];
		char cGrp_Title[nRows][256];
	} WSHGroup;

	nStart = atoi(wsuri->command);
	WSHGroup * WSState1 =NULL;

	nEnd = atoi(wsuri->group);
	nKey = atoi(wsuri->token);
	
	if (remove_hf) {
		if(remove_hf(msg, (char *)&cmd1,0)==1)
			LOG(L_WARN,"remove_hf success token:%s rows:%d \n", wsuri->token, nRows);
		else
			LOG(L_ERR,"remove_hf failure token:%s rows:%d \n", wsuri->token, nRows);
	}

	if (nKey && (shmforkgrpid = shmget(nKey, sizeof(WSHGroup), 0666)) < 0) {
		LOG(L_ERR , "[create_fork_branches] Cann't get Shared Memory Token: %s:%d Error: %s \n\n",wsuri->token,nKey,strerror(errno));
		return -1;
	}
	if ((WSState1  = (WSHGroup *) shmat (shmforkgrpid, NULL, 0)) == (WSHGroup *) -1) {
		LOG(L_ERR , "[create_fork_branches]Cann't attach Shared Memory\n\n");
		return  -1;
	}
	int update_count(int *p) {
		int res = 0;
		if(!p){
			LOG(L_ERR,"[create_fork_branches::update_count] P is Null So returning \n");
			return 0;
		}
		res = WSState1->nMaxForksInGrp + *p - cMaxForksInGrp;
		if(res > 0){
			*p-=res;
			LOG(L_ERR,"[create_fork_branches::update_count] Limit Reached Token: %s Forks: %d/%d res:%d \n",wsuri->token, WSState1->nMaxForksInGrp, cMaxForksInGrp, res);
		}
		WSState1->nMaxForksInGrp += *p;
		return *p;
	}

	if( (WSState1->nProcessed[nStart] == 0xA) && (WSState1->nProcessed[nEnd-1] == 0x38) ) {
		LOG(L_ERR , "[create_fork_branches]: Is it reinvite for FGRP Token:%s id:%d range: %d->%d (%d) End already nProcessed:%d \n", 
						wsuri->token, shmforkgrpid,  nStart, (nEnd-1), nRows, WSState1->nProcessed[nEnd-1]); 
		shmdt(WSState1);
		return 1;
	} else {
		WSState1->nProcessed[nStart] = 0xA;
		WSState1->nProcessed[nEnd-1] = 0x38;
	}
	update_shmid(shmforkgrpid,1); /* For Updating insertion time before processing FGRP Request */	
	for (i=nStart; i <= nEnd  ; i++) {
		
		if(WSState1->nMaxForksInGrp >= cMaxForksInGrp){
		
			LOG(L_ERR,"[create_fork_branches] Ohh Max Braches forked, no need to process range: %d->%d (%d) on TOKEN: %s TotalForks: %d Conf: %d \n", 
						nStart, nEnd, nRows, wsuri->token,WSState1->nMaxForksInGrp,cMaxForksInGrp);
			break;
		}

		if(WSState1->cUserName[i] != NULL && strlen(WSState1->cUserName[i]) > 0) {
			if(WSState1->nRingSecs[i] <= 0){
				LOG(L_WARN, "[create_fork_branches] SETTING NSEC TO 20 as db nsec is less than zero  \n");
				WSState1->nRingSecs[i] = 20;
			}
			is_forked = 0;
			memset(db_phone,0,sizeof(db_phone));
			switch(WSState1->nPhoneType[i]){
				case 1:
				case 2:
				case 3:
					if(WSState1->nFwdflag[i] == 1 && !strchr(WSState1->cGrp_Title[i], '@')){
						snprintf(db_phone,sizeof(db_phone)-1,"%s_%s",WSState1->cUserName[i],WSState1->cGrp_Title[i]);
					}else if(WSState1->nFwdflag[i] == 1 && strlen(group_name)>0){
						snprintf(db_phone,sizeof(db_phone)-1,"%s_%s",WSState1->cUserName[i],group_name);
					}else{
						strncpy(db_phone, WSState1->cUserName[i], sizeof(db_phone)-1);
					}
					get_uri_contact(msg,db_phone, &ni,&update_count, &nDcidHeader);
					if (nSec < WSState1->nRingSecs[i]) {
						nSec = WSState1->nRingSecs[i] ;
					}
					break;
				case 4:
					moc_agentid = NULL;
					if(WSState1->cAgentid[i] != NULL && strlen(WSState1->cAgentid[i]) > 0) {
						strncpy(agentid,WSState1->cAgentid[i],sizeof(agentid));
						ptr = strchr(agentid,'@');
						if(ptr)
							*ptr = '\0';
						moc_agentid = agentid;
					}
					fwdflag=WSState1->nFwdflag[i];
					if ( strncmp(WSState1->cUserName[i],"sip:",4) ) {
						process_thirdparty_phones(msg ,WSState1->cUserName[i] ,wsuri, &ni,moc_agentid,fwdflag,sizeof(WSState1->cUserName[i]),&update_count,&is_forked);
					}else{
						process_thirdparty_phones(msg , &WSState1->cUserName[i][4],wsuri, &ni,moc_agentid,fwdflag,sizeof(WSState1->cUserName[i])-4,&update_count,&is_forked);
					}	
					if (nSec < WSState1->nRingSecs[i]) {
						nSec = WSState1->nRingSecs[i] ;
					}
					break;
				case 6:
					nCalledDisaster = 1;
				case 5:
					if (! strlen ( WSState1->cUserName[i])) break;
					for (j=0; WSState1->cUserName[i][j]; j++ ) {
						if (!isdigit (WSState1->cUserName[i][j])) {
							invalid_phone_no=1;
							break;
						}
					}
					memset(buffer, 0 , sizeof(buffer));
					normalize(WSState1->cUserName[i], buffer, sizeof(buffer),sizeof(WSState1->cUserName[i])-1);
					LOG(L_WARN,"[create_fork_branches] Anlog/Cell Phone = %s int pref:%s len:%d ",buffer,international_prefix,prefix_length);
					moc_agentid = NULL;
					if(WSState1->cAgentid[i] != NULL && strlen(WSState1->cAgentid[i]) > 0) {
						strncpy(agentid,WSState1->cAgentid[i],sizeof(agentid));
						ptr = strchr(agentid,'@');
						if(ptr)
							*ptr = '\0';
						moc_agentid = agentid;
					}
					fwdflag=WSState1->nFwdflag[i];
					memset(loc_phoneno,0,sizeof(loc_phoneno));

					/*handling routing plan to ext calls */
					if((strlen(buffer) >= MINEXTLEN) && (strlen(buffer) <= nMaxExtLen)) {
						
						if(IsSpamCall(msg) == 1) {
							nRouteType = CheckSpamSettings(msg, atoi(buffer), wsuri->context);
							if((nRouteType == STIR_REJECT || nRouteType == STIR_REJECT_WITH_PROMPT) ) {
								LOG(L_ERR, "[create_fork_branches] Spam settings are not allowed so not creating this branch\n");
								break;
							}
						}
				
						memset(&wsuri1, 0x00, sizeof(struct ws_uri));
						memset(cNewUri,0,sizeof(cNewUri));
						if(wsuri && wsuri->type && wsuri->group && wsuri->context && wsuri->token) {
							wsuri1.command = buffer;
							wsuri1.type    = wsuri->type;
							wsuri1.group   = wsuri->group;
							wsuri1.context = wsuri->context;
							wsuri1.token   = wsuri->token;

							/* re-setting to default*/
							nDcid = 0;
							memset(cLocationIP, 0, sizeof(cLocationIP));
							memset(grp_title, 0, sizeof(grp_title));
							strncpy(grp_title, WSState1->cGrp_Title[i], sizeof(grp_title)-1);	
							if((frame_uri(msg, &wsuri1, cNewUri, sizeof(cNewUri), moc_agentid, grp_title, &nDcid)) == 0) {
								if(strlen(cNewUri) > 0) {
									if(enable_dc_transfer && nRouteAcdAndConfCallsToHomeDc && nDcid && nDcid != datacenter_id && (!strncmp(cNewUri,"sip:acd",7) || !strncmp(cNewUri,"sip:cnf",7))) {
										LOG(L_ERR , "[create_fork_branches]  User Set Across DC ACD/CNF EXT in Routingplan. So routing Call to Home DC \n");

										memset(cRequestUri,0,sizeof(cRequestUri));
										snprintf(cRequestUri, sizeof(cRequestUri)-1,"WSRequestURI: %s\r\n",cNewUri+4); /* Avoid sip: */
										add_header(msg,cRequestUri,strlen(cRequestUri));

										GetRemoteDataCenterIP(nOtherDataCenterId,cLocationIP,sizeof(cLocationIP));
									}
									if(strlen(cLocationIP)>0) {
										snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:%s@%s", wsuri1.command, cLocationIP);
									}else{
										if( WSState->nMediaServerType == PRIMARY){
											snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "%s@%s", cNewUri, umslb_proxy);
										}else{
											snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "%s@%s", cNewUri, umslb_proxy1);
										}
									}
								}
							}
						}
					}else{ 
						if(fwdflag == 1){
							snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:oms-%s-%s-%s-%s_%d@%s", buffer, moc_agentid,wsuri->context,wsuri->token,fwdflag,umslb_proxy);
						}else{
							snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:oms-%s-%s-%s-%s@%s", buffer, moc_agentid,wsuri->context,wsuri->token,umslb_proxy);
						}
					}
					LOG(L_WARN,"[create_fork_branches]PRIMARY>>>>>>>>>>>>:%s Token : %s i: %d \n",loc_phoneno,wsuri->token,i);
					if ( ni == 0 ) {
						set_uri2(msg, loc_phoneno);
					} else {
						append_branch (msg, loc_phoneno, strlen (loc_phoneno), 0, 0, 0);
					}
					ni++;
					if (nSec < WSState1->nRingSecs[i]) {
						nSec = WSState1->nRingSecs[i] ;
					}
					is_forked = 1;	
					break;
			}
		}
		if(is_forked){
			WSState1->nMaxForksInGrp++;
		}
	}
	
	WSState1->nCount--;
	if(WSState1->nCount < 1) {
		LOG(L_ERR,"[create_fork_branches] Going to remove id : %d Range: %d->%d (%d) on TOKEN: %s TotalForks: %d \n",
					shmforkgrpid, nStart, nEnd, nRows, wsuri->token,WSState1->nMaxForksInGrp);
	}
	update_shmid(shmforkgrpid,WSState1->nCount);
	shmdt(WSState1);

	if(ni){
		ws_set_iattr(msg, nSec);
	}
	if ((( _nOrigSeq != 1) && (_nOrigSeq != 8) && (ni > 0))) {
		LOG (L_WARN, "[create_fork_branches] Dummy Branch Added\n");
		append_branch(msg, NULL, 0, 0, 0, 0);
	}
	if(nCalledDisaster && (ni > 0) && (_nSupSeq == 0))
	{
		return 2;
	}
	
	LOG(L_WARN,"[create_fork_branches] ni :%d\n",ni);

	return (ni != 0) ? 1 : -1;
}


int prepare_contact_spam_route_fail(struct sip_msg *msg, int nRouteType) {

	char *ptr = NULL;
	char cContactHeader[256] = "", cContactIp[32]="";

	memset(cContactHeader, 0, sizeof(cContactHeader));
	memset(cContactIp, 0, sizeof(cContactIp));

	/* parsing contact header to get server Ip */
	if( parse_headers (msg, HDR_CONTACT, 0) == -1) {
		LOG(L_ERR," [prepare_contact_spam_route_fail] parsing contact header failed \n");
		return -1;
	}

	if(msg->contact == NULL || msg->contact->body.len == 0 ) {
		LOG(L_ERR," [prepare_contact_spam_route_fail] contact is null \n");
		return -1;
	}

	snprintf(cContactHeader, sizeof(cContactHeader)-1, "%.*s", msg->contact->body.len, msg->contact->body.s);
	ptr = strchr(cContactHeader,'@');
	if(ptr && ++ptr) {
		strncpy(cContactIp,ptr,sizeof(cContactIp)-1);   // Server IP
	}

	if(strlen(cContactIp) == 0) {
		LOG(L_ERR," [prepare_contact_spam_route_fail] Ip in contact is empty : cContactHeader %s  \n", cContactHeader);
		return -1;
	}

	memset(cContactHeader, 0, sizeof(cContactHeader));
	snprintf(cContactHeader, sizeof(cContactHeader)-1,"Contact: <sip:tranfail-%d-0-0-0@%s\r\n", nRouteType, cContactIp);

	if (add_lump_rpl(msg, cContactHeader, strlen(cContactHeader), LUMP_RPL_HDR) == 0) {
		LOG(L_ERR,"[prepare_contact_spam_route_fail] append_hf failure Contact: %s \n", cContactHeader);
		return -1;
	}

	LOG(L_ERR,"[prepare_contact_spam_route_fail] contact header in spam route fail is: %s \n", cContactHeader);
	return 1;
}

int IsSpamCall(struct sip_msg *msg) {

	struct to_body * fromb;
	struct sip_uri uri;
	char cDisplay[128] = "";
	
	
	parse_from_header(msg);
	fromb = get_from (msg);
	if (fromb != NULL){
		parse_uri(fromb->uri.s, fromb->uri.len, &uri);
		if (fromb->display.len > 0) {
			memset(cDisplay, 0x00, sizeof(cDisplay));	
			snprintf(cDisplay, sizeof(cDisplay)-1, "%.*s", fromb->display.len,fromb->display.s);
			if(strstr(cDisplay,"SPAM")){
				return 1;
			}
		}
	}
	return 0;
}

int CheckSpamSettings(struct sip_msg *msg, int nExtension, char *cSiteName , int *nStir_Enable) {	

	char cQuery[1024] = "";
	int nStirRoute = 0, nStirEnable = 0, nSiteId  = 0;

	db_res_t *res=NULL, *res1 = NULL;
	db_row_t row;

	char *cSelectSpamSettings = "select t1.stir_route as site_stir_route,t2.stir_enable,t2.stir_route pbx_stir_route, t1.siteid from siteinfo_new t1 left join (SELECT siteid,stir_enable,stir_route from  pbxusers where extensions=%d) t2 on t1.siteid=t2.siteid where REPLACE(REPLACE(t1.sitename,'.','-'),'_','-') = '%s'";

	char *cAAQuery = "SELECT 1 from special where extendedgroup = 1 and extensions = %d and siteid = %d UNION select 1 from uc_apps where extension = %d and siteid = %d";

	memset(cQuery,0,sizeof(cQuery));

	if(cSiteName == NULL || strlen(cSiteName) == 0) {
		LOG(L_ERR,"[CheckSpamSettings] Input parameters are empty \n");
		return -1;	
	}

	snprintf(cQuery, sizeof(cQuery)-1, cSelectSpamSettings, nExtension, cSiteName);	
	ws_dbf.raw_query(h, cQuery, &res);
	if(res==NULL){
		LOG(L_ERR,"[CheckSpamSettings] no result set with query:%s\n",cQuery);
		return -1;
	}
	if(res && RES_ROW_N(res) > 0 ){
		row = RES_ROWS(res)[0];
		if(!VAL_NULL(&row.values[0])){
			nStirRoute = get_db_int(row.values[0]); 
		}
		if(!VAL_NULL(&row.values[1])){
			nStirEnable = get_db_int(row.values[1]); 
			*nStir_Enable = get_db_int(row.values[1]); 
		}
		if(nStirEnable == 1 && !VAL_NULL(&row.values[2])) { /* Apply user spam settings*/
			nStirRoute = get_db_int(row.values[2]); 
		}
		if(!VAL_NULL(&row.values[3])) {
			nSiteId = get_db_int(row.values[3]);
		}
	}
	if (res) {
		ws_dbf.free_result (h,res);
		res = NULL;
	}

	if(nStirRoute != STIR_ROUTE_WITH_SPAM) { /* Route 'AA' calls without apply spam settings */
		memset(cQuery,0,sizeof(cQuery));
		snprintf(cQuery, sizeof(cQuery)-1, cAAQuery, nExtension, nSiteId, nExtension, nSiteId);	
		ws_dbf.raw_query(h, cQuery, &res1);
		if(res1==NULL){
			LOG(L_ERR,"[CheckSpamSettings] no result set with query:%s\n",cQuery);
			return -1;
		}
		if(RES_ROW_N(res1) > 0 ) { /* If result is not '0' then it is a 'AA/streamlet' ext. so route the call without following spam settings */
			nStirRoute = STIR_ROUTE_WITH_SPAM;
		}
		ws_dbf.free_result (h,res1);
		res1 = NULL;
	}	
	LOG(L_ERR,"[CheckSpamSettings] query :%s   nStirRoute :'%d' nStirEnable :'%d'\n", cQuery, nStirRoute, nStirEnable);

	return nStirRoute;
}

int modify_x_parking_info_header(struct sip_msg *msg, char *cAgentId) {

	char cQuery[1024] = "", cSiteName[128] = "", cBuffer[256] = "", X_Parking_Info[256] = "", cBuffer2[256] = "", ctype[128] = "";
	char *ptr = NULL, *ptr1 = NULL;

	db_res_t *res=NULL;
	db_row_t row;

	char *cUserDetailsQuery = "SELECT firstname,lastname FROM pbxusers WHERE sitename = '%s' AND agentid = '%s'" ;

	memset(cQuery,0,sizeof(cQuery));

	if(msg == NULL || cAgentId == NULL) {
		LOG(L_ERR,"[modify_x_parking_info_header] Input parameters are empty \n");
		return -1;	
	}

	if((ptr = strchr(cAgentId,'@')) && ++ptr) {
		strncpy(cSiteName, ptr, sizeof(cSiteName)-1);
	}

	snprintf(cQuery, sizeof(cQuery)-1, cUserDetailsQuery, cSiteName, cAgentId);	
	
	ws_dbf.raw_query(h, cQuery, &res);
	
	if(res==NULL){
		LOG(L_ERR,"[modify_x_parking_info_header] no result set with query:%s\n",cQuery);
		return -1;
	}
	
	if(res && RES_ROW_N(res) > 0 ){
		row = RES_ROWS(res)[0];
		if(!VAL_NULL(&row.values[0]) && !VAL_NULL(&row.values[1])) {
			snprintf(cBuffer , sizeof(cBuffer)-1, "X-Parking-Info: Data;CNAM=%s %s",row.values[0].val.string_val,row.values[1].val.string_val);
			if(is_header_exists(msg,"X-Parking-Info",14, cBuffer2,sizeof(cBuffer2))){

				if((ptr = strstr(cBuffer2, ";ctype"))) {
					strncpy(ctype, ptr, sizeof(ctype)-1);
				}

				if((ptr = strstr(cBuffer2,";CallerID="))) {
					if((ptr1 = strstr(cBuffer2,";P-Cnam="))) {
					       *ptr1 = '\0';
						snprintf(X_Parking_Info, sizeof(X_Parking_Info)-1,"%s%s;P-Cnam=%s %s%s\r\n", cBuffer, ptr, row.values[0].val.string_val,row.values[1].val.string_val,ctype);
					}else{
						snprintf(X_Parking_Info, sizeof(X_Parking_Info)-1,"%s%s\r\n", cBuffer, ptr);
					}
				}
				str cmd1={"X-Parking-Info",14};
				if (remove_hf && remove_hf(msg, (char *)&cmd1,0) !=1) {
					LOG(L_ERR,"[modify_x_parking_info_header] X-Parking-Info remove_hf failure\n");
				}
				add_header(msg, X_Parking_Info, strlen(X_Parking_Info));
			}
		}
	}else{
		LOG(L_ERR,"[modify_x_parking_info_header] no result set with query:%s\n",cQuery);
	}


	if (res) {
		ws_dbf.free_result (h,res);
		res = NULL;
	}

	return 0;
}

int create_branches ( struct sip_msg * msg , db_res_t * res  , int  flag, struct ws_uri * wsuri ){
	int ret,
	nCalledDisaster=0,
	ni = 0,
	db_phonetype = 0,
	db_nsec = 5,
	nSec = 5;
	int j ;
	short i,count,
	invalid_phone_no=0;
	char db_phoneno[256],
	loc_phoneno[256],
	buffer[32];
	urecord_t * rec;
	ucontact_t  * contacts;
	db_row_t row;
	char agentid[256] = "",*moc_agentid = NULL, cAgentId[128]= "";
	int nrows = 0;
	char grp_title[256] = "";
	int fwdflag = 0, nDcidHeader = 0;
	char db_phone[256] = "";		
	char cNewUri[256];		
	struct ws_uri wsuri1 = {};
	int nDcid = 0, nReturn = 1, nRouteType = 0, nStirEnable=0;
	char cLocationIP[32] = "", cRequestUri[256]="", cTransferred_Agentd[256] = "",*cPtr = NULL, cDid[32] = "", header_value[64] = "";

	if (res == NULL) {
		LOG (L_ERR, "no result set?\n");
		return -1;
	}
	nrows = RES_ROW_N (res);
	if (nrows < 1) {
		/* We are freeing result set in the calling method */
		LOG (L_ERR, "no rows are avaialable?\n");
		return -1;
	}
	msg->nRejectRelay = 0;
	memset(msg->cRejectReason, 0, sizeof(msg->cRejectReason));
	/*Added by Tulsi for Virtual Seat implementation*/
	/*Fixed MSICP crash by adding condition for number of branches(MAX_BRANCHES) --kkmurthy 2009-10-01*/
	memset(phonelist,0,sizeof(phonelist));
	for (i=0,count=0; (i < nrows) && (count < MAX_BRANCHES); i++) {
		memset(db_phoneno, 0x00, sizeof(db_phoneno));
		memset(cAgentId, 0, sizeof(cAgentId));
		row = RES_ROWS (res)[i];
		if (row.values[0].nul == FALSE) {
			strncpy (db_phoneno, row.values[0].val.string_val, sizeof(db_phoneno));
			if(strlen(db_phoneno) == 0 || !strcmp(db_phoneno,"0")) {
				continue;
			}
			/*Added by Tulsi for Virtual Seat implementation*/
			if(isInPhoneList(db_phoneno,nrows,count)){
				continue;
			}
			count++;
		} else {
			continue;
		}
		db_phonetype = get_db_int(row.values[1]);
		if(flag == 1) {
			db_nsec = get_db_int(row.values[2]);
			/* If ring sec is less than or zero , make it to 20 sec default --vravi */
			if(db_nsec <= 0){
				LOG(L_WARN, "[create_branches] SETTING NSEC TO 20 as db nsec is less than zero  \n");
				db_nsec = 20;
			}
		} else {
				db_nsec = 20;
		}

		if( (!strncmp(wsuri->type, "icom",4)) ){
			LOG(L_WARN, "[create_branches] for intercom call sequence time = 2\n");
			db_nsec = 2;
		}

		LOG (L_WARN, "Phone %s  %d\n", db_phoneno, db_phonetype);
		contacts = NULL;
		rec = NULL;
		nReturn = 1;
		nRouteType = 0;
		switch (db_phonetype) {
			/* This is SIP: Lookup for credibility */
			/* Worksmart SIP Phones */
			case 1:
			case 2:
			case 3:
				if(RES_ROWS(res)[i].values[5].val.string_val){	
					fwdflag = get_db_int(RES_ROWS(res)[i].values[5]);
				}
				if(RES_ROWS(res)[i].values[3].val.string_val){
					strncpy(grp_title, RES_ROWS(res)[i].values[3].val.string_val, sizeof(grp_title)-1);
				}	
				LOG(L_ERR, "Global group name: '%s' group_title: '%s' Forward Flag: '%d' \n",group_name,grp_title,fwdflag);	
				
				if(fwdflag == 1 && !strchr(grp_title, '@')){
					snprintf(db_phone,sizeof(db_phone)-1,"%s_%s",db_phoneno,grp_title);
				}else if(fwdflag == 1 && strlen(group_name)>0){
					snprintf(db_phone,sizeof(db_phone)-1,"%s_%s",db_phoneno,group_name);	
				}else{
					strncpy(db_phone, db_phoneno, sizeof(db_phone)-1);
				}	

				ret=get_uri_contact(msg,db_phone, &ni,NULL, &nDcidHeader);
				if (nSec < db_nsec) {
					// If morethan 1 phones in the same priority set the maximum sec. among the secs.
					nSec = db_nsec ;
				}
				break;
			case 4: //Third-party SIP Phone
				if (wsuri == NULL) {
					LOG(L_WARN, "[create_branches] Case 4: wsuri is NULL\n");
					break;
				}
				if( (!strncmp(wsuri->type, "icom", 4)) ){
					LOG(L_WARN, "[create_branches] Case 4: Intercom request breaking\n");
					break;
				}
				moc_agentid = NULL;
				if(RES_ROWS(res)[i].values[4].val.string_val){
					strncpy(agentid, RES_ROWS(res)[i].values[4].val.string_val, sizeof(agentid)-1);
					char *ptr = strchr(agentid,'@');
					if(ptr)
						*ptr = '\0';
					moc_agentid = agentid;
				}
				if(RES_ROWS(res)[i].values[5].val.string_val){
					fwdflag = get_db_int(RES_ROWS(res)[i].values[5]);	
				}


				if(nEnableCosTransfer) {
					memset(buffer, 0x00 , sizeof(buffer));
					memset(cDid, 0x00 , sizeof(cDid));
					
					if(strncmp(db_phoneno,"sip:",4) == 0) {
						strncpy(buffer, db_phoneno+4, sizeof(buffer)-1);
					}else if(db_phoneno[0] == '+'){
						strncpy(buffer, db_phoneno+1, sizeof(buffer)-1);
					}else{
						strncpy(buffer, db_phoneno, sizeof(buffer)-1);
					}

					if(isdigit(buffer[0])) {

						cPtr = strchr(buffer,'@');
						if(cPtr) {
							*cPtr = '\0';
							cPtr = NULL;
						}

						if(strlen(buffer) == 10) {
							snprintf(cDid, sizeof(cDid)-1,"%s%s",national_prefix,buffer);
						}else{
							snprintf(cDid, sizeof(cDid)-1,"%s",buffer);
						}

						if(strlen(cDid) > 9 ) {	
							/* Added if condition to check class of service in disater cases -- jun-30-2020 */
							memset(cTransferred_Agentd, 0, sizeof(cTransferred_Agentd));
							snprintf(cTransferred_Agentd, sizeof(cTransferred_Agentd)-1,"%s-%s",moc_agentid,wsuri->context);
						
							nReturn = set_rpid_from_user(msg, cDid, cTransferred_Agentd, wsuri->token);
							if ( nReturn  == 0 || nReturn == -1) {
								send_reply(msg, 302, "Moved temporarily");
  								_nNoRelay = 1;
								return 1;
							}
						}
					}
				}

				msg->nRoutingPlanExtFlag = 3; /* Added for "[PA1-T138] Hunt Group reports -- sep-15-2020*/

				if ( strncmp(db_phoneno,"sip:",4) ) {
					/*Mobile call changes,considering agentname for PSTN calls --kkmurthy 28th December 2008*/
					process_thirdparty_phones(msg , db_phoneno, wsuri, &ni,moc_agentid,fwdflag,sizeof(db_phoneno),NULL,NULL);
				}else{
					process_thirdparty_phones(msg , &db_phoneno[4],wsuri, &ni,moc_agentid,fwdflag,sizeof(db_phoneno)-4,NULL,NULL);
				}	
				if (nSec < db_nsec) {
					// If morethan 1 phones in the same priority set the maximum sec. among the secs.
					nSec = db_nsec;
				}
				break;
			case 6:
				nCalledDisaster = 1;
			case 5:
				///* If the analog/cell phone has 10 digits, make that as 11 Date:22/11/05 --Ramu */
				if (wsuri == NULL) {
					LOG(L_WARN, "[create_branches] Case 5: wsuri is NULL\n");
					break;
				}
				if(nMaxExtLen == 0 || nMaxExtLen <= MINEXTLEN) {
					LOG(L_ERR , "[create_branches] @@@ pls check msicp.conf...nMaxExtLen is either 0 or less than or equals to MINEXTLEN @@@ \n");
				}
				/* Allow extention dialling in PSTN phone field from user routing plan, added by Prashant/Swaroopa */
				if((strlen(db_phoneno) >= MINEXTLEN) && (strlen(db_phoneno) <= nMaxExtLen)) {

					if(IsSpamCall(msg) == 1) {
						nRouteType = CheckSpamSettings(msg, atoi(db_phoneno), wsuri->context,&nStirEnable);
						if((nRouteType == STIR_REJECT || nRouteType == STIR_REJECT_WITH_PROMPT) ) {
							LOG(L_ERR, "[create_branches] Spam settings are not allowed so not creating this branch\n");
							break;
						}
					}
					memset(&wsuri1, 0x00, sizeof(struct ws_uri));
					memset(loc_phoneno,0,sizeof(loc_phoneno));
					memset(cNewUri,0,sizeof(cNewUri));
					if(wsuri && wsuri->type && wsuri->group && wsuri->context && wsuri->token) {
						wsuri1.command = db_phoneno;
						wsuri1.type    = wsuri->type;
						wsuri1.group   = wsuri->group;
						wsuri1.context = wsuri->context;
						wsuri1.token   = wsuri->token;

                        moc_agentid = NULL;
                        if(RES_ROWS(res)[i].values[4].val.string_val){
                            strncpy(agentid, RES_ROWS(res)[i].values[4].val.string_val, sizeof(agentid)-1);
                            char *ptr = strchr(agentid,'@');
                            if(ptr)
                                *ptr = '-';
                            do {
                                ptr = strchr(agentid, '.');
                                if(ptr != NULL) {
                                    *ptr = '-';
                                }
                            } while(ptr != NULL);

                            moc_agentid = agentid;
                        }

                        if(RES_ROWS(res)[i].values[3].val.string_val){
                            strncpy(grp_title, RES_ROWS(res)[i].values[3].val.string_val, sizeof(grp_title)-1);
                        }
					
						/* re-setting to default*/
						nDcid = 0;
						memset(cLocationIP, 0, sizeof(cLocationIP));
						if((frame_uri(msg, &wsuri1, cNewUri, sizeof(cNewUri), moc_agentid, grp_title, &nDcid)) == 0) {	
							if(strlen(cNewUri) > 0) {
								if(enable_dc_transfer && nRouteAcdAndConfCallsToHomeDc && nDcid && nDcid != datacenter_id && (!strncmp(cNewUri,"sip:acd",7) || !strncmp(cNewUri,"sip:cnf",7))) {
									LOG(L_ERR , "[create_branches]  User Set Across DC ACD/CNF EXT in Routingplan. So routing Call to Home DC \n");

									memset(cRequestUri,0,sizeof(cRequestUri));
									snprintf(cRequestUri, sizeof(cRequestUri)-1,"WSRequestURI: %s\r\n",cNewUri+4); /* Avoid sip: */
									add_header(msg,cRequestUri,strlen(cRequestUri));

									GetRemoteDataCenterIP(nOtherDataCenterId,cLocationIP,sizeof(cLocationIP));
								}
								if(strlen(cLocationIP)>0) {
									snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:%s@%s", wsuri1.command, cLocationIP);
								}else{
									if( WSState->nMediaServerType == PRIMARY){
										snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "%s@%s", cNewUri, umslb_proxy);
									}else{
										snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "%s@%s", cNewUri, umslb_proxy1);
									}	
								}	
								LOG(L_ERR , "[create_branches] Framed Uri = %s \n",loc_phoneno);
								goto APPENDURI;
							}
						}
						break;
					}
				} else if(strlen(db_phoneno) > nMaxExtLen){
			/*Added else condition to identify routing plan case or not while adding 'LocalCancel' header in MSICP generated 'CANCEL' request after ring timeout in 'tm' module to fix HUNT group report showing abandoned calls (Routing plan to HUNT Group/user DID/ext) -- Prashant/Abhilash *Sept-2020*
				 * Now 'nRoutingPlanExtFlag' value used for 
				 * nRoutingPlanExtFlag == 1 //Routing plan to extension
				 * nRoutingPlanExtFlag == 2 //Routing plan to DID
				 * nRoutingPlanExtFlag == 3 //Routing plan to sip ipPhone
				 * */		
					msg->nRoutingPlanExtFlag = 2;
				}	
				
				/* Checking for Intercom if it is intercom breaking */
				if( (!strncmp(wsuri->type, "icom",4)) ){
					LOG(L_WARN, "[create_branches] Case 5: Intercom request breaking\n");
					break;
				}

				if (! strlen (db_phoneno)) break;
				for (j=0; db_phoneno[j]; j++ ) {
					if (!isdigit (db_phoneno[j])) {
						invalid_phone_no=1;
						break;
					}
				}
				if (invalid_phone_no)
					break;
				

				memset(buffer, 0 , sizeof(buffer));
				normalize(db_phoneno, buffer, sizeof(buffer),sizeof(db_phoneno));
				LOG(L_WARN,"[ create_branches ] Anlog/Cell Phone = %s int pref:%s len:%d ",buffer,international_prefix,prefix_length);
				/*Mobile call changes,considering agentname for PSTN calls ---kkmurthy 21st December 2008*/
				moc_agentid = NULL;
				if(RES_ROWS(res)[i].values[4].val.string_val){
					strncpy(agentid, RES_ROWS(res)[i].values[4].val.string_val, sizeof(agentid)-1);
					char *ptr = strchr(agentid,'@');
					if(ptr)
						*ptr = '\0';
					moc_agentid = agentid;
				}		
				
				if(moc_agentid == NULL)
					moc_agentid = wsuri->group;
				/*End --kkmurthy*/
//				if(WSState->nMediaServerType == PRIMARY)
				 if(RES_ROWS(res)[i].values[5].val.string_val){
					 fwdflag = get_db_int(RES_ROWS(res)[i].values[5]);
				 }

				if( WSState->nMediaServerType == PRIMARY){
					memset(loc_phoneno,0,sizeof(loc_phoneno));
					if(fwdflag == 1){
						snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:oms-%s-%s-%s-%s_%d@%s", buffer, moc_agentid,wsuri->context,wsuri->token,fwdflag,umslb_proxy);
						if(RES_ROWS(res)[i].values[4].val.string_val) {
							strncpy(cAgentId, RES_ROWS(res)[i].values[4].val.string_val, sizeof(cAgentId)-1);
							modify_x_parking_info_header(msg, cAgentId);
						}
					}else{
						snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:oms-%s-%s-%s-%s@%s", buffer, moc_agentid,wsuri->context,wsuri->token,umslb_proxy);
					}
					LOG(L_WARN,"[create_branches]PRIMARY>>>>>>>>>>>>:%s\n",loc_phoneno);
					if(!is_header_exists(msg,"X-Noreplookup",13,header_value,sizeof(header_value))){
						memset(header_value,0,sizeof(header_value));
						snprintf(header_value, sizeof(header_value)-1,"X-Noreplookup: Yes\r\n");
						add_header(msg,header_value,strlen(header_value));
					}
				}else{
					memset(loc_phoneno,0,sizeof(loc_phoneno));
					if(fwdflag == 1){
						snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:oms-%s-%s-%s-%s_%d@%s", buffer, moc_agentid,wsuri->context,wsuri->token,fwdflag,umslb_proxy1);
						if(RES_ROWS(res)[i].values[4].val.string_val) {
							strncpy(cAgentId, RES_ROWS(res)[i].values[4].val.string_val, sizeof(cAgentId)-1);
							modify_x_parking_info_header(msg, cAgentId);
						}
					}else{
						snprintf (loc_phoneno, sizeof(loc_phoneno)-1, "sip:oms-%s-%s-%s-%s@%s", buffer, moc_agentid,wsuri->context,wsuri->token,umslb_proxy1);
					}
					LOG(L_WARN,"[create_branches]SECONDARY>>>>>>>>>>>>:%s\n",loc_phoneno);
				}

				//if(nEnableCosTransfer &&  db_phonetype == 6 && wsuri && strncmp(wsuri->token,"wspib",5)==0 ) {	
				if(nEnableCosTransfer) {	
						/* Added if condition to check class of service in disater cases -- jun-30-2020 */
						memset(cTransferred_Agentd, 0, sizeof(cTransferred_Agentd));
						snprintf(cTransferred_Agentd, sizeof(cTransferred_Agentd)-1,"%s-%s",moc_agentid,wsuri->context);
						nReturn = set_rpid_from_user(msg, buffer, cTransferred_Agentd, wsuri->token);
						if ( nReturn  == 0 || nReturn == -1) {
							send_reply(msg, 302, "Moved temporarily");
  							_nNoRelay = 1;
							return 1;
						}
				}
#if 0							
				if ( !is_permited(wsuri->command, wsuri->context, buffer) ) {
					LOG(L_ERR,"[create_branches] COS doesnt allow this phone %s\n", buffer);
					continue;
				}
#endif						
// 				strcat(phoneno, ";user=phone");
APPENDURI:
				ret=0;
				
   				if ( ni == 0 ) {
					ret = set_uri2(msg, loc_phoneno);
				} else {
					ret = append_branch (msg, loc_phoneno, strlen (loc_phoneno), 0, 0, 0);
				}
				ni++;
#ifdef SMARTDB
				LOG (L_WARN, "[create_branches]Append Brnch : %d\n",ret);
#endif
				if (nSec < db_nsec) {
					 // If morethan 1 phones in the same priority set the maximum sec. among the secs.
					 nSec = db_nsec ;
				 }
					
				break;
				/* This is PSTN */
			default:
				 /* Added to check if the pstn phone number is valid. The purpose is to avoid
				  * * long rings when an invalid number is set for the PSTN phone: 07-June-2005
				  * */
				 if (! strlen (db_phoneno)) break;
				 for (i=0; db_phoneno[i]; i++ ) {
					 if (!isdigit (db_phoneno[i])) {
						 invalid_phone_no=1;
						 break;
					 }
				 }
				 if (invalid_phone_no) break;
				 /* Till here: 07-June-2005 */
				 /* If the analog/cell phone has 10 digits, make that as 11 Date:22/11/05 --Ramu*/
				 bzero(buffer, sizeof(buffer));
				 normalize(db_phoneno, buffer, sizeof(buffer),sizeof(db_phoneno));
				 if (wsuri == NULL) {
					LOG(L_WARN, "[create_branches] default: wsuri is NULL\n");
				 	break;
				 }
				 memset(loc_phoneno,0,sizeof(loc_phoneno));
				 snprintf(loc_phoneno,sizeof(loc_phoneno)-1,"sip:oms-%s-%s-%s-%s@%s",buffer,wsuri->group,wsuri->context,wsuri->token,umslb_proxy);
				 ret=0;
				 if ( ni == 0 ) {
					 ret = set_uri2(msg, loc_phoneno);
				 } else {
					 ret = append_branch (msg, loc_phoneno, strlen (loc_phoneno), 0, 0, 0);
				 }
				 ni++;
#ifdef SMARTDB
				 LOG (L_WARN, "[create_branches]Append Brnch : %d\n",ret);
#endif
				 if (nSec < db_nsec) {
					 // If morethan 1 phones in the same priority set the maximum sec. among the secs.
					 nSec = db_nsec ;
				 }
					

				 break;
		}   /* End switch (... */
	}   /* End for (... */
	if (flag == 1 && ni ) {
		/* Set the no. of secs to ring for ext dialing*/
		ws_set_iattr(msg, nSec);
	}
#ifdef SMARTDBG
	LOG (L_WARN, "[create_branches] Freeing all phones: %d\n", (ni != 0) ? 1 : -1);
#endif
	if ((( _nOrigSeq != 1) && (_nOrigSeq != 8) && (ni > 0))) {
			LOG (L_WARN, "[create_branches] Dummy Branch Added\n");
			append_branch(msg, NULL, 0, 0, 0, 0);
	}
	if(nCalledDisaster && (ni > 0) && (_nSupSeq == 0))
	{ /*last && added by tulsi for fixing the media server pooling routing at the time of warm transfer to DID to DID*/
			return 2;
	}
	LOG(L_WARN,"ni:%d \n",ni);
	return (ni != 0) ? 1 : -1;
}

int ws_set_transfer_uri (struct sip_msg *msg, char *s1, char *s2)
{
    int nExtension=0;
    struct hdr_field *hf;
    struct sip_uri uri;
    struct to_body * fromb;
    char temp[64], buf[256], user[128],*tmp,*tmp1,*context,*replaces;
    char query[512], newheader[512];

    db_res_t *res=NULL;
    db_row_t row;
    char *format_string =" "
            "SELECT  extensions "
            "  FROM agent a, subscriber s"
            " WHERE a.agentid = s.agentid "
            "   AND s.username = '%s'";

    if(!msg || !msg->from){
	LOG(L_ERR , "[ws_set_transfer_uri] From header field missing !!!!  \n");
	return -1;
    }

    /* Get from body first */
    if (! msg->from->parsed ) parse_from_header(msg);
    fromb = get_from (msg);
    if (fromb == NULL) {
        send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
        return -1;
    }
    parse_uri(fromb->uri.s, fromb->uri.len, &uri);
    bzero(temp,sizeof(temp));
	snprintf(temp, sizeof(temp)-1, "%.*s", uri.user.len, uri.user.s);


#ifdef  SMARTDBG
    LOG (L_WARN, "temp: %s\n", temp);
#endif
    if (strncmp (temp, "ext-", 4)&&strncmp (temp,"cmd-",4)) {
		/* Check request uri too, if it has xfer then process else return */
		parse_uri(msg->first_line.u.request.uri.s, msg->first_line.u.request.uri.len, &uri);
    	bzero(temp,sizeof(temp));
		snprintf(temp, sizeof(temp)-1, "%.*s", uri.user.len, uri.user.s);
   		if (strstr(temp,"xfer") == NULL) {
			return 1;
		}
	}
    parse_headers(msg, HDR_EOH, 0);
    /* Get Refer-To header and User-Agent header */
    /*! We saw this  behaviour from some of the other useragent also
     * * * so irrespective of the User-Agent we are trying to change the Refer-To */
    for (hf=msg->headers; hf; hf=hf->next) {
        if (hf->name.len!=8)continue;
        if (strncasecmp (hf->name.s, "Refer-To", 8)!=0) continue;
        bzero (buf, sizeof (buf));
		snprintf(buf, sizeof(buf)-1, "%.*s", hf->body.len, hf->body.s);
        break ;
    }

    LOG(L_WARN, "Refer-To: %s\n",buf);
    bzero (newheader, sizeof (newheader));
    tmp=strstr(buf,"sip:");
    context=strstr(temp,"-");
    if(context) {
        context++;
        context=strstr(context,"-");
        if(context){
            context++;
        }
        else {
            return -1;
        }
    }
    else{
        return -1;
    }

    if(tmp) {
        tmp+=4;
        tmp1=strchr(tmp,'@');
        if(tmp1) {
            *tmp1='\0';
            tmp1++;
            replaces=strstr(tmp1,"Replaces=");
            LOG(L_WARN, "Replace: %s\n",replaces);
            bzero(user,sizeof(user));
            strncpy(user,tmp,sizeof(user)-1);
            if(isdigit(user[0])) {
                if (replaces) {
					if(strrchr(replaces,'>')) {	
	                    snprintf (newheader, sizeof (newheader)-1, "Refer-To: <sip:%s-%s@%s?%s\r\n", user,context,umslb_proxy,replaces);
					}
					else {
						snprintf (newheader, sizeof (newheader)-1, "Refer-To: <sip:%s-%s@%s?%s>\r\n", user,context,umslb_proxy,replaces);
					}
                }
                else {
                    snprintf (newheader, sizeof (newheader)-1, "Refer-To: <sip:%s-%s@%s>\r\n", user,context,umslb_proxy);
                }
            }
            else {
				memset(query,0,sizeof(query));
                snprintf(query,sizeof(query)-1,format_string,user);
                LOG(L_WARN,"User : %s and Query = %s\n",user,query);
		ws_dbf.raw_query (h, query, &res);
		if(res==NULL){
			LOG(L_WARN,"[ws_set_transfer_uri]'res' is NULL");
			return -1;
		}
                if (RES_ROW_N(res) > 0) {
                    row = RES_ROWS(res)[0];
                    nExtension = get_db_int(row.values[0]);
                    if (res)
                        ws_dbf.free_result (h,res);
                }
                else {
                    if (res)
                        ws_dbf.free_result (h,res);
                    return -1;
                }
                if (replaces) {
					if(strrchr(replaces,'>')) {	
	                    snprintf (newheader,sizeof(newheader)-1, "Refer-To: <sip:%d-%s@%s?%s\r\n", nExtension,context,umslb_proxy,replaces);
					}
					else {
						snprintf (newheader,sizeof(newheader)-1, "Refer-To: <sip:%d-%s@%s?%s>\r\n", nExtension,context,umslb_proxy,replaces);
					}
                }
                else {
                    snprintf (newheader,sizeof(newheader)-1, "Refer-To: <sip:%d-%s@%s>\r\n", nExtension,context,umslb_proxy);
                }
            }
#ifdef  SMARTDBG
    LOG (L_ERR, "newheader: %s\n", newheader);
#endif
        str cmd1={"Refer-To",8};
        if (remove_hf) {
            if(remove_hf(msg, (char *)&cmd1,0)==1)
                LOG(L_WARN,"remove_hf success\n");
            else
                LOG(L_ERR,"remove_hf failure\n");
            }
            str cmd2={newheader,strlen(newheader)};
            if (append_hf) {
                if(append_hf(msg, (char *)&cmd2,0)==1)
                    LOG(L_WARN,"append_hf success\n");
                else
                    LOG(L_ERR,"append_hf failure\n");
            }
            return 1;
        }
    }
    return -1;
}

// For Forwarding the Calls to another Extension
int ws_set_contact_uri (struct sip_msg *msg, char *s1, char *s2) {
	str uri;
	struct sip_uri puri;
	char to_body[256], con_body[256], newheader[512],*ext,*con;
	char extension[16],contact[128];
	/* Get to body first */
	if(!msg || !msg->to){
		LOG(L_ERR , "[ws_set_contact_uri] To header field missing !!!!  \n");
		return -1;
	}
	uri = get_to(msg)->uri;
	parse_uri(uri.s, uri.len, &puri);
	bzero(to_body,sizeof(to_body));
	snprintf(to_body, sizeof(to_body)-1, "%.*s", puri.user.len, puri.user.s);
	LOG (L_WARN,"to body = %s\n",to_body);
	if (strncmp (to_body, "ext-", 4)&&strncmp (to_body,"cmd-",4)) return 1;
	LOG (L_WARN,"I got uri with ext- = %s\n",to_body);
	parse_headers (msg, HDR_CONTACT, 0);
	bzero(con_body,sizeof(con_body));
	snprintf(con_body, sizeof(con_body)-1, "%.*s", msg->contact->body.len, msg->contact->body.s);
	LOG (L_WARN,"I got contact with ext = %s\n",con_body);
	con = strchr(con_body,'@');
	if(!con)
		return -1;
	memset(contact,0,sizeof(contact));
	strncpy(contact,con,sizeof(contact)-1);   // Server IP
	ext = strstr(con_body,"<sip:");
	if(!ext)
		return -1;
	ext+=5;
	bzero(extension,sizeof(extension));
	con = strchr(ext,'@');
	if(!con)
		return -1;
	*con = '\0';
	strncpy(extension,ext,sizeof(extension)-1);
	LOG (L_WARN," Contact Extension %s and Server %s\n",extension,contact);
	ext = strchr(to_body,'-');
	if(!ext)
		return -1;
	ext++;
	con = strchr(ext,'-');
	if(!con)
		return -1;
    	con++;
	bzero(con_body,sizeof(con_body));
	strncpy(con_body,con,sizeof(con_body)-1);   //  Context
	bzero (newheader, sizeof (newheader));
	snprintf (newheader, sizeof (newheader)-1, "Contact: <sip:%s-%s%s\r\n",extension,con_body,contact);
	LOG (L_WARN," New Contact Header  = %s\n",newheader);
	str cmd1={"Contact",7};
	if (remove_hf) {
		if(remove_hf(msg, (char *)&cmd1,0)==1)
		    LOG(L_WARN,"remove_hf success\n");
	    else
		    LOG(L_ERR,"remove_hf failure\n");
	}
	str cmd2={newheader,strlen(newheader)};
	if (append_hf) {
		if(append_hf(msg, (char *)&cmd2,0)==1)
		    LOG(L_WARN,"append_hf success\n");
	    else
	   	    LOG(L_ERR,"append_hf failure\n");
	}
	return 1;
}

int ws_process_subscribe (struct sip_msg *msg, char *phone, char *p2) {

	struct sip_uri sipuri;
	struct hdr_field *hf;
	char *extn;
	char *cont;
	char command[256];
	char uri[256], buf[32],user[128];
	int nExpireValue = 0, nValue = 0;
	struct to_body * fromb;
	struct sip_uri ws_uri;

  	bzero(command,sizeof(command));
 	nValue = atoi(phone);
	if (nValue == 1) {
		/* This is from CISCO phone consider REGISTER as SUBSCRIBE */
		if(!msg || !msg->from){
			LOG(L_ERR , "[ws_process_subscribe] From header field missing !!!!  \n");
			return -1;
		}
		if (! msg->from->parsed ) parse_from_header(msg);
		fromb = get_from (msg);
		if (fromb == NULL) {
			send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
			return -1;
		}
		parse_uri(fromb->uri.s, fromb->uri.len, &ws_uri);
		bzero(user,sizeof(user));
		snprintf(user, sizeof(user)-1, "%.*s", ws_uri.user.len, ws_uri.user.s);
		LOG (L_WARN,"User Name of CISCO Phone: %s\n",user);
		memset(uri,0,sizeof(uri));
		strncpy(uri,user,sizeof(uri)-1);
		nValue = get_direct_extension(user, 1);	
		if (nValue == -1) {
			// Extension not found for this user
			return -1;
		}
		cont=strchr(uri,'-');
		if(!cont)
			return -1;
		*cont='\0';
		cont++;
/*      Don't Subcribe from Here, at the time of user creation only we are subscribing user info. with system 		*/
		snprintf(command,sizeof(command)-1,"sh %s %d %s %s",mwi_subscribe, nValue, cont, user);
		LOG(L_WARN,"[ws_process_subscribe] command=%s\n\n",command);
		system(command);
#ifdef MWI
		snprintf(command,sizeof(command)-1,"sh %s %s %d",mwi_notify, cont, nValue);
		LOG(L_WARN,"[ws_process_subscribe] command=%s\n\n",command);
		system(command);
#endif
		return 1;
	}
//	Usual Behaviour for all Phones	
	bzero(uri,sizeof(uri));
	parse_uri(msg->first_line.u.request.uri.s, msg->first_line.u.request.uri.len, &sipuri);
        memcpy(uri, sipuri.user.s, sipuri.user.len);
	LOG(L_WARN,"[ws_process_subscribe] SUBSCRIBE RCVD URI = %s\n",uri);
	extn=uri;
	cont=strchr(uri,'-');
	if ( cont ) {
		*cont='\0';
		cont++;
	}
	else {
		LOG(L_ERR,"parse uri failed\n");
		return -1;
	}
	/* people may trying to subscribe like ramu-worksmart-com instead of <exten-worksmart-com>.
	 * Simply retun -1. Dont SUBSCIBE to MWI --Ramu Date:15/12/05
	 */
	if (!isdigit(extn[0])) {
		LOG(L_ERR,"Not a Valid MWI URI\n");
		return -1;
	}
	/* If Expire Header contains "0" Dont SUBSCRIBE with MWI but send NOTIFY */
	for (hf=msg->headers; hf; hf=hf->next) {
		parse_headers(msg, HDR_EOH, 0);
		if ( hf->name.len != 7 )continue;
		if ( strncasecmp (hf->name.s, "Expires", 7) ==0 ) {
			bzero (buf, sizeof (buf));
			snprintf(buf, sizeof(buf)-1, "%.*s", hf->body.len, hf->body.s);
			nExpireValue = atoi(buf);
			break;
		}
	}
#ifdef MWI
	if (nExpireValue == 0) {
		LOG(L_WARN,"[ws_process_subscribe] Expire value is zero, only send NOTIFY\n");
		memset(command,0,sizeof(command));
  		snprintf(command,sizeof(command)-1,"sh %s %s %s",mwi_notify, cont, extn);
		LOG(L_WARN,"[ws_process_subscribe] command=%s\n\n",command);
		system(command);
		return 1;
	}
#endif
	if(!msg || !msg->from){
		LOG(L_ERR , "[ws_process_subscribe] From header field missing !!!!  \n");
		return -1;
	}
	if (! msg->from->parsed ) parse_from_header(msg);
	fromb = get_from (msg);
	if (fromb == NULL) {
		send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		return -1;
	}
	parse_uri(fromb->uri.s, fromb->uri.len, &ws_uri);
	bzero(user,sizeof(user));
	snprintf(user, sizeof(user)-1, "%.*s", ws_uri.user.len, ws_uri.user.s);
	LOG (L_WARN,"User Name: %s\n",user);
/* Don't Subcribe from Here, at the time of user creation only we are subscribing user info. with system */	
	memset(command,0,sizeof(command));
	snprintf(command,sizeof(command)-1,"sh %s %s %s %s",mwi_subscribe, extn, cont, user);
	LOG(L_WARN,"[ws_process_subscribe] command=%s\n\n",command);
	system(command);
#ifdef MWI
	memset(command,0,sizeof(command));
	snprintf(command,sizeof(command)-1,"sh %s %s %s",mwi_notify, cont, extn);
	LOG(L_WARN,"[ws_process_subscribe] command=%s\n\n",command);
	system(command);
#endif
	return 1;
}

void strreplace(char* in, char** out, char* old, char* new) {
char* temp = NULL;
char* found = strstr(in, old);

if(!found) {
*out = malloc(strlen(in) + 1);
strncpy(*out, in,strlen(in));
return;
}

int index = found - in;

*out = realloc(*out, strlen(in) - strlen(old) + strlen(new) + 1);
strncpy(*out, in, index);
strcpy(*out + index, new);
strcpy(*out + index + strlen(new), in + index + strlen(old));

temp = malloc(index+strlen(new)+1);
strncpy(temp,*out,index+strlen(new));
temp[index + strlen(new)] = '\0';

strreplace(found + strlen(old), out, old, new);
temp = realloc(temp, strlen(temp) + strlen(*out) + 1);
strcat(temp,*out);

free(*out);
*out = temp;
}

/* Global function for NOTIFY processing at MSICP */

int ws_process_notify (struct sip_msg *msg, char *phone, char *p2){
	
	struct sip_uri sipuri;
	char uri[1024]="",actual_uri[512]="";
	int notify_type = 0,ret_flag = 0;
	struct ws_uri wsuri;

	if(!msg ){
		LOG(L_ERR,"[ws_process_notify] Msg is NULL\n");
		return 0;
	}	
	memset(uri, 0 , sizeof(uri));

	memset(&wsuri,0,sizeof(wsuri));
	
	parse_uri(msg->first_line.u.request.uri.s, msg->first_line.u.request.uri.len, &sipuri);
	memcpy(uri, sipuri.user.s, sipuri.user.len);

	if (parse_ws_uri_buffer(uri, strlen(uri), &wsuri) < 0) {
		return send_reply(msg,404, "Agent Not Found: Hint: Not a valid WSURI");
	}
	
	if(wsuri.type && strstr(wsuri.type,"wsenh")){
		notify_type = atoi(wsuri.command);
			switch(notify_type){
				case NOTIFY_REBOOT:
						memset(actual_uri,0x00,sizeof(actual_uri));
						snprintf(actual_uri,sizeof(actual_uri)-1,"%s-%s",wsuri.group,wsuri.context);
						ret_flag = ws_process_reboot_notify(msg,actual_uri,wsuri.token); /*Remote reboot Notify processing */
					break;
				case NOTIFY_MWI:
						memset(actual_uri,0x00,sizeof(actual_uri));
						snprintf(actual_uri,sizeof(actual_uri)-1,"%s@%s",wsuri.group,wsuri.context);
						ret_flag = ws_process_mwi_notify(msg,actual_uri,wsuri.token);/*Processing MWI Notify*/
						break;
				default:
					LOG(L_ERR,"[ws_process_notify] Unknown type rececived not processing\n");
					ret_flag = 404;
					break;
			}	
	}else {
		LOG (L_ERR,"[ws_process_notify] No wsenh  type in the Ruri of notify\n");
		ret_flag =  404;
	}
	if(ret_flag == 404){
		return send_reply(msg,404, "Agent Not Found");	
	}else if(ret_flag == 500){	
		return send_reply(msg,500, "Internal Server Error");
	}	
	return 1;
}

static int ws_process_mwi_notify (struct sip_msg *msg, char *user, char *p2)
{
	char uri[1024]="",query[512]="",cMacid[32]="",cUsername[128]="";
	char cSoftphone[128]="",cIpphone1[128]="",cIpphone2[128]="",cAgentid[128]="";
	db_res_t *res=NULL,*res1=NULL;
	db_row_t row;
	int nRow = 0, nRet = 0,nCount = 0, nDcid = 0, nIs_SubscriptionExists = 0;
	char *ptr=NULL; 
	
	memset(query, 0 , sizeof(query));
	snprintf(query, sizeof(query)-1,MSG_NOTIFY_QUERY, user);
	if(ws_dbf.raw_query (h, query, &res)){
		LOG(L_ERR,"[ws_process_mwi_notify] Query execution got failed:%s\n",query);
		return 404;
	}
	if (!res || RES_ROW_N(res) <= 0 ) {
		LOG(L_ERR,"[ws_process_mwi_notify] Rows Not Found:%s\n",query);
		if ( res ) {
			ws_dbf.free_result(h ,res);	
		}
		return 404;
	}

	if (RES_ROW_N(res) > 0) {
			for ( nRow = 0; nRow < RES_ROW_N(res); nRow++ ) {
				nIs_SubscriptionExists = 0;
				row = RES_ROWS (res)[nRow];
				if (! res || ! row.values)  {
					break;
				}
			
				memset(cUsername, 0 , sizeof(cUsername));
				if(!(VAL_NULL(&row.values[0])) && strlen(row.values[0].val.string_val) > 0){
					strncpy(cUsername,row.values[0].val.string_val,sizeof(cUsername)-1);
				}
				if(!(VAL_NULL(&row.values[3]))) {
					nDcid = get_db_int(row.values[3]);
				}
				memset(cMacid, 0 , sizeof(cMacid));
				if(!(VAL_NULL(&row.values[4])) && strlen(row.values[4].val.string_val) > 0){
					strncpy(cMacid,row.values[4].val.string_val,sizeof(cMacid)-1);
				}
				memset(cAgentid,0,sizeof(cAgentid));
				snprintf(cAgentid,sizeof(cAgentid)-1,"%s",user);
				while((ptr=strchr(cAgentid,'.')))
				{
					*ptr='-';
				}
				ptr=NULL;
				if((ptr=strchr(cAgentid,'@'))) {
					*ptr='\0';
					ptr++;
					if(ptr) {
						memset(cSoftphone,0,sizeof(cSoftphone));
						memset(cIpphone1,0,sizeof(cIpphone1));
						memset(cIpphone2,0,sizeof(cIpphone2));
						snprintf(cSoftphone,sizeof(cSoftphone)-1,"%s-%s",cAgentid,ptr);
						snprintf(cIpphone1,sizeof(cIpphone1)-1,"%s1-%s",cAgentid,ptr);
						snprintf(cIpphone2,sizeof(cIpphone2)-1,"%s2-%s",cAgentid,ptr);
					}
				}
				if(mwi_subscribe_flag && strlen(cMacid) > 0 && strlen(cSoftphone) > 0 && strlen(cIpphone1) > 0 && strlen(cIpphone2) > 0) {
					/* Checking the entry in subscription table with username and macid */
					memset(query, 0 , sizeof(query));
					snprintf(query, sizeof(query)-1,MWI_SUBSCRIBE_QUERY, cMacid,cSoftphone,cIpphone1,cIpphone2);
					if(ws_dbf.raw_query (h, query, &res1)){
						LOG(L_ERR,"[ws_process_mwi_notify] Query1 execution got failed:%s\n",query);
						return 404;
					}
					/* If we get result from subscription, we are not sending MWI notify to user on particular Macaddress*/
					if (RES_ROW_N(res1) > 0) {
						LOG(L_ERR,"[ws_process_mwi_notify] Got values from subscription table for mac: %s\n",cMacid);
						nIs_SubscriptionExists = 1;
					}
					if ( res1 ) {
						ws_dbf.free_result(h ,res1);	
						res1 = NULL;
					}
				}

				if(!nIs_SubscriptionExists){
					memset(uri, 0 , sizeof(uri));
					if(datacenter_id && nDcid && nDcid != datacenter_id && !(VAL_NULL(&row.values[0])) && strlen(row.values[0].val.string_val) > 0 &&
						!(VAL_NULL(&row.values[2])) && strlen(row.values[2].val.string_val) > 0) {
							snprintf(uri ,  sizeof(uri)-1, "sip:%s@%s",row.values[0].val.string_val,row.values[2].val.string_val);
					}
					else if(!(VAL_NULL(&row.values[0])) && strlen(row.values[0].val.string_val) > 0 && !(VAL_NULL(&row.values[1])) &&
						strlen(row.values[1].val.string_val) > 0) {
							snprintf(uri ,  sizeof(uri)-1, "sip:%s@%s",row.values[0].val.string_val,row.values[1].val.string_val);
					}
					
					if(strlen(uri) > 0) {
						if ( nCount == 0 ){
							nRet = set_uri2(msg, uri);
							nCount++;
						}else{
							nRet = append_branch (msg, uri, strlen (uri), 0, 0, 0);
						}
					}
				}
			}
	}

	ws_dbf.free_result (h,res);
	if ( nRet && t_relay && t_relay(msg, NULL, NULL) < 0){
		LOG(L_ERR,"[ws_process_mwi_notify] t_relay is failed\n");
		return 500;
	}
	return 1;
}

/* Added for hoteling feature remote reboot NOTIFY processing --vravi */
static int ws_process_reboot_notify (struct sip_msg *msg, char *user, char *mac)
{
	char uri[512]="",query[512]="",actual_contact[256]="" ,route_header[256]="";
	db_res_t *res;
	db_row_t row;
	int  nRet = 0,no_row_values = 0, nDcid=0;

	if( !msg || !user || !mac){
		LOG(L_ERR,"[ws_process_reboot_notify] Either msg or phone is NULL \n");
	}

	snprintf(query, sizeof(query)-1,QUERYSELREGCONTACT, user,mac);
	if(ws_dbf.raw_query (h, query, &res)){
		LOG(L_ERR,"[ws_process_reboot_notify] Query execution got failed:%s\n",query);
		return 404;
	}
	if (!res || RES_ROW_N(res) <= 0 ) {
		LOG(L_ERR,"[ws_process_reboot_notify] Rows Not Found:%s\n",query);
		if ( res ) {
			ws_dbf.free_result(h ,res);	
		}
		return 404;
	}

	row = RES_ROWS (res)[0];

	if(!(VAL_NULL(&row.values[3]))) {
		nDcid = get_db_int(row.values[3]);
	}
	
	memset(uri, 0 , sizeof(uri));
	if(datacenter_id && nDcid && nDcid != datacenter_id && !(VAL_NULL(&row.values[0])) && strlen(row.values[0].val.string_val) > 0 && 
			!(VAL_NULL(&row.values[2])) && strlen(row.values[2].val.string_val) > 0) {
		snprintf(uri ,  sizeof(uri)-1, "sip:%s@%s",user,row.values[2].val.string_val);
		strncpy(actual_contact,row.values[2].val.string_val,sizeof(actual_contact)-1);
	}
	else if(!(VAL_NULL(&row.values[0])) && strlen(row.values[0].val.string_val) > 0 && !(VAL_NULL(&row.values[1])) && strlen(row.values[1].val.string_val) > 0)
	{
		snprintf(uri ,  sizeof(uri)-1, "sip:%s@%s", user,row.values[0].val.string_val);
		strncpy(actual_contact,row.values[1].val.string_val,sizeof(actual_contact)-1);
	}else{
		no_row_values = 1;
	}
	
	if(res){
		ws_dbf.free_result (h,res);
	}

	if(no_row_values){
		return 404;
	}

	if(strlen(uri) > 0) {
		nRet = set_uri2(msg, uri);
	}

	/* Added route header with exact contact */
	memset(route_header,0,sizeof(route_header));
	snprintf(route_header, sizeof(route_header)-1,"Route: <%s>\r\n",actual_contact);
	
	str cmd1={route_header,strlen(route_header)};
			
	if(append_hf(msg, (char *)&cmd1,0) !=1)
		LOG(L_ERR,"[ws_process_reboot_notify] append_hf failure\n");
	
	if ( nRet && t_relay && t_relay(msg, NULL, NULL)< 0){
		LOG(L_ERR,"[ws_process_reboot_notify] t_relay is failed\n");
		return 500;
	}
	return 1;
}

void get_wsagent_id(const char *inbuffer,char *outbuffer,int outbuf_len, int *value)
{
	char *ptr = NULL, *cPtr = NULL;
    int len=0;
    char val[4] = "";
    if(!inbuffer){
	    LOG(L_ERR,"source buffer is NULL\n");
	    return;
	}
    strncpy(outbuffer,inbuffer,outbuf_len-1);
    if((ptr = strchr(outbuffer,'-'))){
	    if(isdigit(outbuffer[ptr-outbuffer-1])){
			memset(val, 0x0, sizeof(val));
			strncpy(val, ptr - 1, sizeof(val) - 1);
			if((cPtr = strchr(val,'-'))){
				*cPtr = '\0';
			}
	        *value = atoi(val);
	        len = strlen(ptr) + 1;
	        while(len){
	             outbuffer[ptr-outbuffer-1] = *ptr;
	             ptr++;
	             len--;
	        }
	    }else {
			*value = 0;
		}
	    ptr = strchr(outbuffer,'-');
	    *ptr = '@';
		ptr++;
		while((ptr = strchr(ptr,'-'))){
	       *ptr = '.';
	      	ptr++;
	    }
	}
}
int get_direct_extension (char *username, int nValue)
{
	int exten=-1, nRet = 0;
	char cUserName[512] = "";
	static char query [512];
	static char *format_string =  ""
							"SELECT  extensions"
							"  FROM pbxusers"
							" WHERE agentid = '%s'";

		db_res_t *res=NULL;
		db_row_t row;
		bzero(cUserName, sizeof(cUserName));
		get_wsagent_id(username, cUserName, sizeof(cUserName)-1, &nRet);
		bzero (query, sizeof (query));
		// For Click and Dial -- Ramu
		if(strlen(cUserName) > 0){
			snprintf (query, sizeof(query)-1, format_string, cUserName);
		}else{
			LOG (L_ERR, "[get_direct_extension]: User name is NULL \n");
			return -1;	
		}
		ws_dbf.raw_query (h, query, &res);
		if(res==NULL){
			LOG(L_ERR,"[get_direct_extension]'res' is NULL");
			return -1;
		}else 	if ( RES_ROW_N(res) < 1 ) {
			ws_dbf.free_result (h, res);
			return -1;
		}
		row = RES_ROWS (res)[0];
		if (! res || ! row.values) {
			if (res)
				ws_dbf.free_result(h, res);
			return -1;
		}
		exten = row.values[0].val.int_val;
		if (res)
			ws_dbf.free_result(h, res);

#ifdef SMARTDBG
		LOG (L_WARN, "Got extension: %d\n", exten);
#endif

		return exten;
}

int worksmart_route_to_operator2 (struct sip_msg *msg, int nSiteId,int nGroupId, char *sitename,char *token)
{
	char uribuf [256]="";
	int nExtension = 0;

	bzero (uribuf, sizeof (uribuf));
	nExtension = get_operator_extension(nGroupId, nSiteId );
	if(nExtension > 0){	
		snprintf (uribuf, sizeof (uribuf)-1, "opr-%d-%d-%s-%s", nExtension,nGroupId, sitename, token);
	}else{
		snprintf (uribuf, sizeof (uribuf)-1, "opr-01-%d-%s-%s", nGroupId, sitename, token);
	}

#ifdef  SMARTDBG
	LOG (L_WARN, "[worksmart_route_to_operator2]: %s\n", uribuf);
#endif

	/* This is a Operator Call set the timer 250 sec.
	 * * because wsproxy may generate CANCEL after 20 sec.
	 * * Ramu, Date 05/01/05
	 * */
	ws_set_iattr(msg, 250);
	
	return route_to_media_server (msg, uribuf, "");
}

int worksmart_route_to_operator (struct sip_msg *msg, struct PstnPhone p)
{
	char time_s [20], uribuf [100];
	unsigned uniq_value = 0;
	
	bzero (time_s, sizeof (time_s));
	uniq_value = generate_unique_token();
	snprintf (time_s, sizeof (time_s)-1, "%010u", uniq_value);
	
	bzero (uribuf, sizeof (uribuf));
	snprintf (uribuf, sizeof (uribuf)-1, "opr-0-%d-%s-%s", p.nAssignedID, p.sitename, time_s);

#ifdef  SMARTDBG
	LOG (L_WARN, "[worksmart_route_to_operator]: %s\n", uribuf);
#endif
	
	/* This is a Operator Call set the timer 250 sec.
	 * because wsproxy may generate CANCEL after 20 sec.
	 * Ramu, Date 05/01/05
	 */
	ws_set_iattr(msg, 250);	
	
	return route_to_media_server (msg, uribuf,"");
}

// wrapper function for actual set_iattr  21/12/05 --Ramu
int ws_set_iattr(struct sip_msg *msg, int nSec ) {
	str cmd1={"callee_fr_inv_timer",19};
	if (set_iattr) {
		if(set_iattr(msg, (char *)&cmd1,(char *)nSec)==1)
			LOG(L_WARN,"[ws_set_iattr]set_iattr success and nSec: %d\n",nSec);
		else
			LOG(L_WARN,"[ws_set_iattr]set_iattr failure and nSec: %d\n",nSec);
	}		 
	return 1;
}

int process_acd_agent_call2 (struct sip_msg *msg, char *priority, struct ws_uri *wsuri) {
  char * formatstring = ""
          "SELECT STRAIGHT_JOIN phone, phonetype, ringsecs, p.agentid, p.agentid, p.wscallfwdcname "
        "  FROM pbxusers p, dialplantypes d, weeklyplans w, phones ph, siteinfo_new s "
	" WHERE ph.pbxid = p.id "
        "   AND p.id = d.pbxid  "
        "   AND d.id = w.dialplanid "
		" 	AND w.pbxid = p.id "
        "   AND s.siteid = p.siteid  "
        "   AND weekid = ( SELECT dayofweek(convert_tz(now(), '%s', tdiff)) "
        "       FROM pbxusers pu "
        "      WHERE pu.id = p.id ) "
        "   AND w.dialplanid = ph.dialplanid "
        "   AND extensions = '%s' "
        "   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
        "   AND ph.seq = %d "
        "   AND ph.phonetype < 4 "
        "   AND w.pbxid NOT IN ( "
        "        SELECT dm.pbxid "
        "          FROM dialplanmasks dm "
        "         WHERE CURRENT_TIMESTAMP between dm.localfrom AND dm.localto "
        "   ) "
          "   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
           "OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
        "  OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
        "UNION "
        "SELECT STRAIGHT_JOIN phone, phonetype, ringsecs ,p.agentid, p.agentid,p.wscallfwdcname "
        "  FROM pbxusers p, dialplantypes d, dialplanmasks dm, phones ph, siteinfo_new s  "
	" WHERE ph.pbxid = p.id "
        "   AND p.id = d.pbxid "
        "   AND d.id = dm.dialplanid "
        "   AND dm.dialplanid = ph.dialplanid "
        "   AND extensions = '%s' "
        "   AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s' "
          "   AND ( (localfromtime > localtotime AND ( time(now()) BETWEEN localfromtime AND time('23:59:59') "
         "OR time(now()) BETWEEN '00:00:00' AND localtotime)) "
        "  OR ( time(now()) BETWEEN ph.localfromtime AND ph.localtotime ) ) "
//      "   AND date(now()) BETWEEN date(dm.localfrom) AND dm.localto "
		"   AND CURRENT_TIMESTAMP BETWEEN dm.localfrom AND dm.localto "
        "   AND s.siteid = p.siteid  "
        "   AND ph.seq = %d "
        "   AND ph.phonetype < 4 ";
	db_res_t *res=NULL;
	int nRes = -1;
	int nSeq = atoi(priority);
	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group ) {
		return -1;
	}
	clear_branches();
	for (; nSeq <= 4; nSeq++ ) {
		char query[2048];
		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1, formatstring, servertz, wsuri->command, wsuri->context, nSeq, wsuri->command, wsuri->context, nSeq);
		//LOG(L_ERR, "[process_acd_agent_call2] %s\n", query);
		ws_dbf.raw_query (h, query, &res);
		if(res==NULL){
			LOG(L_WARN,"[process_acd_agent_call2]'res' is NULL");
			return -1;
		}
		if (RES_ROW_N(res) > 0) {
			nRes = create_branches(msg, res, 1, wsuri);
			if (res) {
				ws_dbf.free_result (h,res);
			}
			if ( nRes < 0) {
				continue;
			}
			if ( t_on_failure ) {
				t_on_failure (msg, (char *)(nSeq + 1), NULL);
				LOG(L_ERR, "On failure : %d\n", nSeq + 1 );
			}

			if (_nOrigSeq == 1) {
				/* Set the Remote-Party-ID only for the first priority -Ramu 04/04/06*/
				ws_set_rpid(msg, registrar_domain ,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
			}
	      break;
    	}
		if (res) {
			ws_dbf.free_result (h,res);
		}
  }
  if ( nSeq == 5  ) {
  	if ( atoi(priority) == 1 ) 
		send_reply(msg, 480, "Agent not available");
  	_nNoRelay = 1;
  }
  	
  return 1;
}
int worksmart_route_to_ivr2 (struct sip_msg *msg,int nFlag) {

	struct ws_uri wsuri;
	struct sip_uri parsed_uri;

	char query[2048];
	char result[128];
	db_res_t *res=NULL;
	db_row_t row;
	char * formatstring = ""
		"SELECT STRAIGHT_JOIN concat(concat(concat(concat(concat(concat(concat('ivr-', '%s'), '-'), greetingid), '-'),'%s'), '-'), '%s') "
        "  FROM siteinfo_new s, groupgreetings g  "
        " WHERE s.siteid = g.siteid "
        "   AND REPLACE(s.sitename,'.','-') = '%s'  "
        "   AND g.specialid = '%s'  "
        "   AND g.greetingid <> 0 "
        "   AND ( (localfrom > localto AND ( time(now()) BETWEEN localfrom AND time('23:59:59')      "
        "    OR time(now()) BETWEEN '00:00:00' AND localto))       "
        "    OR ( time(now()) BETWEEN localfrom AND localto ) )     ";
  if (-1 == parse_headers (msg, HDR_TO, 0)) {
	   LOG (L_ERR, "[ws_set_greeting] Error parsing from header...\n");
	   return 0;
	}
	if(!msg || !msg->to){
		LOG(L_ERR , "[worksmart_route_to_ivr2] To header field missing !!!!  \n");
		return 0;
	}
	parse_uri (get_to(msg)->uri.s, get_to(msg)->uri.len, &parsed_uri) ;
	if ( !parse_ws_uri_buffer(parsed_uri.user.s, parsed_uri.user.len, &wsuri) ) {
		LOG (L_WARN,"[worksmart_route_to_ivr2] Group:%s & Context :%s\n",wsuri.group,wsuri.context);				
	}
	else 
		return -1;

	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, formatstring, wsuri.group, wsuri.context, wsuri.token, wsuri.context, wsuri.group );

	LOG(L_ERR, "[worksmart_route_to_ivr2]Query: \n%s\n", query);
	ws_dbf.raw_query (h, query, &res);
	if(res==NULL){
		LOG(L_WARN,"[worksmart_route_to_ivr2]'res' is NULL");
		return -1;
	}

	if (RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		bzero (result, sizeof (result));	
		strncpy(result,row.values[0].val.string_val,sizeof(result)-1); 
		//Check if the call is coming from IVR system.
		// If it is from media server, we should send 302 otherwise, send a regular INVITE.
		LOG(L_WARN, "[worksmart_route_to_ivr2] %s\n", result);
		if (res)
			ws_dbf.free_result (h, res);
		return route_to_media_server(msg, result,"");
//		return route_to_server(msg, result, media_server);
	}
	else {
		// greeting not found for a group. send to voicemail
		if (res)
			ws_dbf.free_result (h, res);
		if(nFlag){
			return 0;
		}
		int nCode;
		formatstring =  ""
				"SELECT STRAIGHT_JOIN s.code "
				"  FROM special s, extendedpbxivrcommand e, siteinfo_new si "
				" WHERE REPLACE(REPLACE(si.sitename,'.','-'),'_','-') = '%s' "
				"   AND si.siteid = s.siteid "
				"   AND s.siteid = e.siteid " 	
				"   AND e.extensiongroupcode = s.code "
				"   AND e.groupcode = '%s'   "
				"   AND e.command = 0 "
				"UNION "
				"SELECT STRAIGHT_JOIN a.extensions "
				"  FROM special s, agent a, siteinfo_new si "
				" WHERE REPLACE(REPLACE(si.sitename,'.','-'),'_','-') = '%s' "
				"   AND si.siteid=s.siteid "
				"   AND s.siteid=a.siteid "
				"   AND a.agentid=s.operator "
				"   AND s.operatortype = 2 "
				"	AND s.code = '%s' ";
		memset(query,0,sizeof(query));
		snprintf(query, sizeof(query)-1, formatstring, wsuri.context, wsuri.group, wsuri.context,wsuri.group );
        LOG (L_ERR, "[worksmart_route_to_ivr2]query: \n%s\n", query);
        ws_dbf.raw_query(h, query, &res);
	if(res==NULL){
		LOG(L_ERR,"[worksmart_route_to_ivr2]'res' is NULL\n");
		return -1;
	}
        if ( RES_ROW_N(res) == 0 ) {
            // Play the invalid Extension  26/12/05 Ramu
			memset(query,0,sizeof(query));
            snprintf (query, sizeof (query)-1, "vml-00-00-%s-%s", wsuri.context, wsuri.token);
            route_to_media_server(msg, query,"");
//          send_reply(msg, 404, "Not found");
        } else {
			int nRespCode = get_resp_code(msg);
			nCode = get_db_int(RES_ROWS(res)[0].values[0]);
			memset(query,0,sizeof(query));
            snprintf (query, sizeof (query)-1, "vml-%d-%d-%s-%s", nCode, nRespCode, wsuri.context, wsuri.token);
            route_to_media_server(msg, query,"");
        }
        if (res)
        ws_dbf.free_result (h, res);
        return 1;			
	}
	return 1;
}
int get_resp_code(struct sip_msg * msg) {
	int nRespCode = 404;
	if ( _nOrigSeq == 1 ) {
		// Unavailable
		nRespCode = 404;
	} else {
		regex_t* re;
		if ((re=pkg_malloc(sizeof(regex_t)))!=0) {
			regcomp(re, "486|480|603|600", REG_EXTENDED|REG_ICASE|REG_NEWLINE);
			if ( t_check_status(msg, (char *)re, NULL) > 0) {
				nRespCode = 486;
				// Busy
			} else {
				// Unavailable
				nRespCode = 404;
			}
			pkg_free(re);
		}
	}
	return nRespCode;
}

int ws_set_group_name (struct sip_msg *msg , char  * groupname) {
		char gname[200];
		bzero(gname, sizeof(gname));

		if(groupname) {
				/*Added this as a new  header ---*/
				str removegrp = {"X-WSGroupName", 13};
				if(remove_hf){
						remove_hf( msg , (char *)&removegrp, 0);
				}
				snprintf( gname ,sizeof(gname)-1, "X-WSGroupName: %s\r\n" , groupname);
				str groupheader={gname, strlen(gname)};
				if(append_hf){
						if(append_hf(msg, (char *)&groupheader,0)==1)
								LOG(L_ERR,"append_hf groupname success\n");
						else
								LOG(L_ERR,"append_hf groupname failure\n");
				}
		}
		return 1;
}

int use_secondary_server(struct sip_msg *msg, char *server, char *prio2) 
{
	LOG(L_WARN,"[use_secondary_server] Switching UMSLB\n" );
	WSState->nMediaServerType = (WSState->nMediaServerType == PRIMARY)?SECONDARY:PRIMARY;
	return 1;
}


int use_secondary_wsicp_server(struct sip_msg *msg, char *server, char *prio2) 
{
	LOG(L_WARN,"[use_secondary_wsicp_server] Switching WSICP\n" );
	WSicpState->nICPServerType = (WSicpState->nICPServerType == PRIMARY)?SECONDARY:PRIMARY;
	return 1;
}

int ws_process_911(struct sip_msg *msg, char *prio1, char *prio2)
{
	int status = 0;
	char phnumber[32];
	char uri_userpart[512];
	char uri_userpart_copy[512];
	char query[1024];
	char ipaddr[IP_ADDR_MAX_STR_SIZE];
    char* username = NULL;
    char* sitename = NULL;
    char* substring = NULL;
    str from_uri;
    struct sip_uri parsed_uri;

	db_res_t *res = NULL;
	db_row_t row;
	
	if (msg == NULL)
	{
		LOG (L_WARN, "[ws_process_911] some one calling 911, but where is the msg??\n");
		goto RouteError;
	}
	_nOrigSeq = atoi(prio1);
	memset(ipaddr,0,sizeof(ipaddr));
	strncpy(ipaddr, ip_addr2a(&msg->rcv.src_ip), sizeof(ipaddr)-1);
	// parse from line and get the site name and user name
	if ((parse_from_header(msg)) == -1)
	{
		LOG (L_WARN, "[ws_process_911] Failed to parse FROM header\n");
		goto RouteError;
	}
	
	from_uri = get_from(msg)->uri;
	if (parse_uri(from_uri.s, from_uri.len, &parsed_uri))
	{
		LOG (L_WARN, "[ws_process_911] Failed to parse FROM uri\n");
		goto RouteError;
	}

	memset(uri_userpart, 0, sizeof(uri_userpart));
	snprintf(uri_userpart, sizeof(uri_userpart)-1, "%.*s", parsed_uri.user.len, parsed_uri.user.s);
	memset(uri_userpart_copy, 0, sizeof(uri_userpart_copy));
	strncpy(uri_userpart_copy, uri_userpart, sizeof(uri_userpart_copy)-1);

	substring = strchr(uri_userpart, '-');
	if (substring == NULL)
	{
		LOG (L_WARN, "[ws_process_911] sitename separator not found\n");
		goto RouteError;
	}
	else
	{
		uri_userpart[substring-uri_userpart] = '\0';
		username = uri_userpart;
		sitename = substring + 1;
	}

	// check whether a profile has been setup for this IP Address and get the callerid
	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, "SELECT status, phonenumber FROM nineoolocation WHERE ipaddress='%s' AND REPLACE(REPLACE(sitename,'_','-'), '.', '-')='%s'", ipaddr, sitename);
	LOG(L_WARN, "[ws_process_911]Query: %s\n", query);
	
	// we need to get exactly one record as result
	ws_dbf.raw_query(h, query, &res);
	if (res == 0 || (RES_ROW_N(res) <= 0) || (RES_ROW_N(res) > 1))
	{
		if (res)
		{
			ws_dbf.free_result (h, res);
		}
		
		goto RouteError;
	}
	                                                                         
    // get the 911 status and callerid
	row = RES_ROWS(res)[0];
	status = row.values[0].val.int_val; // does the IP Address have 911 profile
	strncpy(phnumber, row.values[1].val.string_val, sizeof(phnumber));
	ws_dbf.free_result(h, res);

	if (status != 1)
	{
		goto RouteError;
	}
	
	// update the name of the last caller of 911 in the table from the curent IP Address
	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, "UPDATE nineoolocation SET username = (CASE ipaddress WHEN '%s' THEN '%s' ELSE NULL END) WHERE REPLACE(REPLACE(sitename,'_','-'), '.', '-')='%s' AND phonenumber='%s'", ipaddr, username, sitename, phnumber);
	LOG(L_WARN, "[ws_process_911]Query: %s\n", query);
	ws_dbf.raw_query(h, query, &res);

	goto RouteEmergencyCall;

RouteError:
	strncpy(phnumber, "0", sizeof(phnumber)-1);

RouteEmergencyCall:
	// route the call to 911 server
	// Request URI must be set to the callerid of the 911 profile used by the above
	// obtained IP Address
	ws_set_iattr(msg, 90);
	route_to_server(msg, phnumber, emergency_server,OTHER);
	return 1;
}


int process_x11_returnedcall(struct sip_msg* msg, char* phnumber)
{
	char query[512]="";
	char phoneno[512]="";
	char username[128]="";
	char *buffer=phnumber;
	char ipaddr[IP_ADDR_MAX_STR_SIZE]="";
	char cMacaddr[16]="";
	int ret=0, i=0, ni=0;

	db_res_t *res = NULL;
	db_row_t row;

	if (msg == NULL || phnumber==NULL)
	{
		// too bad... no phnumber/msg !?!?!
		return -1;
	}

	if (strlen(buffer) > 10)
		buffer = buffer + (strlen(buffer)-10);
	snprintf(query,sizeof(query)-1, "SELECT ipaddress, CONCAT(username, CONCAT('-',REPLACE(sitename ,'.','-'))), device_macid "
					"FROM nineoolocation "
					"WHERE phonenumber='1%s' "
					"AND username IS NOT NULL order by updatetime desc", buffer); //phnumber
	ws_dbf.raw_query(h, query, &res);
	if (res == 0 || (RES_ROW_N(res) <= 0))
	{
		LOG(L_ERR,"[process_x11_returnedcall] Query : %s \n",query);
		if (res)
		{
			ws_dbf.free_result (h, res);
		}
		//911 caller cannot be found (query failed!!)
		return -1;
	}
	//get the ip address, username and sitename
	row = RES_ROWS(res)[0];
	 if (!VAL_NULL(&row.values[0] ) ){
		strncpy(ipaddr, row.values[0].val.string_val,sizeof(ipaddr)-1);
	 }
	 if(!VAL_NULL(&row.values[1])){
		strncpy(username, row.values[1].val.string_val,sizeof(username)-1);
	 }
	 if(!VAL_NULL(&row.values[2])){ 
		strncpy(cMacaddr, row.values[2].val.string_val,sizeof(cMacaddr)-1);
	 }
	 ws_dbf.free_result(h, res);
	res = NULL;
	if (((strlen(cMacaddr) <=0) && (strlen(ipaddr) <=0)) || strlen(username) <=0)
	{
		LOG(L_WARN, "[process_x11_returnedcall] IP Address or User Name is NULL\n");
		//911 caller cannot be found now (!!! can't find caller)
		return -1;
	}

	//get the contact addresses of the phones whose ipaddress and username match with the results of the above query
	if( strlen(cMacaddr) > 0 )
    {
		snprintf(query,sizeof(query)-1, "SELECT DISTINCT registrar FROM location WHERE username='%s' AND contact like '%%%s%%' AND macid='%s' ", username, ipaddr, cMacaddr);
	}
	else
	{
		snprintf(query,sizeof(query)-1, "SELECT DISTINCT registrar FROM location WHERE username='%s' AND contact like '%%%s%%'", username, ipaddr);
	}
	ws_dbf.raw_query(h, query, &res);
	if (res == NULL || (RES_ROW_N(res) <= 0))
	{
		if (res)
		{
			ws_dbf.free_result (h, res);
		}
		//911 caller cannot be found (query failed!!)
		return -1;
	}
	//call all registrars for that contact
	for (i=0; i<RES_ROW_N(res); i++)
	{
		row = RES_ROWS(res)[i];
		if(strlen(cMacaddr)>0)
			snprintf(phoneno,sizeof(phoneno)-1, "sip:911_%s_%s@%s", cMacaddr ,username, row.values[0].val.string_val);
		else
			snprintf(phoneno,sizeof(phoneno)-1, "sip:%s@%s", username, row.values[0].val.string_val);

		if (i==0)
		{
			ret = set_uri2(msg, phoneno);
			if (ret < 0)
			{
				LOG(L_WARN, "[process_x11_returnedcall] Could not add branch for %s\n", phoneno);
				break;
			}
			ni++;
	}
		else
		{
			ret = append_branch(msg, phoneno, strlen(phoneno), 0, 0, 0);
			if (ret >= 0)
				LOG(L_WARN, "[process_x11_returnedcall] Appended branch : '%s' , %d\n", phoneno, ret);
			else
			{
				LOG(L_WARN, "[process_x11_returnedcall] Append branch failed: '%s', %d\n", phoneno, ret);
				break;
			}
		}
	}

	if (res)
	{
		ws_dbf.free_result (h, res);
	}
 
	if (ni <= 0)
	{
		//911 caller cannot be found now (!! strange, contacts found but can't call them)
		return -1;
	}

	ws_set_iattr(msg, 60);
	return 1;
}

int ws_systemwide_disaster(struct sip_msg * msg,char *token){
	char ruri_out[256]="";
    if(get_source(msg) == MediaServer ) {
        if( WSState->nDisasterMode == ENABLED ){
            WSState->nDisasterMode = DISABLED;
        }else {
            WSState->nDisasterMode = ENABLED;
        }
		bzero(ruri_out,sizeof(ruri_out));
		snprintf(ruri_out,sizeof(ruri_out)-1,"wsdis-%d-0-pandoranetworks-com-%s",WSState->nDisasterMode,token);
        route_to_media_server(msg,ruri_out,"");
		return 2;
    }else{
    /*Some body else trying to enable disaster*/
		LOG(L_WARN, "[enable_system_disaster] some body else is trying to enable disaster\n");
        return -1;
    }
    /*We don't want to use rtp relay server*/
}





int process_bla_call (struct sip_msg *msg, char *priority ,struct ws_uri *wsuri, int nFlag){
	int nRes = -1,nSeq = -1;
	char fullquery[3000];
	char groupname[100];
	db_res_t *res=NULL;

	char  * disasterquery = ""
			" SELECT STRAIGHT_JOIN  sp.disasterphone as phone, 6 as phonetype, 90 as ringsecs, 'Worksmart', sp.title, 0 "
			" FROM   siteinfo_new s, special sp  "
			" WHERE  REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s'  "
			" AND sp.siteid = s.siteid  "
			" AND sp.code = '%s'  "
			" AND sp.disasterflag = 1  "
			" AND sp.disasterphone IS NOT NULL";

	char * bla_formatstring = ""
			" SELECT sp.disasterphone as phone,6 as phonetype,90 as ringsecs, 'Worksmart', sp.title, 0 "
			" FROM   siteinfo_new s, special sp  "
			" WHERE  REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s'  "
			" AND sp.siteid = s.siteid  "
			" AND sp.code = '%s'  "
			" AND sp.disasterflag = 1  "
			" AND sp.disasterphone IS NOT NULL  "
			" UNION  "
			" (SELECT ph.username, 2, ph.ringsecs, sp.title , p.agentid, p.wscallfwdcname"
			" FROM  siteinfo_new s,agentspecial a,pbxusers p,special sp,pbxblausers ph,appearance_info app "
			" WHERE a.code =%s "
			" AND ph.siteid = s.siteid "
			" AND REPLACE(REPLACE(s.sitename,'.','-'),'_','-') = '%s'"
			" AND a.agentid = ph.agentid "
			" AND a.seq_num = %d "
			" AND s.siteid = a.siteid "
			" AND p.agentid = a.agentid "
			" AND sp.siteid = a.siteid "
			" AND sp.siteid = s.siteid "
			" AND sp.code = %s "
			" AND sp.code = a.code "
			" AND sp.grouptype = 3 "
			" AND sp.disasterflag = 0 "
			" AND sp.disablecallroute = 0 "
			" AND app.sharedid = ph.username "
			" AND app.appearancestate = 'FREE' "
			" ORDER BY a.seq_num "
			" limit 1)";

	if ( !wsuri->type || strcmp(wsuri->type, "bla" ) ) {
		LOG(L_ERR, "Invalid URI for process_bla_call : Level1 error");
		return -1;
	}
	if ( !wsuri->command || !wsuri->group || !wsuri->context ) {
		LOG(L_ERR, "Invalid URI for process_bla_call: Level2 error");
		return -1;
	}

	
	clear_branches();
	if(priority){
		nSeq = atoi(priority);
	}

	/* Added bla sequential routing for brguest fixes --kimo/vravi */
	LOG (L_ERR, "[blagroup::process_bla_call] MAX_BLA_LINES  : %d\n", max_bla_lines);
	if( _nSupSeq != 4){  /* _nSupSeq == 4 means Got response other than 486 for first sequence for BLA call --vravi */
		for (; nSeq <= max_bla_lines; nSeq++ ) {
			if(WSState->nDisasterMode == DISABLED ) {
					memset(fullquery,0,sizeof(fullquery));
					snprintf(fullquery, sizeof(fullquery)-1, bla_formatstring, wsuri->context,wsuri->command,wsuri->command,wsuri->context,nSeq,wsuri->command);
			}else{
					memset(fullquery,0,sizeof(fullquery));
					snprintf(fullquery, sizeof(fullquery)-1, disasterquery , wsuri->context, wsuri->command);
			}
			
			ws_dbf.raw_query (h, fullquery, &res);
			if(res==NULL){
				LOG(L_ERR,"[blagroup::process_bla_call] 'res' is NULL ... route to vml , query : %s ",fullquery);
				nRes = -1; //Need to route to vml
				break;
			}
			if (RES_ROW_N(res) <= 0) {
				LOG(L_ERR,"[blagroup::process_bla_call] result set is not found .." );
				ws_dbf.free_result (h, res);
				continue;
			}
			if(RES_ROWS (res)[0].values[3].type == DB_STRING ){
				memset(groupname, 0, sizeof(groupname));
				strncpy( groupname ,   RES_ROWS(res)[0].values[3].val.string_val, sizeof(groupname)-1);
			}
			if ((nRes = create_branches(msg, res, 1, wsuri)) < 0 ){
				LOG(L_WARN,"[blagroup::process_bla_call] nRes vale:%d ,create branches failed continuing....",nRes);
				ws_dbf.free_result (h, res);
				continue;
			}
				
			/* Set the Remote-Party-ID only for first priority -Ramu 04/04/06*/
			if (strlen(groupname)) {
				LOG(L_WARN,"[blagroup::process_bla_call] Setting Group name as : %s\n", groupname);
				ws_set_group_name(msg, groupname);	
			}
			ws_set_rpid(msg, registrar_domain,NULL,NULL,0,NULL,0,0,1, NULL);/*Caller ID Passthrogh for EXT-EXT calls -Vijay 240909*/
			if ( t_on_failure ) {
				t_on_failure (msg, (char *)(nSeq + 1), NULL);
				LOG(L_WARN, "[blagroup::process_bla_call]On failure : %d\n",nSeq + 1);
			}
			if (res){
				ws_dbf.free_result (h, res);
			}
			break;
		}
		LOG(L_WARN, "[blagroup::process_bla_call] On failure route  : %d , nRes : %d\n", nSeq + 1,nRes);
	}else {
		LOG (L_ERR, "[blagroup::process_bla_call] BLA ROUTING TO VOICEMAIL AS WE DID'NT GOT 486 \n");
	}
	if (nRes < 0) {
		//Route him to voicemail
		int nCode;
		memset(fullquery,0,sizeof(fullquery));
		snprintf(fullquery, sizeof (fullquery)-1,"SELECT s.code,s.title "
												"FROM special s, siteinfo_new si "
												"WHERE REPLACE(REPLACE(si.sitename,'.','-'),'_','-') = '%s' "
												"AND si.siteid = s.siteid "
												"AND s.code = '%s'",wsuri->context,wsuri->command);
		LOG (L_WARN, "fullquery: %s\n", fullquery);
		ws_dbf.raw_query(h, fullquery, &res);
		if(res==NULL){
			LOG(L_ERR,"[process_bla_call]'res' is NULL");
			return -1;
		}
		int nRespCode = get_resp_code(msg);
		if ( RES_ROW_N(res) == 0 ) {
			// Play the invalid Extension  26/12/05 Ramu
			memset(fullquery,0,sizeof(fullquery));
			snprintf (fullquery, sizeof (fullquery)-1, "vml-00-00-%s-%s", wsuri->context, wsuri->token);
			route_to_media_server(msg, fullquery, "");
		} else {
			nCode = get_db_int(RES_ROWS(res)[0].values[0]);
			memset(fullquery,0,sizeof(fullquery));
			snprintf (fullquery, sizeof (fullquery)-1, "vml-%d-%d-%s-%s", nCode, nRespCode, wsuri->context, wsuri->token);
			route_to_media_server(msg, fullquery, "");
		}
		if (res)
			ws_dbf.free_result (h, res);
		return 2;
	}
	return 1;
}

/*This function is added for the processing the 302 received from the phone or end user
 * Also this function will remove any dialog stuck in the acc_active table --- SANTOSH -- 05-04-2010 */
int ws_process_302(struct sip_msg *msg , char *prio1 , char *prio2)
{
	LOG(LOG_INFO,"This is the reply 302 for the INVITE we send \n");
	if (racc_302){
	if(!msg || !msg->to){
			LOG(L_ERR , "[ws_process_302] To header field missing !!!!  \n");
			return 0;
	}
		racc_302(msg , get_to(msg)->uri.s , NULL);
	} else {
		LOG(LOG_INFO,"Ops !!!!! We do not have the racc_302 exported ?????????????  \n");
	}
	return 1;
}

int ws_process_invite_replaces(struct sip_msg *msg , char *prio1)
{
		LOG(L_ERR, "[ws_process_invite_replaces] $$$$$$$$$$$$$$$$$\n\n");
		str sipuri;
		char *tmp = NULL;
		int nRes = -1;
		char uri[256] = "";
		char phone[256] = "";
		char requri[256] = "";

		char tmpuri[256]="",cCallID[128]="";

		struct ws_uri wsuri;

		_nNoRelay = 0;
		_nOrigSeq = atoi(prio1);

		if(msg != NULL){/*First line parsing*/
				sipuri = msg->first_line.u.request.uri;
				if(sipuri.s){
						snprintf(uri, sizeof(uri)-1, "%.*s", sipuri.len, sipuri.s);
						snprintf(requri, sizeof(requri)-1, "%.*s", sipuri.len, sipuri.s);
				}else{
						return -1;
				}
		}else{
				LOG(L_WARN, "[ws_process_invite_replaces] MSG is NULL!\n");
				return -1;
		}

		tmp = strchr(uri , ':');
		if(tmp){
			tmp++;
			strncpy(uri, tmp, sizeof(uri)-1);
			tmp = strchr(uri , '-');
			if(tmp){
				*tmp = '\0';
			}
		}else{
			return nRes;
		}

		LOG(L_WARN, "[ws_process_invite_replaces] URI format : %s\n", uri);

		tmp = strchr(requri , ':');
		if(tmp){
			tmp++;
			strncpy(phone,tmp,sizeof(phone)-1);
			tmp = strchr(phone , '@');
			
			if(tmp){
				*tmp = '\0';
			}else{
				return nRes;
			}

		}else{
			return nRes;
		}

		if(enable_dc_transfer){	
			memset(cCallID,0,sizeof(cCallID));
			if(is_header_exists(msg,"Replaces",8,cCallID,sizeof(cCallID))){
				nRes = GetCallLocation(msg, cCallID, phone);
			}
		}

		if(nRes == -1){
			if(isInList(msuris , uri)){/*Compare the ip address with MS URIS*/
				LOG(L_WARN, "[ws_process_invite_replaces] Request URI format points to media server, route to mslbalancer\n");

				if(WSState->nMediaServerType == PRIMARY){
						nRes = route_to_server(msg, phone, umslb_proxy,MS);
				}else{
						nRes = route_to_server(msg, phone, umslb_proxy1,MS);
				}
			}else if (isInList(omsuris , uri)){/*Compare the ip address with OMS URIS*/
				LOG(L_WARN, "[ws_process_invite_replaces] Request URI format points to acd server, route to acdserver\n");
				if(WSState->nMediaServerType == PRIMARY){
					nRes = route_to_server(msg,phone, umslb_proxy,OMS);
				}else{
					nRes = route_to_server(msg,phone, umslb_proxy1,OMS);
				}

			}else if (isInList(acduris , uri)){/*Compare the ip address with ACD URIS*/
				LOG(L_WARN, "[ws_process_invite_replaces] Request URI format points to OMS, route to omslbalancer\n");
			
				if (WSState->nMediaServerType == PRIMARY){
					nRes = route_to_server(msg, phone, umslb_proxy,ACDMS);
				}else{
					nRes = route_to_server(msg, phone, umslb_proxy1,ACDMS);
				}

			}else if(check_special_characters(uri)){
				memset(&wsuri,0x00,sizeof(wsuri));
				strncpy(tmpuri,phone,sizeof(tmpuri));
				if(parse_ws_uri_buffer(tmpuri,strlen(tmpuri),&wsuri)){
					if(WSState->nMediaServerType == PRIMARY){
							nRes = route_to_server(msg, phone, umslb_proxy,MS);
					}else{
							nRes = route_to_server(msg, phone, umslb_proxy1,MS);
					}
				}
				LOG(L_WARN, "[ws_process_invite_replaces] Request URI format points to OMS, route to omslbalancer\n");
			}
		}

		
		if ( nRes > 0 ) {

				if ( nRes == 2 ) {
						if ( ws_on_reply ) {
								ws_on_reply(msg, (char *) 2, NULL);
						} else {
								LOG(L_ERR, "**ws_on_reply is NULL\n");
						}
				}else{
						if ( ws_on_reply ) {
								ws_on_reply(msg, (char *) 1, NULL);
						} else {
								LOG(L_ERR, "ws_on_reply is NULL\n");
						}
				}

				if ( t_relay ) {
						if ( t_relay(msg, NULL, NULL) < 0 ) {
								LOG(L_ERR, "**--** Relay failed 1\n");
						}
				}
		}

		LOG(L_WARN, "[ws_process_invite_replaces] res= '%d'\n",nRes);
		return (nRes > 0 ? 1 :nRes);
}

int select_shmid (int shm_id)
{
	int i = 0,count = 0,flag=0;
	SHMIDS * list= NULL;
	i = SHM_OFFSET + sizeof(SHMIDS); /* reserving 1 unit extra to avoid sem_key overriding */
	list = (char *)shmids_ptr + i;
	while (count < noofshmsegments-2){
		if(list && (list->shmid == shmid) && (list->in_use == 1)){
			LOG(L_ERR,"[select_shmid] WARNING: SHMID: %d Node already exists at: %d, no need to insert ?? \n", list->shmid, count);
			flag = 1;
			break;
		}	
		list+=1;
		count++;
	}
	return flag;
}

int insert_shmid_shm (int shm_id)
{
	int i = 0,count = 0,semval=0;
	SHMIDS * list= NULL;
	i = SHM_OFFSET + sizeof(SHMIDS); /* reserving 1 unit extra to avoid sem_key overriding */
	list = (char *)shmids_ptr + i;
	sem_wait(sem_key);
	while (count < noofshmsegments-2){
		if(list && list->in_use == 0){
			list->shmid = shm_id;
			list->insert_time = time(0);
			list->in_use = 1;
			sem_getvalue(sem_key,&semval);
			LOG(L_ERR,"[insert_shmid_shm] Inserted SHMID: %d at:%d insert_time: %d semval: %d \n", list->shmid, count, list->insert_time, semval);
			break;
		}	
		list+=1;
		count++;
	}
	sem_post(sem_key);
	if(count >= (noofshmsegments-2)){
		LOG(L_ERR,"[insert_shmid_shm] ERROR: Unable to insert FGRP SHM Nodes, No More Segments available Here. Total Segments:%d Used:%d (out of memory)\n", noofshmsegments, count);
	}
	return 1;
}

int shmid_clear_onstartup ()
{
	int i = 0,count = 0;
	SHMIDS * list= NULL;
	i = SHM_OFFSET + sizeof(SHMIDS);
	list = (char *)shmids_ptr + i;
	while(count < noofshmsegments_prev-2)
	{
		if(list && list->in_use == 1){
			LOG(L_ERR,"[shmid_clear_onstartup] Removing SHMID: %d  at:%d insert_time: %d  \n",list->shmid, count, list->insert_time);
			shmctl(list->shmid,IPC_RMID,NULL);
			list->shmid = 0;
			list->insert_time = 0;
			list->in_use = 0;
		}
		list+=1;
		count++;
	}
	return 1;
}
int update_shmid(int shmid,int nCount)
{
	int i = 0,count = 0;
	SHMIDS * list= NULL;
	i = SHM_OFFSET + sizeof(SHMIDS);
	list = (char *)shmids_ptr + i;
	while (count < noofshmsegments-2){
		if(list && list->shmid == shmid && (list->in_use == 1)){
			list->insert_time = time(0);
			if(nCount < 1){
				LOG(L_ERR,"[update_shmid] Removing Node for SHMID: %d at: %d \n",list->shmid,nCount);
				list->in_use = -1;
				shmctl(list->shmid,IPC_RMID,NULL);
				list->shmid = 0;
				list->in_use = 0;
			}
			break;
		}
		list+=1;
		count++;
	}
	return 1;
}

static int GetCallLocation(struct sip_msg *msg, char *cCallID, char *cUri){

	char cQuery[1024]="",cLocationIP[128] = "",header_value[64]="", *ptr = NULL;
		
	db_res_t *res=NULL;

	if(!cCallID || !cUri || !msg) {
		LOG(L_ERR, "Received Empty values for any of above\n");
		return -1;
	}

	if((ptr = strchr(cCallID,';'))){
		*ptr = '\0';
	}

	memset(cQuery,0,sizeof(cQuery));
	snprintf(cQuery, sizeof(cQuery)-1 , GET_DCID_QUERY , cCallID);
	ws_dbf.raw_query (h, cQuery, &res);
	
	if(res != NULL && RES_ROW_N(res) == 0 ){
		/* Call is in other Data center */
		/* OtherDc header added to skip DB lookup in other datacenter */
		memset(header_value,0,sizeof(header_value));
		snprintf(header_value, sizeof(header_value)-1,"OtherDc: %d\r\n",nOtherDataCenterId);
		add_header(msg,header_value,strlen(header_value));
		GetRemoteDataCenterIP(nOtherDataCenterId,cLocationIP,sizeof(cLocationIP));
		if(strlen(cLocationIP) > 0) {	
			ws_dbf.free_result (h,res);
			 res = NULL;
			return route_to_server(msg, cUri, cLocationIP,MS);
		}
	}else{
		LOG(L_ERR, "[GetCallLocation] res is NULL. Query <%s> ...\n",cQuery);
	}
	if(res != NULL){
		ws_dbf.free_result (h,res);
		res = NULL;	
	}	
	return -1;
}

int GetRemoteDataCenterIP(int nDcid, char *cLocationIP, int nLen)
{
	db_res_t *res=NULL;
	db_row_t row;
	char query[512]="";
	int nRet = -1;

	if(!nDcid || !cLocationIP) {
		LOG(L_WARN, "[GetRemoteDataCenterIP] Received Empty DCID ...\n");
		return -1;
	}
	memset(cLocationIP, 0, nLen);
	memset(query,0, sizeof(query));
	snprintf(query, sizeof(query)-1 , DCEROUTEQUERY , nDcid);

	ws_dbf.raw_query (h, query, &res);

	if(res == NULL || RES_ROW_N(res) <= 0) {
		LOG(L_ERR,"[GetRemoteDataCenterIP]Result is Empty,so returning here...\n");
		if(res){
			ws_dbf.free_result (h,res);
			return -1;
		}
	}

	if (res && RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		if (!VAL_NULL(&row.values[0])) {
			strncpy(cLocationIP, row.values[0].val.string_val,nLen-1);
		}
		nRet = 0;
	}
	if (res)
		ws_dbf.free_result (h,res);

	return nRet;
}

/*Added this function to get destination of autoattendant groups for dtmf(o to 9 ,* and #)--Anusha*/
int get_group_cmd_plan ( struct sip_msg *msg, char * prio, struct ws_uri * wsuri, int nFlag) {

	char query[2048]="";
	db_res_t *res=NULL;
    db_row_t row;
	int nExtension = 0,nUsr_Flag=0,nCode=0, nRouteType = 0,nStirEnable=0;
	char buffer[128]="", cPhoneNumber[128]="", cSpamBuffer[128] = "";

	if (!msg || !wsuri || !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group ) {
		LOG(L_ERR,"[get_group_cmd_plan]any of the above is null,so return here\n");
		return -1;
	}

	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, GET_IVR_CMD_PLAN, wsuri->context,wsuri->group,wsuri->command,wsuri->context,wsuri->group,wsuri->command,wsuri->context,wsuri->group,wsuri->command,wsuri->context,wsuri->group,wsuri->command,wsuri->context,wsuri->group,wsuri->command );

	ws_dbf.raw_query(h, query, &res);

     LOG(L_WARN,"[get_group_cmd_plan] query for AA group..:%s\n",query);

	if (!res || RES_ROW_N(res) <=0) {
		if(res) {
			ws_dbf.free_result(h, res);
			res = NULL;
		}
		if(nFlag){
			memset(buffer,0,sizeof(buffer));
			snprintf (buffer, sizeof (buffer)-1, "vml-00-00-%s-%s", wsuri->context, wsuri->token);
			route_to_media_server(msg, buffer, "");
			return 1;
		}
		LOG(L_ERR,"[get_group_cmd_plan]no result set with query:%s\n",query);
		return -1;
	}
    if ( RES_ROW_N(res) > 0 ) {
        row = RES_ROWS(res)[0];
        if (!VAL_NULL(&row.values[0])) { // added for retrieve PSTN number from database
            nUsr_Flag = get_db_int(row.values[1]);
            if(nUsr_Flag == Ivr_PSTN){
                memset(cPhoneNumber,0,sizeof(cPhoneNumber));
                snprintf(cPhoneNumber, sizeof(cPhoneNumber)-1, "%s",row.values[0].val.string_val != NULL ? row.values[0].val.string_val :"");
            }else {
                nExtension = get_db_int(row.values[0]);
                nCode = get_db_int(row.values[2]);

            }
        }

    }

	if (res){
		ws_dbf.free_result(h, res);
		res = NULL;
	}
	if ( !nExtension && !nCode && nUsr_Flag != Ivr_Operator && (strlen(cPhoneNumber) <= 0)) {
		LOG(L_ERR,"[get_group_cmd_plan]Did not got extension from Query and also ivr destination is not an operator,so return here\n");
		if(nFlag){
			memset(buffer,0,sizeof(buffer));
			snprintf (buffer, sizeof (buffer)-1, "vml-00-00-%s-%s", wsuri->context, wsuri->token);
			route_to_media_server(msg, buffer, "");
			return 1;
		}
		return -1;
	}else{
		nFlag=0;
	}

	if(nUsr_Flag == Ivr_User || nUsr_Flag == Ivr_Group || nUsr_Flag == Ivr_Operator || nUsr_Flag == Ivr_Streamlet) {
		if( IsSpamCall(msg) ) {
			nRouteType = CheckSpamSettings(msg, nExtension, wsuri->context,&nStirEnable);
			if((nRouteType == STIR_REJECT || nRouteType == STIR_REJECT_WITH_PROMPT) && (prepare_contact_spam_route_fail(msg, nRouteType)==1)){
				memset(buffer, 0, sizeof(buffer));
				snprintf(buffer, sizeof(buffer)-1, "ext-%d-%s-%s-%s", nExtension, wsuri->group, wsuri->context, wsuri->token);
				snprintf(cSpamBuffer, sizeof(cSpamBuffer)-1, "SPAM#%d", nStirEnable);
            			racc_missed_call_entry(msg, buffer, cSpamBuffer);
				send_reply(msg, 302, "Moved temporarily");
  				_nNoRelay = 1;
				return 1;
			}
		}
	}

	memset(buffer,0,sizeof(buffer));
	if(nUsr_Flag == Ivr_User){/*Ivr Destination is user*/
		snprintf(buffer, sizeof(buffer)-1, "ext-%d-%s-%s-%s", nExtension, wsuri->group, wsuri->context, wsuri->token);
		parse_ws_uri_buffer(buffer, strlen(buffer), wsuri);
		return process_ext_call2(msg, prio, wsuri, nFlag,0,0);
	}else if(nUsr_Flag == Ivr_Group){/*Ivr Destination is group*/
		return nUsr_Flag;
	}else if(nUsr_Flag == Ivr_Operator){/*Ivr Destination is operator*/
        return nUsr_Flag;
    }else if(nUsr_Flag ==  Ivr_Vml){/*Ivr Destination is voicemail*/
        memset(buffer,0,sizeof(buffer));
        snprintf (buffer, sizeof (buffer)-1, "vml-%d-404-%s-%s", nExtension,wsuri->context, wsuri->token);
        route_to_media_server(msg, buffer,"");
        return 1;
    }else if(nUsr_Flag == Ivr_PSTN) { /*Ivr Destination is PSTN OB Number*/
        if( CheckPSTNorDID(cPhoneNumber)){
        	strncat(cPhoneNumber,"_1",2);               // Appending _1 to indicate cPhoneNumber is DID and _0 for PSTN 
        }else{
            strncat(cPhoneNumber,"_0",2);
        }

        LOG(L_ERR,"[get_group_cmd_plan]PSTN Number To transfer the call..:%s\n", cPhoneNumber);
        route_to_media_server(msg, cPhoneNumber,"");
        return 1;
    }else if (nUsr_Flag == Ivr_Streamlet) { /*Ivr Destination is Streamlet */
        snprintf (buffer, sizeof (buffer)-1, "wsucapp-%d-%d-%s-%s",nExtension, nCode ,wsuri->context, wsuri->token);
        route_to_media_server(msg, buffer,"");
        return 1;
    }
	return -1;
}

/*This function is added to know phone number is ws_DID or External PSTN number*/
int CheckPSTNorDID(const char *cPhoneNumber){
    
    int is_DID = 0;
	char query[1024] = "";
    db_res_t *res=NULL;

    if(!cPhoneNumber || strlen(cPhoneNumber) <= 0){
        LOG(L_ERR,"[CheckPSTNorDID]cPhoneNumber is NULL\n ");
        return 0;
    }

	memset(query, 0x00, sizeof(query));
    snprintf(query,sizeof(query)-1,ACCOUNTDIDQUERY,cPhoneNumber);
    ws_dbf.raw_query(h, query, &res);

    LOG(L_WARN,"[CheckPSTNorDID] query for DID or PSTN ..:%s\n",query);

    if ( res && (0 < RES_ROW_N(res))){
        is_DID = 1;
    }
    if(res){
        ws_dbf.free_result(h, res);
        res = NULL;
    }

    return is_DID;
}

/*This function is added to getting ivr operator ,if operator is group*/
int get_opr_grp_plan ( struct sip_msg *msg, char * prio, struct ws_uri * wsuri, int nExten,int nFlag) {

	char query[1024]="",buffer[128]="";
	db_res_t *res=NULL;
	int nExtension = 0;
	if (!msg || !wsuri || !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group ) {
		LOG(L_ERR,"[get_opr_grp_plan]any of the above is null,so return here\n");
		return -1;
	}

	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1, IVR_OPR_QUERY, nExten,wsuri->context);
	ws_dbf.raw_query(h, query, &res);

	if (!res || RES_ROW_N(res) <=0) {
		if(res) {
			ws_dbf.free_result(h, res);
			res = NULL;
		}
		LOG(L_ERR,"[get_opr_grp__plan]no result set with query:%s\n",query);
		return -1;
	}
	if ( RES_ROW_N(res) > 0 ) {
		nExtension = get_db_int(RES_ROWS(res)[0].values[0]);
	}
	if (res){
		ws_dbf.free_result(h, res);
		res = NULL;
	}
	if (!nExtension) {
		nExtension = nExten;
	}
	memset(buffer,0,sizeof(buffer));
	snprintf(buffer, sizeof(buffer)-1, "ext-%d-%s-%s-%s", nExtension, wsuri->group, wsuri->context, wsuri->token);
	parse_ws_uri_buffer(buffer, strlen(buffer), wsuri);
	return process_group_ext_call( msg , prio , wsuri , nFlag , 0, 0);
}

/* Added changes for checking class of service for making other account DID calls, if ext911 class of service is enabled then user cann't make another account DID calls --Saidulu/Selvan */
int get_isext911_plan(struct sip_msg * msg,int callee_siteid)
{
	db_res_t *res=NULL;
	db_row_t row;
	int classid=0,siteid=0,nRet=0;
	char query[512]= "", agentid[256] = "",fromuser[256]="";
	struct to_body * fromb;
	struct sip_uri uri;
	if(!msg || !msg->from){
		LOG(L_ERR," [ get_isext911_plan ] Either msg or msg->from is  NULL \n");
		return nRet;
	}

	if (! msg->from->parsed )
		parse_from_header(msg);
	fromb = get_from (msg);
	if (fromb == NULL) {
		send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		return nRet;
	}
	parse_uri(fromb->uri.s, fromb->uri.len, &uri);
	if(uri.user.s == NULL || uri.user.len == 0){
		send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
		return nRet;
	}
	memset(fromuser , 0 , sizeof(fromuser));
	snprintf(fromuser, sizeof(fromuser)-1, "%.*s", uri.user.len,uri.user.s);
	
	memset(agentid , 0 , sizeof(agentid));
	snprintf(agentid,sizeof(agentid)-1,"%s",fromuser);
	_get_agentid(msg,agentid,NULL);

	/*Added below query to check if caller belongs to extension and 911 service plan*/
	memset(query,0,sizeof(query));
	snprintf(query,sizeof(query)-1,EXT911_QUERY,agentid);

	ws_dbf.raw_query(h,query,&res);
	if( res && RES_ROW_N(res)>0) {
		row=RES_ROWS(res)[0];
		siteid=get_db_int(row.values[0]);
		classid=get_db_int(row.values[1]);
	}
	if(res){
		ws_dbf.free_result (h, res);
		res = NULL;
	}

	if((classid==-1) && siteid && callee_siteid != siteid){/* if classid is -1 in agent table means , the user is having ext911 class of service, with this service plan user can make extension , 911 and within account DID calls --Saidulu/Selvan */
		LOG(L_WARN , "Agentid : %s having Extension and 911 class of service, so he cann't make across account DID calls \n",agentid);
		nRet=1;
	}
	return nRet;
}

/* Getting Wsucapp Extension */
int get_wsucapp_id(int did,int nSiteid)
{
	char query[1024]="";
	db_res_t *res=NULL;
	db_row_t row;
	int ucapp_id = 0;

	memset(query,0,sizeof(query));
	snprintf(query, sizeof(query)-1 , UCAPPID_QUERY , did,nSiteid);
	ws_dbf.raw_query(h, query, &res);
	if(res == NULL || RES_ROW_N(res) <= 0){
		LOG(L_WARN,"[get_wsucapp_id]'res' is NULL :: Query : < %s > \n", query);
		if (res){
			ws_dbf.free_result (h,res);
			res = NULL;
		}
		return -1;
	}
	if (RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		ucapp_id = get_db_int(row.values[0]);
	}
	if (res){
		ws_dbf.free_result (h,res);
		res = NULL;
	}
	return ucapp_id;
}

/* Fun for routing INTE/PSTN (lcr-) calls to fwdout -- 08-08-2018 */
int route_to_fwdout_server(struct  sip_msg * msg,char *prio1,struct ws_uri * wsuri){
	
	char fulluri[1024]="",cBuffer[256]="", cBuffer2[64]="";
	int nSeq=0,is_cogent_fwdout=0,add_to_header = 1;
	str To_header = {"To", 2};
	str WSWEBCallInfo_header = {"WSWEBCallInfo", 13};
	str Transferred_By_header = {"Transferred_By", 14};
	char cToken[32] = "", cSiteName[128] = "";
	int nSiteId = 0;
	
	struct lump *tmp = NULL, *t = NULL;
	
	if(!msg || !prio1 || !wsuri){
		LOG(L_ERR,"[route_to_fwdout_server]Any of the above value is NULL,so Return here\n");
		return -1;
	}

	memset(fulluri, 0, sizeof(fulluri));
	if(prio1){
		nSeq=atoi(prio1);
	}

	if(wsuri->command && strlen(wsuri->command)>0){
		if(nSeq == 1 && wsuri->group && strlen(wsuri->group) >0 && atoi(wsuri->group) == 1){ /*Route to Cogent server*/
			snprintf(fulluri,sizeof(fulluri)-1,"sip:%s@%s",wsuri->command,secondary_fwdout_server);
			is_cogent_fwdout = 1;
		}else{
			snprintf(fulluri,sizeof(fulluri)-1,"sip:%s@%s",wsuri->command,((nSeq ==1)? primary_fwdout_server:secondary_fwdout_server));
		}
	}
	
	LOG(L_WARN,"[route_to_fwdout_server]  fwdout  RURI < %s >\n",fulluri);
	if(strlen(fulluri) == 0 ) {
		return -1;
	}

	if((is_cogent_fwdout == 1 || nSeq != 1)) {

		if(delete_to_header_failover == 1){
	
		/* Deleting To header form server LUMPS which fixes To header issue in case of failure carrier  21-08-2018 */ 

			tmp=msg->add_rm; 

			if(tmp) {

				for(t=tmp->next;t;t=t->next) {

					if(t->before && t->before->len > 0 && t->before->op == LUMP_ADD && t->before->type == 0) { 

						if((t->before->u.value) && (strlen(t->before->u.value)>0) && !strncmp(t->before->u.value,"To:", 3) && strstr(t->before->u.value, "5070")) { 
							LOG(L_ERR,"!!!! SETTING LUMP_DEL  PTR : %p   VALUE : %s and TYPE : %d  AND  LEN : %d !!!!!!!\n",t->before,t->before->u.value,t->before->type,t->before->len); 
							t->before->op = LUMP_DEL;
						} 
					} 
				}
			}
		}else {
			add_to_header = 0;

		}
	}else {
		/*Added changes to send to uri as sip:dialedno@fwdout instead of sending request uri in to header(if we send request uri in to header geting issue in protocols and from header..*/
		if (remove_hf) {
			if(remove_hf(msg, (char *)&To_header,0)!= 1 )
			LOG(L_ERR,"[route_to_fwdout_server] Removing 'To' header field failed \n");
		}
	}

	if( add_to_header == 1 ){ /* Add To header only when it is LCR failover and delete_to_header_failover == 0 --vravi */
		memset(cBuffer,0,sizeof(cBuffer));
		snprintf(cBuffer,sizeof(cBuffer)-1,"To: <%s>\r\n",fulluri);
		str cNew_Toheader={cBuffer,strlen(cBuffer)};
		if(append_hf){
			if(append_hf(msg, (char *)&cNew_Toheader,0)!=1){
				LOG(L_ERR, "[route_to_fwdout_server]Error while appending header!!!\n");
			}
		}
	}
	if(is_header_exists(msg,"WSWEBCallInfo",13,cBuffer2,sizeof(cBuffer2))){
		if (remove_hf) {
			if(remove_hf(msg, (char *)&WSWEBCallInfo_header,0)!= 1 )
				LOG(L_ERR,"[route_to_fwdout_server] Removing 'WSWEBCallInfo' header field failed \n");
			}
	}

	if(is_header_exists(msg,"Transferred_By",14,cBuffer2,sizeof(cBuffer2))){
		if (remove_hf) {
			if(remove_hf(msg, (char *)&Transferred_By_header,0)!= 1 )
				LOG(L_ERR,"[route_to_fwdout_server] Removing 'Transferred_By' header field failed \n");
		}
	}

	if(nSeq == 1 && !is_header_exists(msg,"X-CDR-Info",10,cBuffer2,sizeof(cBuffer2)) && wsuri->context && wsuri->token) {
		memset(cBuffer, 0, sizeof(cBuffer));
		memset(cSiteName, 0, sizeof(cSiteName));
		strncpy(cSiteName, wsuri->context, sizeof(cSiteName)-1);
		snprintf(cBuffer,sizeof(cBuffer)-1,"X-CDR-Info: %s#%d\r\n",wsuri->token, GetOnlySiteId(cSiteName));
		str X_Cdr_header={cBuffer,strlen(cBuffer)};
		if(append_hf){
			if(append_hf(msg, (char *)&X_Cdr_header,0)!=1){
				LOG(L_ERR, "[route_to_fwdout_server]Error while appending X_Cdr_header header!!!\n");
			}
		}
	}

	clear_branches();
	if (set_uri2(msg, fulluri) > 0){
		if (t_on_failure && !is_cogent_fwdout){
			t_on_failure(msg, (char *)(nSeq+29), NULL);
		}
		if (nSeq != 1){
			append_branch(msg, NULL, 0, 0, 0, 0);
		}
	}else{
		send_reply(msg, 500, "Internal Server Error, hint - 1");
	}
	return 1;
}

int GetOnlySiteId(char *cSiteName) {

	int  nSiteId = 0;
	char sqlcommand[1024] = "";
	char *ptr = NULL;
	
	db_res_t *res=NULL;

	if(cSiteName == NULL) {
		LOG(L_INFO,"[GetSiteId] cSiteName is NULL \n");
		return 0;
	}

	while((ptr = strchr(cSiteName, '-'))) {
		*ptr = '.';
	}

	snprintf(sqlcommand , sizeof(sqlcommand)-1 ,"SELECT siteid FROM siteinfo_new where sitename = '%s'" , cSiteName);

	ws_dbf.raw_query (h, sqlcommand, &res);
	if(res==NULL){
		LOG(L_ERR,"[GetOnlySiteId]'res' is NULL");
		return -1;
	}

	if (RES_ROW_N(res) > 0) {
		nSiteId = get_db_int(RES_ROWS(res)[0].values[0]);
	}

	if (res) {
		ws_dbf.free_result (h,res);
	}

	return nSiteId;
}

/* This function will frame uri based on the extension type and returns 0 if URI is framed else -1 --Prashant/Swaroopa */
int frame_uri(struct sip_msg *msg, struct ws_uri *wsuri, char *cNewuri, int nLen, char *cAgentId, char *cGrpName, int *nDcid)
{
	int     nRes            	  =  0;
	int     nFlag           	  = -1;
	int     nRet            	  = -1;
	int     nGroupcode      	  = -1;
	int     nSiteid         	  = -1;
	int     nPvalue          	  =  0;
    int     bTempExt              =  0;
    int     isURIFramed           =  0;
    int     nExtLen               =  0;
	char    cQuery[1024] 		  = "";
	char    cData[512]  		  = "";
	char cDisasterphonenumber[50] = "";
    enum GroupType gType;
	db_row_t row ;
	db_res_t *result = NULL;

	if(!wsuri || !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group || !wsuri->token || !wsuri->command) {
		LOG (L_WARN, "[frame_uri] Invalid wsuri format or phoneno in PSTN phone field??:\n");
		return -1;
	}

    bTempExt = atoi(wsuri->command);
    nExtLen = strlen(wsuri->command);

    if(nExtLen == 3 || nExtLen == 4) {
    
        switch(bTempExt) {

            case 311:
            {
                nRet = 0; 
                snprintf(cNewuri, nLen-1, "sip:dir-%s-0-%s-%s", wsuri->command, wsuri->context, wsuri->token);
                isURIFramed = 1;
            }
            break;

            case 511:
            {
                nRet = 0;
                snprintf(cNewuri, nLen-1, "sip:oms-%s-0-%s-%s", wsuri->command, wsuri->context, wsuri->token);
                isURIFramed = 1;
            }
            break;

            case 611:
            {
                nRet=frame_611_call_uri(msg, wsuri, cNewuri, nLen);
                isURIFramed = 1;
            }
            break;

            case 211:
            case 411:
            case 711:
            case 811:
            case 911:
            {
                snprintf(cNewuri, nLen-1, "sip:inv-0-0-pandoranetworks-com-%s", wsuri->token);
                isURIFramed = 1;
            }
            break;

            case 9994:
            case 9996:
            case 9997:
            case 9998:
            case 9999:
            {
                nRet = frame_feature_codes_uri(msg, atoi(wsuri->command) , wsuri->token, wsuri->context, cNewuri, nLen);
                isURIFramed = 1;
            }
            break;
        }
    }

    if(isURIFramed == 1) {
        goto LABEL;
    }

	memset(cQuery,0,sizeof(cQuery));
	memset(cData,0,sizeof(cData));
	snprintf(cQuery,sizeof(cQuery)-1,GROUP_EXT_QUERY,wsuri->command,wsuri->context,wsuri->command,wsuri->context , wsuri->command,wsuri->context);
	ws_dbf.raw_query (h, cQuery, &result);

	LOG(L_ERR,"[frame_uri] Query : '%s' \n", cQuery);

	if(result==NULL){
		LOG(L_ERR,"[frame_uri]'result' is NULL with Query : '%s' \n", cQuery);
		return -1;
	}

	if (RES_ROW_N(result) > 0) {
	
		row = RES_ROWS(result)[0];
		strncpy(cData,row.values[0].val.string_val,sizeof(cData)-1);
		nFlag = atoi(cData);
		nSiteid = row.values[1].val.int_val;
		strncpy(cData,row.values[2].val.string_val,sizeof(cData)-1);
		nGroupcode = atoi(cData);

		if (!VAL_NULL(&row.values[3])){
			nPvalue = get_db_int(row.values[3]);
		}
		if (!VAL_NULL(&row.values[4])){
			*nDcid = get_db_int(row.values[4]);
		}
	}

	if(result){
		ws_dbf.free_result(h,result);
	}

	LOG(L_ERR,"[frame_uri] groupcode '%d' flag:%d blocked_call_type:%d nSiteid : < %d >\n", nGroupcode,nFlag,nPvalue , nSiteid);

	if(nPvalue == BLOCK_IB){
		send_reply(msg, 403, "Forbidden - Callee account is interrupted");
		_nNoRelay = 1;
		return -1;
	}

	nRes = 0;
	if(nFlag == WS_USER_EXTEN) {
		nRes = check_conf_exten(msg, wsuri, nSiteid, cNewuri, nLen);
		if(nRes == -1) {
			snprintf(cNewuri, nLen-1, "sip:xfer-%s-0-%s-%s", wsuri->command, wsuri->context, wsuri->token);
		}
		nRet=0;
	}else if(nFlag == WS_GROUP_EXTEN) {

		gType = get_group_type(nGroupcode, nSiteid, cDisasterphonenumber,sizeof(cDisasterphonenumber));
		if(frame_group_uri (msg, wsuri, nGroupcode, nSiteid, gType, cDisasterphonenumber, cNewuri, nLen) == -1) {
			snprintf(cNewuri, nLen, "sip:gext-%s-0-%s-%s", wsuri->command, wsuri->context, wsuri->token);
		}
		nRet=0;
	}
	else if(nFlag == WS_STREAMAPP_EXTEN) {
		nRes = get_wsucapp_id(atoi(wsuri->command), nSiteid);
		snprintf(cNewuri , nLen-1 , "sip:wsucapp-%s-%d-%s-%s" , wsuri->command , nRes , wsuri->context , wsuri->token);
		nRet=0;
	}
	else{
		nRes = frame_associate_acc_uri (msg, "1", wsuri,cNewuri, nLen);
		if(nRes == -1){
			LOG (L_WARN, "[frame_uri] &&&&&& Invalid extension  &&&&&&&&&&&&&& \n");
			snprintf(cNewuri, nLen-1, "sip:inv-0-0-pandoranetworks-com-%s",wsuri->token);
		}
		nRet=0;
	}
LABEL:
	msg->nRoutingPlanExtFlag = 1;
    memset(msg->cAgentId, 0x0, sizeof(msg->cAgentId));
    memset(msg->cGrpName, 0x0, sizeof(msg->cGrpName));
    strncpy(msg->cAgentId, cAgentId, sizeof(msg->cAgentId) - 1);
    strncpy(msg->cGrpName, cGrpName, sizeof(msg->cGrpName) - 1);
	_nNoRelay = 0;
	return nRet;
}

/* Checking is given extension assigned to conf or not ???  if yes then framing URI --Prashant/Swaroopa*/
int check_conf_exten(struct sip_msg *msg,  struct ws_uri *wsuri, int nSiteid, char * cNewuri, int nLen)
{
	int nRet = -1;
	char cQuery[1024] = "";
	char *formatstring_conf = " SELECT concat(concat(concat(concat(concat(concat('cnf-',  c.paswd), '-'), c.confid),'-'),REPLACE(sitename,'.','-')),'-%s') "
				    		   " FROM conference  c, siteinfo_new s WHERE c.siteid = s.siteid AND c.extension = '%s' AND sitename='%s'";
							
	db_res_t *res=NULL;
	db_row_t row;

	if ( wsuri == NULL || !(wsuri->command) || !(wsuri->context)) {
		LOG(L_WARN, "[check_conf_exten] Received NULL parameters.. \n");
		return -1;
	}

	LOG(L_WARN, "[check_conf_exten] wsuri->token:[%s] wsuri->command:[%s] wsuri->context:[%s] \n", wsuri->token , wsuri->command , wsuri->context);

	memset(cQuery,0,sizeof(cQuery));
	snprintf(cQuery,sizeof(cQuery)-1, formatstring_conf, wsuri->token, wsuri->command, wsuri->context);
	ws_dbf.raw_query (h, cQuery, &res);
		
	if (RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		
		if (!VAL_NULL(&row.values[0])) {
			snprintf(cNewuri,  nLen-1, "sip:%s", row.values[0].val.string_val);
		}
		nRet = 1; 
	}
 
	if (res) {
		ws_dbf.free_result (h,res);
		res = NULL;
	}
 
	return nRet;

}

/* Checking is given extension assigned to any group ???  if yes then framing URI and relaying request  to balancer   --Prashant/Swaroopa*/
int frame_group_uri (struct sip_msg * msg,struct ws_uri *wsuri, int nGroupId, int nSiteId, int gType, char *disasterphonenumber, char *cNewuri, int nLen)
{
	char * formatstring = ""
			" SELECT STRAIGHT_JOIN DISTINCT 1, greetingid, operatortype FROM greetingmasks gm,special sp WHERE sp.siteid = %d AND gm.siteid = sp.siteid "
			" AND sp.code = gm.code AND gm.code = %d AND ( ((now()) between localfrom AND localto) OR (periodic = 1 AND mtype = 1 AND ((DATE_FORMAT(now(),'%s')"
			" BETWEEN DATE_FORMAT(localfrom,'%s') AND DATE_FORMAT(localto,'%s')) OR (date_format(now(),'%s')='12-31' AND YEAR(localfrom) < YEAR(localto) AND"
			" time(now()) BETWEEN time(localfrom) AND '23:59:59') OR (date_format(localfrom,'%s')='12-31' AND YEAR(localfrom) < YEAR(localto) AND"
			" date_format(now(),'%s')  BETWEEN '01-01 00:00:00' and date_format(localto,'%s')))) OR ( mtype = 2 AND periodic = 1  AND"
			" ((DATE_FORMAT(now(), '%w') = DATE_FORMAT(localfrom, '%w') AND time(now()) BETWEEN time(localfrom) AND time(localto)) OR"
			" (DATE_FORMAT(now(), '%w') = DATE_FORMAT(localfrom, '%w') AND time(now()) BETWEEN time(localfrom) AND '23:59:59') AND DATE(localfrom) < DATE(localto)"
			" OR (DATE_FORMAT(now(), '%w') = DATE_FORMAT(localto, '%w') AND time(now()) BETWEEN '00:00:00' AND time(localto)) AND DATE(localfrom) < "
			" DATE(localto))))"
			" UNION "
			" SELECT STRAIGHT_JOIN DISTINCT 2, greetingid, operatortype FROM groupgreetings gm, special sp WHERE gm.siteid = sp.siteid AND sp.siteid = %d"
			" AND sp.code = specialid AND specialid = %d AND ( (localfrom > localto AND ( time(now()) BETWEEN localfrom AND time('23:59:59') OR"
			" time(now()) BETWEEN '00:00:00' AND localto)) OR ( time(now()) BETWEEN localfrom AND localto ) ) ORDER BY 1,2 ";
	db_res_t *res=NULL;
	char cQuery[2048];
	int nFlag = -1;
	int nGreetingID = 1;
	int nOprtype=0;
	int nExtension=0;
	char cHeader[64] = "";

	switch ( gType ) {
		case ACDGroup:
			/*!
		 	 *	*	Project :CNAM
			 *  *   Date :18-11-2009
			 *  *   Usage: Send CNAM request
			 *  * * */

			LOG(L_WARN, "[frame_group_uri] ACD Group\n");
			if(strncmp(cnam_enable,"yes",3) || ws_cnam_request(msg,nSiteId)){
				LOG(L_ERR, "[frame_group_uri] CNAM is not enabled\n");
				snprintf(cNewuri,nLen-1,"sip:acd-%s-%d%0*d-%s-%s", wsuri->command, nSiteId, nGrpId_Parser, nGroupId, wsuri->context, wsuri->token);
			}else{
				snprintf(cNewuri,nLen-1, "sip:acdcnam-%s-%d%0*d-%s-%s",wsuri->command, nSiteId, nGrpId_Parser, nGroupId, wsuri->context, wsuri->token);
				LOG(L_WARN, "[frame_group_uri] CNAM is enabled\n");
			}
			return 0;
			break;

		case ExtendedGroup:
			LOG(L_ERR, "[frame_group_uri] Extended\n");
			memset(cQuery,0,sizeof(cQuery));
			snprintf(cQuery, sizeof(cQuery)-1, formatstring, nSiteId, nGroupId, "%m-%d %H:%i:%s", "%m-%d %H:%i:%s", "%m-%d %H:%i:%s","%m-%d", "%m-%d",
																				"%m-%d %H:%i:%s", "%m-%d %H:%i:%s", nSiteId, nGroupId );
			LOG(L_WARN, "[frame_group_uri] Query: \n%s\n", cQuery);
			ws_dbf.raw_query (h, cQuery, &res);
			if(res==NULL){
				LOG(L_ERR,"[frame_group_uri]'res' is NULL");
				return -1;
			}

			if (RES_ROW_N(res) > 0) {
				nFlag =  get_db_int (RES_ROWS(res)[0].values[0]);
				nGreetingID = get_db_int(RES_ROWS(res)[0].values[1]);
				LOG(L_WARN, "[frame_group_uri] nFlag: %d and nGreetingID : %d\n",nFlag, nGreetingID);

				if (nGreetingID != 0) {
					LOG(L_ERR, "[frame_group_uri]  ExtendedGroup : Mask Found/Regular IVR\n");
					if (RES_ROWS(res)[0].values[2].nul == FALSE){
						nOprtype = get_db_int(RES_ROWS(res)[0].values[2]);
					}
					if(nOprtype == WS_OPRTYPE_GROUP || nOprtype == WS_OPRTYPE_USER){
						snprintf(cNewuri, nLen-1, "sip:ivr-%d-%d-%s-%s", nGroupId, nGreetingID, wsuri->context, wsuri->token);
					}else{
						LOG(L_ERR,"[frame_group_uri] ExtentedGroup: There is no user or group setting!\n");
						snprintf(cNewuri, nLen-1, "sip:nop-%d-%d-%s-%s", nGroupId, nGreetingID, wsuri->context, wsuri->token);
					}
					if (res){
						ws_dbf.free_result (h, res);
					}
					return 0;

				}else {
					LOG(L_WARN, "[frame_group_uri]  ExtendedGroup : **operator** greeting found\n");

					if (res) ws_dbf.free_result (h, res);
					nExtension = get_operator_extension(nGroupId, nSiteId );
					if(nExtension > 0){
						snprintf (cNewuri, nLen-1, "sip:opr-%d-%d-%s-%s", nExtension,nGroupId, wsuri->context, wsuri->token);
					}else{
						snprintf (cNewuri, nLen-1, "sip:opr-01-%d-%s-%s", nGroupId, wsuri->context, wsuri->token);
					}
					return 0;
				}
			}else {
				LOG(L_WARN, "[frame_group_uri]  ExtendedGroup : Default IVR\n");
				if(res) {
					ws_dbf.free_result (h, res);
				}
				snprintf(cNewuri, nLen-1, "sip:ivr-%d-1-%s-%s", nGroupId, wsuri->context, wsuri->token);
				return 0;
			}
			break;
		case DisasterACDGroup:
			if(!is_header_exists(msg,"X-Noreplookup",13,cHeader,sizeof(cHeader))){
				memset(cHeader, 0, sizeof(cHeader));
				snprintf(cHeader, sizeof(cHeader)-1, "X-Noreplookup: Yes\r\n");
				add_header(msg, cHeader, strlen(cHeader));
			}
			if (strlen(disasterphonenumber)) {
				snprintf(cNewuri, nLen-1, "sip:oms-%s-%s-%s-%s", disasterphonenumber, wsuri->group, wsuri->context, wsuri->token);
				return 0;
			}else {
				snprintf(cNewuri, nLen-1, "sip:vml-%d-404-%s-%s", nGroupId, wsuri->context, wsuri->token);
				return 1;
			}
			break;

		default:
			LOG(L_WARN, "[frame_group_uri] Unlikely place!! Claling a regular group!\n");
			return -1;
	}
		return -1;
}

/* Checking is given extension assigned to associate account ???  if yes then framing URI and relaying request to balancer   --Prashant/Swaroopa*/
int frame_associate_acc_uri(struct sip_msg *msg, char *priority, struct ws_uri *wsuri, char *cNewuri, int nLen)
{
	int nFlag = -1;
	int nIs_conf = 0;
	int nSiteid = 0;
	int nRes    = 0;
	int nGroupcode=0;
	char cSitename[256] = "";
	char cQuery[4096] = "";
	char cDisasterphonenumber[64] = "";
	struct ws_uri *wsuri1 = NULL;
	enum GroupType gType;
	db_row_t row ;
	db_res_t *result = NULL;
	char * ws_ext_query = ""
			" SELECT 1 assignedto, REPLACE(si.sitename,'.','-'), si.siteid, ag.conference, 0 grpcode FROM agent ag, "
			" siteinfo_new si WHERE si.siteid = ag.siteid AND si.siteid IN (SELECT a.siteid "
			" FROM siteinfo_new a, accountassociationdetails b WHERE a.multiaccountassociationid = b.associationid AND b.sitename = REPLACE('%s','-','.') AND "
			" a.sitename != REPLACE('%s','-','.')) "
			" AND si.sitename != REPLACE('%s','-','.') AND extensions = '%s' "
			" UNION "
			" SELECT 2 assignedto, REPLACE(si.sitename,'.','-'), si.siteid, 0 conference, sp.code FROM special sp, "
			" siteinfo_new si WHERE si.siteid = sp.siteid AND sp.siteid IN (SELECT a.siteid FROM "
			" siteinfo_new a, accountassociationdetails b WHERE a.multiaccountassociationid = b.associationid "
			" AND b.sitename = REPLACE('%s','-','.') AND a.sitename != REPLACE('%s','-','.')) AND si.sitename != REPLACE('%s','-','.') "
			" AND extensions = '%s' LIMIT 1 ";

	if ( !wsuri->context || !wsuri->type || !wsuri->command || !wsuri->group || !wsuri->token) {
		LOG (L_WARN, "[frame_associate_acc_uri] Invalid wsuri format??:\n");
		return -1;
	}
	memset(cQuery,0,sizeof(cQuery));
	snprintf(cQuery,sizeof(cQuery)-1,ws_ext_query,wsuri->context,wsuri->context,wsuri->context,wsuri->command,wsuri->context,wsuri->context,
			wsuri->context,wsuri->command);
	ws_dbf.raw_query (h, cQuery, &result);
	if(result==NULL){
		LOG(L_ERR,"[frame_associate_acc_uri]'result' is NULL with Query : '%s' \n", cQuery);
		return -1;
	}
	if (RES_ROW_N(result) > 0) {
		row = RES_ROWS(result)[0];
		if (row.values[0].nul == FALSE)
			nFlag = get_db_int (row.values[0]);
		if (row.values[1].nul == FALSE) {
			strncpy(cSitename,row.values[1].val.string_val,sizeof(cSitename)-1);
		}
		if (row.values[2].nul == FALSE)
			nSiteid = get_db_int (row.values[2]);
		if (row.values[3].nul == FALSE)
			nIs_conf = get_db_int (row.values[3]);
		if (row.values[4].nul == FALSE)
			nGroupcode = get_db_int (row.values[4]);
		}

		LOG(L_WARN,"[frame_associate_acc_uri]: Flag:'%d' Sitename:'%s' Siteid:'%d' Is_conf:'%d' Groupcode:'%d' \n",nFlag, cSitename, nSiteid, nIs_conf, nGroupcode);
		if(result){
			ws_dbf.free_result(h,result);
		}
		wsuri1=wsuri;
		wsuri1->context=cSitename;
		if( nFlag == WS_USER_EXTEN ){
			if(nIs_conf == 1) {
				nRes = check_conf_exten (msg, wsuri1, nSiteid, cNewuri, nLen);
				if(nRes == -1) {
					snprintf(cNewuri, nLen-1, "sip:xfer-%s-0-%s-%s", wsuri1->command, wsuri1->context, wsuri1->token);
				}
			}else {
				snprintf(cNewuri, nLen-1, "sip:xfer-%s-0-%s-%s", wsuri1->command, wsuri1->context, wsuri1->token);
			}
		}else if(nFlag == WS_GROUP_EXTEN){
			gType = get_group_type(nGroupcode, nSiteid, cDisasterphonenumber,sizeof(cDisasterphonenumber));
			if(frame_group_uri (msg, wsuri1, nGroupcode, nSiteid, gType, cDisasterphonenumber, cNewuri, nLen) == -1) {
				snprintf(cNewuri, nLen, "sip:gext-%s-0-%s-%s", wsuri1->command, wsuri1->context, wsuri1->token);
			}
		}else{
			return -1;
		}
		return 1;
}


/* Checking is given extension is feature code ???  if yes then framing URI according to the feature code and relaying request  to balancer   --Prashant/Swaroopa*/
int frame_feature_codes_uri (struct sip_msg *msg, int nFeature, char *cToken, char *cSiteName, char *cNewuri, int nLen) 
{
    int nRet = 0;
    int nSiteId = 0;
	if(cSiteName == NULL) {
        LOG(L_ERR," [frame_feature_codes_uri] SiteName found NULL \n");
		return -1;
	}

	LOG(L_ERR," [frame_feature_codes_uri] SiteName : '%s' \n", cSiteName);

	switch(nFeature) {
		case SmartConnect:
        {
			nSiteId = GetSiteId(cSiteName);
			snprintf(cNewuri, nLen-1, "sip:9994-%d-9994", nSiteId);
			route_to_server(msg, cNewuri, smartconnect_server,OTHER);
        }
		break;

		case AcdAgent:
	        snprintf (cNewuri, nLen-1, "sip:agt-9997-0-%s-%s", cSiteName, cToken);
	    break;
		case GreetingAdmin:
			snprintf (cNewuri, nLen-1, "sip:rec-9998-0-%s-%s", cSiteName, cToken);
		break;
		case VoicemailAdmin:
			snprintf (cNewuri, nLen-1, "sip:vmr-9999-0-%s-%s", cSiteName, cToken);
		break;
		case AutoProvision:
			snprintf (cNewuri, nLen, "sip:wsprov-9995-0-%s-%s", cSiteName, cToken);
		break;

		default:
        {
			snprintf(cNewuri, nLen-1, "sip:inv-0-0-pandoranetworks-com-%s", cToken);
			_nNoRelay = 1;
            nRet = -1;
		    LOG(L_ERR," [frame_feature_codes_uri] **** Not a valid feature code \n ");
        }
			break;
	}
	return nRet;
}

/* Checking is given extension is 611 ???  if yes then framing URI (user set for 611 )  and relaying request  to balancer   --Prashant/Swaroopa*/
int frame_611_call_uri(struct sip_msg *msg, struct ws_uri *wsuri, char *cNewuri, int nLen)
{
	int nRet = -1;
	db_res_t *res = NULL;
	char cQuery[1024],cAgentId[64] = "",cUserExtn [32] = "",cSiteName[64]="",*cPtr = NULL;

	memset(cQuery, 0x00, sizeof(cQuery)-1);
	memset(cAgentId, 0x00, sizeof(cAgentId)-1);
	memset(cUserExtn, 0x00, sizeof(cUserExtn)-1);
	memset(cSiteName, 0x00, sizeof(cSiteName)-1);
	snprintf(cQuery, sizeof(cQuery)-1, GET611_AGENTID_QUERY, wsuri->context);
    
	ws_dbf.raw_query (h, cQuery, &res);

	nRet = Getting_DbResult(res, cAgentId,sizeof(cAgentId)-1);
	if(nRet == -1)
	{
		strncpy(cSiteName, ws_support_site, sizeof(cSiteName)-1);
		strncpy(cUserExtn, ws_support_exten, sizeof(cUserExtn)-1);
		LOG(L_ERR," [frame_611_call_uri] Getting AgentId Query Failed:'%s'", cQuery);
	}
	else
	{
		memset(cQuery, 0x00, sizeof(cQuery)-1);
		
		snprintf(cQuery, sizeof(cQuery)-1,GET_611USER_EXTN_QUERY,cAgentId);
		ws_dbf.raw_query (h, cQuery, &res);

		nRet = Getting_DbResult(res, cUserExtn, sizeof(cUserExtn)-1);
		if(nRet == -1)
		{
			strncpy(cSiteName, ws_support_site, sizeof(cSiteName)-1);
			strncpy(cUserExtn, ws_support_exten, sizeof(cUserExtn)-1);
			LOG(L_ERR,"[frame_611_call_uri] Getting Username Query Failed:'%s'",cQuery);
		}
		else
		{
			cPtr = strchr(cAgentId,'@');
			if(cPtr != NULL) {
 				cPtr++;
				if(cPtr)
					strcpy(cSiteName,cPtr);
           
				cPtr = strchr(cSiteName,'.');
				while(cPtr)
				{
					*cPtr = '-';
					cPtr = strchr(cSiteName,'.');
				}
			}
		}
		/*Here We are assigning Corresponding Customer/Agent Who are Handle 611 Calls --Jagan*/
		strncpy(cSiteName, ws_support_site, sizeof(cSiteName)-1);
		strncpy(cUserExtn, ws_support_exten, sizeof(cUserExtn)-1);
	}
	
	snprintf(cNewuri, nLen-1, "sip:xfer-%s-0-%s-%s", cUserExtn, cSiteName, wsuri->token);
	LOG(L_ERR,"[frame_611_call_uri] Call Routing to Support Who's Exten:'%s' and Sitename:'%s' \n", cUserExtn, cSiteName);
	return 0;
}

/* Added to get siteid based on sitename  --Prashant/Surekha */
int GetSiteId(char *cSiteName) 
{
	db_res_t *res = NULL;
	db_row_t row ;
	char cQuery[512]="";
	int nSiteId=0;
	
	if(cSiteName == NULL) {
		LOG(L_ERR,"[GetSiteId] SiteName is NULL  \n");
		return -1;
	}
	memset(cQuery, 0x00, sizeof(cQuery)-1);
	snprintf(cQuery, sizeof(cQuery)-1, GET_SITEID_QUERY, cSiteName);
	
	ws_dbf.raw_query (h, cQuery, &res);
	if(res == NULL){
	    LOG(L_ERR,"[GetSiteId] 'result' is NULL with Query : '%s' \n", cQuery);
		return -1;
	}

	if (RES_ROW_N(res) > 0) {
	    row = RES_ROWS(res)[0];
		if (row.values[0].nul == FALSE)
		    nSiteId = get_db_int (row.values[0]);
	}

    if(res){
        ws_dbf.free_result(h, res);
    }
    return nSiteId;
}

int validate_calleid_611(struct sip_msg * msg, char *cToken, char * cSix11extensionsitename)
{
    struct to_body  fromb;
    struct sip_uri uri ;
    char fromuser[256] = "",query[256]= "", cCallerid[256]= "",cSitename[64]= "", cExtn[16]="" ,cTmpSitename[256]="", *cPtr = NULL, cNewuri[256] = "" ,cData[64]= "";
    int nSiteid = 0, i,is_blocked_call= 0, nExtension = -1;

    db_res_t *res=NULL;
    db_row_t row;

    memset(fromuser,0x00, sizeof(fromuser));
    memset(query,0x00, sizeof(query));
    memset(&fromb, 0, sizeof(struct to_body));
    memset(cCallerid,0x00, sizeof(cCallerid));
    memset(cExtn,0x00, sizeof(cExtn));
    memset(cSitename,0x00, sizeof(cSitename));
    memset(cTmpSitename,0x00, sizeof(cTmpSitename));
    memset(cNewuri,0x00,sizeof(cNewuri));
    memset(cData,0x00,sizeof(cData));

    if(cSix11extensionsitename == NULL || strlen(cSix11extensionsitename) <= 0) {
        LOG(L_ERR,"[validate_calleid_611]call without extension & sitename ??????????\n");
        return -1;
    }

    if ( msg == NULL || msg->from == NULL || parse_headers(msg,HDR_FROM,0) == -1){
        LOG(L_ERR,"[validate_calleid_611]call without From header?\n");
        send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
        _nNoRelay = 1;
        return -1;
    }

    parse_to(msg->from->body.s,msg->from->body.s+msg->from->body.len+1,&fromb);

    if(fromb.uri.s == NULL || fromb.uri.len <= 0){
        LOG(L_ERR,"[validate_calleid_611]parsing of From header is failed\n");
        send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
        return -1;
    }

    parse_uri(fromb.uri.s, fromb.uri.len, &uri);
    if(uri.user.s == NULL || uri.user.len <= 0){
        LOG(L_ERR,"[validate_calleid_611]parsing of From URI is failed\n");
        send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
        return -1;
    }

    strncpy(cData, cSix11extensionsitename, sizeof(cData) - 1);
    memset(cTmpSitename,0 ,sizeof(cTmpSitename)-1);

    cPtr = strchr(cData, '#');
    if(cPtr) {
        *cPtr = '\0';
        cPtr++;
        if(cPtr) {
            strncpy(cTmpSitename, cPtr,sizeof(cTmpSitename)-1);
        }
    }
    nExtension = atoi(cData);  
    snprintf(fromuser, sizeof(fromuser)-1,"%.*s",uri.user.len ,uri.user.s);
    _get_agentid(msg,fromuser,NULL);
    snprintf(cSitename, sizeof(cSitename)-1,"%s",cTmpSitename);
    SITENAME_PARSE(cSitename);
    snprintf(query,sizeof(query)-1,LOCAL_CALLERID_QUERY_611,fromuser,cSitename);
    ws_dbf.raw_query (h, query, &res);

    if(res && RES_ROW_N(res) > 0) {
        row = RES_ROWS(res)[0];
        if (!VAL_NULL(&row.values[0])) {
            strncpy(cCallerid,row.values[0].val.string_val,sizeof(cCallerid)-1);
        }
        nSiteid=get_db_int(row.values[1]);
    }

    if(res)
        ws_dbf.free_result (h,res);

    if((!strstr(fromuser,cSitename)) && (strlen(cCallerid) > 0)){
        is_blocked_call  = validate_callerid(cCallerid, nSiteid, cExtn,sizeof(cExtn)-1,sizeof(cCallerid)-1);
    }

    if(is_blocked_call > 0) {
        snprintf (cNewuri, sizeof (cNewuri), "ext-%d-00-%s-%s",nExtension,cTmpSitename,cToken);
        return route_to_blockdestination(msg,cExtn, cTmpSitename, cToken,is_blocked_call,cSix11extensionsitename,cNewuri);
    }

    return 0 ;
}

/*Added for getting agentid from From Header --kkmurthy 2009-07-27*/
static void get_fromuser(struct sip_msg *msg,char *fromuser,int from_len)
{
    struct to_body  fromb;
    struct sip_uri uri;

    if ( msg == NULL || fromuser == NULL || msg->from == NULL || parse_headers(msg,HDR_FROM,0) == -1){
        LOG(L_ERR,"[get_fromuser]call without From header?\n");
        send_reply(msg,400,"Bad Request Hint: From Header not framed properly");
        _nNoRelay = 1;
        return;
    }
    memset(&fromb, 0, sizeof(struct to_body));
    parse_to(msg->from->body.s,msg->from->body.s+msg->from->body.len+1,&fromb);
    if(fromb.uri.s == NULL || fromb.uri.len <= 0){
        LOG(L_ERR,"[get_fromuser]parsing of From header is failed\n");
        return;
    }
    memset(&uri, 0, sizeof(struct sip_uri));
    parse_uri(fromb.uri.s, fromb.uri.len, &uri);
    if(uri.user.s == NULL || uri.user.len <= 0){
        LOG(L_ERR,"[get_fromuser]parsing of From URI is failed\n");
        return;
    }
    memset(fromuser, 0, sizeof(fromuser));
    snprintf(fromuser,from_len-1, "%.*s",uri.user.len, uri.user.s);
}


/* Fun for routing OB inter account calls to umslb for stir/shaken --rajesh 06-04-2021 */
int route_to_balancer_on_did_checks(struct  sip_msg * msg, char * realdid , char * token ,char *didsitename ){

	char *ptr=NULL;
	char fromuser[128]="",buffer[256]="",query[512]="",sitename[128]="", cForwardUser[256] = "", cSiteName[128] = "";
	db_res_t *res=NULL;
	db_row_t row;

	if(!msg || !realdid || !strlen(realdid) || !token || !strlen(token) || !didsitename || !strlen(didsitename)) {
		LOG(L_ERR,"[route_to_balancer_on_did_checks]  inputs are empty so returning here ...!\n");
		return -1;
	}
	
	bzero(buffer,sizeof(buffer));
	bzero(query,sizeof(query));
	bzero(sitename,sizeof(sitename));
	bzero(cForwardUser,sizeof(cForwardUser));
	bzero(cSiteName,sizeof(cSiteName));

	if(is_header_exists(msg,"ForwardFromUser",15, cForwardUser, sizeof(cForwardUser)) && (ptr = strchr(cForwardUser,'-'))) {
		if(cForwardUser[0] != '-' && isdigit(ptr[-1])) { // cForwardUser[0] != '-' this condition never mets for forward user; we are checking the ptr -1 address inthis place for safe side added this condition
			ptr[-1] = '\0';
		}else{
			*ptr = '\0';
		}
		if(++ptr) {
			strncpy(cSiteName,ptr,sizeof(cSiteName)-1);
		}
	}

	memset(fromuser,0,sizeof(fromuser));
	get_fromuser(msg, fromuser,sizeof(fromuser));

	if (isdigit(fromuser[0]) && isdigit(fromuser[1])){
		
		snprintf(query, sizeof(query)-1, ACCOUNTDIDQUERY, fromuser);
		ws_dbf.raw_query (h, query, &res);
		if (res && RES_ROW_N(res) > 0 ) {
			row = RES_ROWS(res)[0];
		    if (!VAL_NULL(&row.values[3])) {
				strncpy(sitename, row.values[3].val.string_val,sizeof(sitename)-1);
			}
		}
		if(res){
			ws_dbf.free_result (h,res);
		}

	}else{
		
		ptr=strchr(fromuser,'-');
		if (ptr){
			if(fromuser[0] != '-' && isdigit(ptr[-1]) ){
				ptr[-1]='\0';
				ptr++;	
			}else{
				*ptr++='\0';
			}
			if(ptr) {
				strncpy(sitename, ptr,sizeof(sitename));
			}	
		}
	}

	if (strlen(sitename) > 1 && !strcmp(didsitename, sitename)){
		LOG(L_ERR,"[route_to_balancer_on_did_checks] did sitename < %s > from sitename < %s > Not inter account call so returning \n", didsitename, sitename);
		return -1;
	}

	LOG(L_ERR,"[route_to_balancer_on_did_checks]  cForwardUser : %s, cSiteName : %s, sitename : %s, didsitename : %s \n",cForwardUser, cSiteName, sitename, didsitename);

	if(strlen(cForwardUser) >0 && strlen(cSiteName)>0) { /* it is transfer or forward call. AS per new logic, routing the call to UMSLB same as disaster flow */
		snprintf(buffer, sizeof(buffer)-1, "oms-%s-%s-%s-%s",realdid,cForwardUser,cSiteName, token);
	}else if(strlen(sitename) > 0) {
		 snprintf(buffer, sizeof(buffer)-1, "oms-%s-%s-%s-%s",realdid,fromuser,sitename, token);
	}else {
		 snprintf(buffer, sizeof(buffer)-1, "oms-%s-%s-%s-%s",realdid,fromuser,didsitename, token);
	}

	if (WSState->nMediaServerType == PRIMARY)
	{
		return route_to_server(msg, buffer, umslb_proxy,MS);
	}else{
		return route_to_server(msg, buffer, umslb_proxy1,MS);
	}
	return 1;
}

int preparePAIheader(struct  sip_msg * msg){
	
	struct to_body * fromb;
	struct sip_uri uri;
	
	char fromuser[128]="" , display[128]="" , cBuffer[512] = "",cVerstat[128]="",cPaiuser[255]="",callerid[128]="",agentid[128]="";
	char *ptr=NULL;
	int ret=0, nIsverstat=0;	

	if(!msg){
		LOG(L_ERR,"[preparePAIheader] msg is null Unable to add the P-Asserted-Identity header \n");
		return 0;
	}
	
	memset(fromuser,0,sizeof(fromuser));
	memset(display,0,sizeof(display));
	memset(cVerstat,0,sizeof(cVerstat));
	memset(cBuffer,0,sizeof(cBuffer));
	
	if (!msg->from->parsed )
			parse_from_header(msg);

	fromb = get_from (msg);
	if (fromb == NULL){
			send_reply(msg, 400, "Bad Request Hint: From Header not framed properly");
			_nNoRelay = 1;
			return 0;
	}

	parse_uri(fromb->uri.s, fromb->uri.len, &uri);
	snprintf(fromuser, sizeof(fromuser)-1, "%.*s", uri.user.len,uri.user.s);
	if (fromb->display.len > 0) {
			snprintf(display, sizeof(display)-1, "%.*s", fromb->display.len,fromb->display.s);
	}

	memset(cBuffer,0,sizeof(cBuffer));
	if(is_header_exists(msg,"Privacy",7,cBuffer,sizeof(cBuffer))) {
			str cmd1={"Privacy",7};
			if (remove_hf) {
					if(remove_hf(msg, (char *)&cmd1,0)==1) {
							LOG(L_ERR,"[preparePAIheader] Privacy removing is success\n");
					}	
					else {
							LOG(L_ERR,"[preparePAIheader] remove_hf  Privacy failure\n");
					}	
			}
	}

	/*Checking if the message is already contains the PAI header or not*/
	if(is_header_exists(msg,"P-Asserted-Identity",19, cBuffer, sizeof(cBuffer))) {
	
		ptr = strstr(cBuffer,"verstat=");	
		if(ptr){
			nIsverstat=1;
			snprintf(cVerstat, sizeof(cVerstat)-1, "%s", ptr+8);
			ptr = NULL;
			ptr = strchr(cVerstat,'@');
			if(ptr) {
				*ptr='\0';
			}
		}

		ptr=NULL;
		ptr= strstr(cBuffer,"<sip:");
		if(ptr){
			ptr=ptr+5;
			snprintf(cPaiuser, sizeof(cPaiuser)-1, "%s", ptr);
			ptr = NULL;
			if(nIsverstat == 1){
				ptr = strchr(cPaiuser,';');
				if(ptr){
					*ptr='\0';
				}
			}else{
				ptr = strchr(cPaiuser,'@');
				if(ptr){
					*ptr='\0';
				}
			}
		}
	
		if(strstr(fromuser,"Anonymous") || strstr(fromuser,"Restricted")) {
				memset(cBuffer,0,sizeof(cBuffer));
				snprintf(cBuffer,sizeof(cBuffer)-1,"Privacy: id\r\n");

				str cmd1={cBuffer,strlen(cBuffer)};
				if(append_hf(msg, (char *)&cmd1,0)==1){
					LOG(L_ERR,"[preparePAIheader] Privacy append_hf success >> '%s'\n",cBuffer);
				}else{
					LOG(L_ERR,"[preparePAIheader] Privacy append_hf failure >> '%s'\n",cBuffer);
				}
			
				if(cPaiuser && strlen(cPaiuser) > 0){
					if(cPaiuser[0] == '+' ? isdigit(cPaiuser[1]) : isdigit(cPaiuser[0]) && isdigit(cPaiuser[2])){
						return 0;
					}else{
							memset(agentid,0,sizeof(agentid));
							snprintf(agentid, sizeof(agentid)-1,"%s",cPaiuser);
							_get_agentid(msg,agentid,NULL);
							memset(callerid, 0 , sizeof(callerid));

							ret = GetAgentCallerId(agentid, callerid, sizeof(callerid));
							if(ret != 0 || strlen(callerid) == 0) { // From user do not have the caller id or account donot have the callerid
								memset(callerid, 0 , sizeof(callerid));
								strncpy(callerid,fromuser,sizeof(callerid)-1);
							}
							/*Removing old P-Asserted-Identity header*/
							str cmd1={"P-Asserted-Identity",19};
							if (remove_hf) {
									if(remove_hf(msg, (char *)&cmd1,0)==1) {
											LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity remove_hf success\n");
									}		
									else {
											LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity remove_hf failure\n");
									}		
							}

							/*Adding new P-Asserted-Identity header*/
							memset(cBuffer,0,sizeof(cBuffer));
							if(nIsverstat == 1){
								snprintf(cBuffer,sizeof(cBuffer)-1,"P-Asserted-Identity: %s <sip:%s;verstat=%s@%.*s>\r\n", display, callerid, cVerstat, uri.host.len+uri.port.len + 1, uri.host.s);
							}else{
								snprintf(cBuffer,sizeof(cBuffer)-1,"P-Asserted-Identity: %s <sip:%s@%.*s>\r\n", display, callerid, uri.host.len+uri.port.len + 1, uri.host.s);
							}

							str cmd={cBuffer,strlen(cBuffer)};
							if(append_hf(msg, (char *)&cmd,0)==1){
									LOG(L_ERR, "[preparePAIheader] P-Asserted-Identity append_hf success >> '%s'\n",cBuffer);
							}else{
									LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity append_hf failure >> '%s'\n",cBuffer);
							}

					}
				}
				return 0;
		}

		/*Removing old P-Asserted-Identity header*/
		str cmd1={"P-Asserted-Identity",19};
		if (remove_hf) {
			if(remove_hf(msg, (char *)&cmd1,0)==1) {
					LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity remove_hf success\n");
			}		
			else {
					LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity remove_hf failure\n");
			}		
		}
		
		/*Adding new P-Asserted-Identity header*/
		memset(cBuffer,0,sizeof(cBuffer));
		 if(nIsverstat == 1){
		snprintf(cBuffer,sizeof(cBuffer)-1,"P-Asserted-Identity: %s <sip:%s;verstat=%s@%.*s>\r\n", display, fromuser, cVerstat, uri.host.len+uri.port.len + 1, uri.host.s);
		 }else{
		snprintf(cBuffer,sizeof(cBuffer)-1,"P-Asserted-Identity: %s <sip:%s@%.*s>\r\n", display, fromuser, uri.host.len+uri.port.len + 1, uri.host.s);
		 }
				
		str cmd={cBuffer,strlen(cBuffer)};
		if(append_hf(msg, (char *)&cmd,0)==1){
			LOG(L_ERR, "[preparePAIheader] P-Asserted-Identity append_hf success >> '%s'\n",cBuffer);
		}else{
			LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity append_hf failure >> '%s'\n",cBuffer);
		}

	}else if (strstr(fromuser,"Anonymous") || strstr(fromuser,"Restricted")){/*Adding PAI header in case of NOT there*/

			memset(cBuffer,0,sizeof(cBuffer));
			if(is_header_exists(msg,"InitialCnamCid",14,cBuffer,sizeof(cBuffer))) {
					if((ptr=strrchr(cBuffer,'#'))) {
						*ptr='\0';
						if((ptr=strchr(cBuffer,'#')) && ++ptr) {
							memset(agentid,0,sizeof(agentid));
							snprintf(agentid, sizeof(agentid)-1,"%s",ptr);
							_get_agentid(msg,agentid,NULL);
						}				
					}
					memset(callerid, 0 , sizeof(callerid));

					ret = GetAgentCallerId(agentid, callerid, sizeof(callerid));
					if(ret != 0 || strlen(callerid) == 0) { // From user do not have the caller id or account donot have the callerid
							memset(callerid, 0 , sizeof(callerid));
							strncpy(callerid,fromuser,sizeof(callerid)-1);
					}
					
					memset(cBuffer,0,sizeof(cBuffer));
					snprintf(cBuffer,sizeof(cBuffer)-1,"P-Asserted-Identity: \"Anonymous\" <sip:%s@%.*s>\r\n", callerid, uri.host.len+uri.port.len + 1, uri.host.s);
					str cmd={cBuffer,strlen(cBuffer)};
					if(append_hf(msg, (char *)&cmd,0)==1){
							LOG(L_ERR, "[preparePAIheader] P-Asserted-Identity append_hf success >> '%s'\n",cBuffer);
					}else{
							LOG(L_ERR,"[preparePAIheader] P-Asserted-Identity append_hf failure >> '%s'\n",cBuffer);
					}

					memset(cBuffer,0,sizeof(cBuffer));
					snprintf(cBuffer,sizeof(cBuffer)-1,"Privacy: id\r\n");

					str cmd1={cBuffer,strlen(cBuffer)};
					if(append_hf(msg, (char *)&cmd1,0)==1){
							LOG(L_ERR,"[preparePAIheader] Privacy append_hf success >> '%s'\n",cBuffer);
					}else{
							LOG(L_ERR,"[preparePAIheader] Privacy append_hf failure >> '%s'\n",cBuffer);
					}
			}
	}
	return 1;
}

int ws_process_response(struct sip_msg *msg)
{
	int ret = -1;
	int ntype = 1;
	int nVal = 0;

	char cHeader[128]="";
	char cReason[128]="";
	char *cPtr = NULL;

	struct sip_msg p_msg;

	if(msg == NULL){
		LOG(L_ERR, "[ws_process_response]msg found NULL so returning here...\n");
		return ret;
	}

	memset(cHeader, 0, sizeof(cHeader));
	memset(cReason, 0, sizeof(cReason));

	cPtr = strstr(msg->headers->name.s, "X-Reject-Reason: " );
	if(cPtr && (cPtr+17)) {
		strncpy(cHeader, cPtr+17, sizeof(cHeader)-1);
		if((cPtr = strchr(cHeader,'\r'))) {
			*cPtr = '\0';
		}

		cPtr = strchr(cHeader, ';');
		if(cPtr != NULL){
			*cPtr = '\0';
			if(++cPtr){
				snprintf(cReason, sizeof(cReason)-1, "%s", cPtr);
			}
		}
		nVal = atoi(cHeader);
	}

	LOG(L_WARN, "processing response.. '%d' - '%s'\n", nVal, cReason);

	if(save_stir_details_to_msg){

		ret = save_stir_details_to_msg(&msg, &nVal, &cReason);
		if(ret != 0){
			LOG(L_ERR, "failed to update the flag in orig transaction \n");
		}
	}

	return -1;
}

int GetAgentCallerId(char *agentid, char *callerid, int len) {

	char query[256] = "";
	db_res_t *res=NULL;
	
	if (!agentid) {
		LOG(L_WARN,"[GetAgentCallerId] inputs are wrong so returning from here ");
		return -1;
	}
	
	memset(query,0,sizeof(query));
	snprintf(query,sizeof(query)-1,LOCAL_CALLERID_QUERY,agentid);

	ws_dbf.raw_query (h, query, &res);
	if(res && RES_ROW_N(res) > 0){
		if (RES_ROWS(res)[0].values[0].nul == FALSE) {
			snprintf(callerid, len - 1, "%s", RES_ROWS(res)[0].values[0].val.string_val);
			if (strlen (callerid) == 0 && RES_ROWS(res)[0].values[5].nul == FALSE){
				snprintf(callerid, len - 1, "%s", RES_ROWS(res)[0].values[5].val.string_val);
			}
		}
	}	
	if (res){
		ws_dbf.free_result(h, res);
		res = NULL;
	}
	return 0;
}

int GetAgentAssignedDID(char *agentid, char *phnumber, int len) {

	char query[256] = "";
	const char *formatstring = "select phnumber from agent a, accountphonenumbers ph where a.agentid = '%s' AND a.siteid = ph.siteid AND a.extensions = ph.extensions AND ph.assignedto != %d limit 1";

	db_res_t *res=NULL;
	db_row_t row;

	if (!agentid) {
		LOG(L_WARN,"[GetAgentAssignedDID] inputs are wrong so returning from here ");
		return -1;
	}
	
	memset(query, 0, sizeof(query));
	memset(phnumber, 0, len);
	snprintf(query, sizeof(query) - 1, formatstring, agentid, ASSIGNED_TO_FAX);

	LOG(L_WARN,"[GetAgentAssignedDID] query here is :'%s' \n", query);
	ws_dbf.raw_query (h, query, &res);
	if(res==NULL || RES_ROW_N(res) <= 0){
		if(res) {
			ws_dbf.free_result (h,res);
		}	
		return -1;
	}
	
	if (RES_ROW_N(res) > 0) {
		row = RES_ROWS(res)[0];
		if(row.values[0].nul == FALSE) {	
			strncpy (phnumber, row.values[0].val.string_val, len - 1);	
		}	
	}
	
	if (res){
		ws_dbf.free_result (h,res);
		res=NULL;
	}
	return 0;
}	

int get_from_displayname(struct sip_msg *msg, char *display, int len) {

  	struct to_body fromb;
    	struct sip_uri uri;

	if ( msg == NULL || msg->from == NULL || parse_headers(msg,HDR_FROM,0) == -1){
		return -1;
	}
	
	memset(&fromb, 0, sizeof(struct to_body));
	parse_to(msg->from->body.s,msg->from->body.s+msg->from->body.len+1,&fromb);
	if(fromb.uri.s == NULL || fromb.uri.len <= 0){
		return -1;
	}
	
	parse_uri(fromb.uri.s, fromb.uri.len, &uri);
	if(uri.user.s == NULL || uri.user.len <= 0){
		return -1;
	}

	if(fromb.display.len > 0) {
		snprintf(display, len-1, "%.*s",fromb.display.len ,fromb.display.s);
	}
	return 0;
}	

int modify_from_to_for_buddydialing(struct sip_msg *msg, char *touser, char *token) {

	int ret = 0;
	int bracefound = 0;

	char fromuser[256] = "";
	char agentid[256] = "";
	char fromsitename[256] = "";
	char tositename[256] = "";
	char phnumber[32] = "";
	char callerid[32] = "";
	char header[256] = "";
	char newheader[256] = "";
	char display[256] = "";
	char buffer[256] = "";
	char temp[256] = "";

	char *ptr = NULL, *ptr1 = NULL;

	memset(fromuser, 0x00, sizeof(fromuser));
	memset(fromsitename, 0x00, sizeof(fromsitename));
	memset(tositename, 0x00, sizeof(tositename));

	get_fromuser(msg,fromuser,sizeof(fromuser));

	if(!touser || isdigit(touser[0]) || !token) { // This is an extension dialing; it is not a buddy dialing
		return 0;
	}	


	ptr = strchr(fromuser, '-');
	if(ptr && ++ptr) {
		strncpy(fromsitename, ptr, sizeof(fromsitename) -1);
	}	

	ptr = strchr(touser, '-');
	if(ptr && ++ptr) {
		strncpy(tositename, ptr, sizeof(tositename) -1);
	}

	if(strlen(fromsitename) == 0 || strlen(tositename) == 0) {
		return 0;
	}	

	if(strcmp(fromsitename, tositename)) {
		// Both sitenames are not equal so it is an interaccount buddy call

		memset(agentid , 0 , sizeof(agentid));
		snprintf(agentid,sizeof(agentid) - 1,"%s",touser);
		_get_agentid(msg,agentid,NULL);

		memset(phnumber, 0 , sizeof(phnumber));
		ret = GetAgentAssignedDID(agentid, phnumber, sizeof(phnumber));
		if(ret != 0 || strlen(phnumber) == 0) { // Touser do not have the DID
			return 0;
		}	
		// User has the phone number get caller callerid.	

		memset(agentid,0,sizeof(agentid));
		snprintf(agentid, sizeof(agentid)-1,"%s",fromuser);
		_get_agentid(msg,agentid,NULL);
		memset(callerid, 0 , sizeof(callerid));

		ret = GetAgentCallerId(agentid, callerid, sizeof(callerid));
		if(ret != 0 || strlen(callerid) == 0) { // From user do not have the caller id or account donot have the callerid
			return 0;
		}


		// fromuser has the callerid

		memset(header, 0x00, sizeof(header));
		ret = is_header_exists(msg,"From", 4, header, sizeof(header));
		if(ret == 1) {
			
			memset(display, 0x00, sizeof(display));
			get_from_displayname(msg, display, sizeof(display));

			ptr = strchr(header, '<');
			if(ptr) {
				bracefound = 1;
			}	
	
			ptr = strchr(header, '@');
			if(ptr) {
				memset(newheader,0,sizeof(newheader));
				//memset(rpid,0,sizeof(rpid));
				if(bracefound == 1) {
					snprintf(newheader,sizeof(newheader)-1, "From: %s <sip:%s%s\r\n", display, callerid, ptr);
				}	
				else {
					memset(temp, 0x00, sizeof(temp));
					ptr1 = strstr(header, ";tag=");
					if(ptr1) {
						strncpy(temp, ptr1, sizeof(temp) - 1);
						*ptr1='\0';
					}	

					if(strlen(temp)) {
						snprintf(newheader,sizeof(newheader)-1, "From: %s <sip:%s%s>%s\r\n", display, callerid, ptr, temp);
					}	
				}
				if(strlen(newheader)) {

					str cmd1={"From", 4};
					if (remove_hf) {
						if(remove_hf(msg, (char *)&cmd1,0)==1)
							LOG(L_WARN,"remove_hf AAPSTN success\n");
						else
							LOG(L_ERR,"remove_hf AAPSTN failure\n");
					}
					add_header(msg, newheader, strlen(newheader));	
					//add_header(msg, rpid, strlen(rpid));
					memset(newheader,0,sizeof(newheader));
					//snprintf(newheader,sizeof(newheader)-1, "P-Streams-Direction: OB;%s;%s\r\n", callerid, phnumber);
					//add_header(msg, newheader, strlen(newheader));
					//*rpidadded = 1;	
				}	
			}


			memset(header, 0x00, sizeof(header));
			ret = is_header_exists(msg, "To", 2, header, sizeof(header));
			if(ret == 1) {
			
				ptr = strchr(header, '<');
				if(ptr) {
					bracefound = 1;
				}	
	
				memset(newheader,0,sizeof(newheader));
				ptr = strchr(header, '@');
				if(ptr) {
					if(bracefound == 1) {
						snprintf(newheader,sizeof(newheader)-1, "To: <sip:%s%s\r\n", phnumber, ptr);
					}	
					else {
						snprintf(newheader,sizeof(newheader)-1, "To: <sip:%s%s>\r\n", phnumber, ptr);
					}

					if(strlen(newheader)) {

							str cmd1={"To", 2};
							if (remove_hf) {
								if(remove_hf(msg, (char *)&cmd1,0)==1)
									LOG(L_WARN,"remove_hf AAPSTN success\n");
								else
									LOG(L_ERR,"remove_hf AAPSTN failure\n");
							}
						
							add_header(msg, newheader, strlen(newheader));	
					}	
				}

				bzero(buffer,sizeof(buffer));
    			snprintf(buffer, sizeof(buffer)-1, "oms-%s-0-%s-%s", phnumber, tositename, token);
				if(WSState->nMediaServerType == PRIMARY){
        			LOG(L_WARN,"[route_to_outboundserver] PRIMARY>>>>>>>>>>>>\n");
        			route_to_server(msg, buffer, umslb_proxy,OMS);
					return 1;
    			}else{
        			LOG(L_WARN,"[route_to_outboundserver] SECONDARY>>>>>>>>>>>>\n");
        			route_to_server(msg, buffer, umslb_proxy1,OMS);
					return 1;
    			}
			
	
			}
		}	
	}

	LOG(L_WARN,"[modify_from_to_for_buddydialing] From username is :'%s' to username is :'%s' fromsitename :'%s' tositename :'%s' agentid :'%s' phnumber:'%s' callerid :'%s' \n", fromuser, touser, fromsitename, tositename, agentid, phnumber, callerid);

	return 0;
}

/*to check fraud triggered or not --rajesh 17Aug2021*/
int isFruadTriggered( struct sip_msg *msg ){

		int nRet= 0 , siteid=0;
		char query[1024]="",agentid[256]="",fromuser[256]="",callid[256]="",cEndPointTable[128]="";
		db_res_t *res=NULL,*res1=NULL,*res2=NULL;
		db_row_t row,row1,row2;

		if(msg == NULL){
				LOG(L_ERR, "[isFruadTriggered] msg id NULL so returning here !! \n");
				return 0;
		}

		memset(agentid , 0 , sizeof(agentid));
		memset(fromuser , 0 , sizeof(fromuser));

		get_fromuser(msg,fromuser,sizeof(fromuser));

		bzero (query, sizeof (query));
		if(fromuser[0] == '+'){
				snprintf (query,sizeof(query)-1, WSDID_USER_DETAILS ,fromuser+1);
		}else{
				snprintf (query,sizeof(query)-1, WSDID_USER_DETAILS ,fromuser);
		}
		ws_dbf.raw_query (h, query, &res);
		if(res && RES_ROW_N(res) > 0) {
				row = RES_ROWS (res)[0];
				if (!VAL_NULL(&row.values[0])){
						siteid = get_db_int(row.values[0]);
				}
				if (!VAL_NULL(&row.values[1])) {
						strncpy(agentid, row.values[1].val.string_val,sizeof(agentid)-1);
				}
		}
		if (res)
				ws_dbf.free_result(h, res);

		if(!siteid || !agentid){
				return 0;
		}	

		bzero (query, sizeof (query));
		snprintf (query,sizeof(query)-1, ISFRAUDTRIGGERED, siteid , siteid , agentid);
		ws_dbf.raw_query (h, query, &res2);
		if(res2 && RES_ROW_N(res2) > 0) {
				row2 = RES_ROWS (res2)[0];
				if(!VAL_NULL(&row2.values[0])) {
					nRet = RES_ROWS(res2)[0].values[0].val.int_val;

				}
	}
		if (res2)
				ws_dbf.free_result(h, res2);

		return nRet;
}

/* Fun to get Specific monthwise tabel name for selecting end_point table details --rajesh 17 aug 2021 */
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

