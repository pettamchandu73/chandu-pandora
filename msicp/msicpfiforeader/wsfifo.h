#ifndef _WS_FIFO_H_
#define _WS_FIFO_H_
#include <unistd.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RACC_BRANCHID_LEN		200
#define RACC_TO_LEN		     	200
#define RACC_FROM_LEN			200
#define RACC_URI_LEN			100
#define RACC_CALLID_LEN			200
#define RACC_FROMTAG_LEN		200
#define RACC_TOTAG_LEN			200
#define RACC_WS_TOKEN_LEN		10
#define RACC_WS_CALLINFO_LEN	100
#define RACC_WS_CONTACT_LEN		200
#define RACC_WS_USERAGENT_LEN   256

#define CALL_DECLINED 603
#define CALL_UNWANTED 607
#define CALL_REJECTED 608

#define LIVE 1
#define WARE 2
#define ACCACTIVE 3
#define WSPARTNERS 4
#define CONTACTSDB 5

int outfd;

/* Added for 3pcc calls*/
#define CTC_TAG "-3pcc"
#define ACDSUP_TAG "-acdsup"
#define CTCTYPE "ctc-"

/*Added by Tulsi for billing engine-23-07-09*/
#define UNSASSIGNED                 0
#define ASSGINED_TO_GROUP           1
#define ASSIGNED_TO_EXTENSION       2
#define ASSIGNED_TO_CONFERENCE      3
#define ASSIGNED_TO_X11PROFILE      4
#define ASSIGNED_TO_FAX             5
#define ASSIGNED_TO_SMARTCONNECT    6
#define ASSIGNED_TO_VOICEMAIL       7
#define ASSIGNED_TO_BLA             8

#define Pulse                       1
#define Call                        3

#define IPINB						2
#define PSINB 						1

#define OUTBOUND	2
#define IN_BOUND	1

#define INTIAL_TIME "0000-00-00 00:00:00"

#define MYSQL_TABLE_NOT_FOUND_ERRCODE 1146

/* Usage Billing Changes */
#define BVB "BUSINESSVOICEBASIC"
#define BV "BUSINESSVOICE"
#define COMMONSEAT "COMMONSEAT"
#define VIRTUALEXTENSION "VIRTUALEXTENSION"

/*-----------------ULM Changes---------------------*/
#define PRIMARY_CONNECTION              1
#define SECONDARY_CONNECTION            2

/* XML Protocol Format*/
#define XML_CALLDETAILS "<Message id=\"523\">\r\n" \
                        "<starttime><![CDATA[%s]]></starttime>" \
                        "<endtime><![CDATA[%s]]></endtime>" \
                        "<caller>%s</caller>" \
                        "<callee>%s</callee>" \
	                	"<callid>%s</callid>" \
	               		"<status>0</status>" \
                        "<cnam><![CDATA[%s]]></cnam>" \
						"<iscalleriddisplay>%d</iscalleriddisplay> \r\n" \
	                	"</Message>\r\n%s"

/*XML Pinging Protocol*/
#define XML_LOGINPROTOCOL "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
                          "<Message id=\"1009\" server=\"%s\"></Message>\r\n%s"

#define XML_PINGPROTOCOL  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
                          "<Message id=\"764\" server=\"%s\"></Message>\r\n%s"

char cXMLBuf[1024];


/*Alter this structure care fully. If any changes made here, similar changes need to be made in fiforeader and Presence Agent --Krishna Murthy Kandi 2011-04-22*/
struct wsacc_fifo_call_details {
	int method;
	int phone;
	int is_direct_call;
	/*! Basic call details */
	char uri	[RACC_URI_LEN];
	char to		[RACC_TO_LEN];
	char from	[RACC_FROM_LEN];
	/*! Call identifying  details */
	char callid		[RACC_CALLID_LEN];
	char branch		[RACC_BRANCHID_LEN];
	char fromtag	[RACC_FROMTAG_LEN];
	char totag		[RACC_TOTAG_LEN];
	char contact	[RACC_WS_CONTACT_LEN];
	char user_agent [RACC_WS_USERAGENT_LEN];
	int timestamp;
	/* Added for Missed calls */
	int resp_cause;
	int restype;
	char macid[64];
	/*it contains from calltype to calltype and to sitename*/
	//char call_info	[RACC_WS_CALLINFO_LEN];
	/*Added for storing CNAM value*/
	char callername[128];
	char DialedNumber[128];
	char QName[128];
	char AccUser[256];
	int nRoutingPlanExtFlag; /* Added to impliment call routing featue "EXT routing in PSTN phone field"*/
    char cAgentId[256];
    char cGrpName[256];
    int nMissedCallFlag ;
    char cSix11extensionsitename[256];
    int nVoicmailCalleridBlock ;
    int nResCode;
	int reasoncode;    
	int nLcrReject;
	int nWSacdaacallerid;
	char rpid[128];
	struct racc_call_details *prev;
    struct racc_call_details *next;

};

struct wsacc_fifo_call_details wsacc_call;

/*struct PstnPhone {
	char * phone;
	int nSiteid;
	int nAssignedTo;
	int nAssignedID;
	char sitename[50];
};

enum CallSource {
	MediaServer,
	OtherSource
};


enum GroupType {
	InvalidGroupType,
	RegularGroup,
	ExtendedGroup,
	ACDGroup,
	OperatorRouted
};*/

struct ws_uri {
	char parsed[256];
	char actual_token[32];
	char * type;
	char * command;
	char * group;
	char * context;
	char * token;
};

enum UriType {
	Invalid_Uri = -2,
	IVR_Number = 0,
	Operator_Routed,	
	IVR_Command,
	CNF_Request,
	ACD_Request,
	Direct_DID,
	Voice_Mail,
	Voice_Mail_Reading,
	Greeting_Manager,	
	Extension,
	DIR_Request,
	ACD_Voicemail,
	ACD_Voicemail_Transfer,
	ACD_Agent,
	PSTN_Transfer,
	Pickup_Request,
	MWI_Request,
	ACD_DID,
	FAX_URI,
	PTT_URI,
	OMS_Request,
	XFER_Request,
	Group_EXT,
	INB_Xfer,
	LCR,
	Phone_Number = 25,
	International_Phnumber,
	Feature_Request,
	WS_URI,
	Local_User,
    SmartLocal_Request,
	SmartPSTN_Request
};

enum CALLSTATUS 
{
	 RACC_INVITE,         /* INVITE                               */
	 RACC_RINGING,        /* 180 Ringing                          */
	 RACC_ACCEPTED,       /* 200 OK (for INVITE)             */
	 RACC_ESTABLISHED,    /* ACK for INVITE                  */
	 RACC_TIMEDOUT,      /* 408 Request Timed Out      */
	 RACC_CANCELLED,      /* CANCEL                          */
	 RACC_REJECTED,       /* 486 Busy Here / 603 Decline     */
	 RACC_TERMINATING,    /* BYE                                  */
	 RACC_TEMPMOVED,    /* Temporarily moved                      */
	 RACC_HOLD,				/*HOLD-METRICS*/
	 RACC_UNDEF,               /* What if something else comes?*/
	 RACC_302_INVITE,        /*302 INVITE for ivr calls*/
	 RACC_DEFAULT
}CALLSTATUS;

/*Modified by Tulsi for Billing engine..23-07-09*/
enum CALLTYPE {
	IP_INBOUND = 1,
	IP_OUTBOUND,
	PSTN_INBOUND,
	PSTN_OUTBOUND,
	VOICEMAIL_IP,
	VOICEMAIL_PSTN,
	PSTN_FORWARD,
	IP_FORWARD,
	TOLLFREE_INBOUND,
	CONFERENCING,
	INT_OUTBOUND,
	INBOUNDFAX,
	SMARTCASTIP,
	SMARTCASTPSIB,
	SMARTCASTTOLLFREE_INBOUND,
	SMARTCAST_OUTBOUND,
	SMARTCAT_RESERVE,
	OBXFER,
	INXFER,
	ACD_MONITOR,
	COMMONSEAT_OUTBOUND,
	TOLLFREE_CONF,
	TOLLFREE_FAX,
	TOLLFREE_OUTBOUND,
	NINEONEONE,
	FOURONEONE,
	NONACD_ABONDONED_TRANSFER_OUT=31,
	AAXFER
}CALLTYPE;

struct wscallratedetails
{
	char agentid[256];
	char group_title[64];
	char ownerdid[16];
	int groupcode;
	int siteid;
	int calltype;
	int callduration;
	int accounttype;
	int billtype;
	int serviceid;
	int ownerext;
	int nagtId;
	int pbxid;
	int assignedto;
	float pulserate;
	float callrate;
	float saasrate;
	float nonsaasrate;
	float callcost;
	char orgname[128];
	char poolname[128];
	char aliasdid[128];
	char tdiff[32];
    char sitename[256];
};

enum WSFUNTYPES{
	REQUEST_FWDED = 1,
    RESPONSE_OUT,
    RESPONSE_IN,
    REQUEST_FWDED_LOCAL
};

enum FAILEDCALLTYPE{
		F_DEFAULT,
		F_CANCEL,
		F_REJECT,
		F_RINGTIMEOUT,
		F_TIMEOUT,
		F_NOSERVICE,
		F_INV,
		F_OTHER,
		F_OANSWER,
		F_PENREG,
		F_PICKUP,
		F_NOREGSITER,
		F_SERVICE_UNAVILABLE=14,
		F_CALL_REJECTED_DUE_TO_CALLER_ID_BLOCK=15,
		F_CALL_COMPLETED_ELSEWHERE=16,
		F_ROUTINGRINGTIMEOUT, /* This won't be in DB */
		F_CANCELFORWARD, /* This won't be in in DB */
		F_SPAM_REJECT = 20, /* bad-reputation and user reject set option */
		F_BLACKLISTED,
		F_FRAUD_NUMBER,
		F_HIGH_RISK_CALLING_NUMBER,
		F_INVALID_CALLIG_NUMBER,
		F_INVALID_CALLED_NUMBER,
		F_OTHER_REASON,
		F_SPAM_PLAY_MSG_AND_DISCONNECT,
		F_BLOCK_BY_FAR_END_SERVICE_PROVIDER = 30,
		F_BLOCK_BY_FAR_END_CUSTOMER,
};

enum PHONETYPE {
    IP1 = 1,
    IP2,
    SOFTPHONE,
    PSTN_CALL,
    CTC_CALL,
    INBOUND,
	DISASTER_FORWARD
};

/* Added for stir-shaken */
struct wsacc_fifo_stir_details {

		int timestamp;
		char callid [256];
		char req_uri[256];
		char lcontact_ip[56];
		char contact_ip[256];
		char via_ips[256];
		struct racc_stir_details *prev;
		struct racc_stir_details *next;
};
struct wsacc_fifo_stir_details wsacc_stir;

#define GET_SITENAME for (i=0; site[i]; i++) {if ('-' == site[i]) site[i] = '_';}
#define GET_OLD_SITENAME for (i=0; old_site[i]; i++) {if ('-' == old_site[i]) old_site[i] = '_';}
/*Added by Tulsi*/
#define GET_ORGSITENAME for (i=0; orgsite[i]; i++) {if ('_' == orgsite[i]) orgsite[i] = '.';}
#define WS_SEPERATOR '-'
#define SITENAME_PARSE for (i=0; cSitename[i]; i++) {if ('-' == cSitename[i]) cSitename[i] = '.';}

//void racc_fifo_db_request (db_con_t* db_handle,db_func_t *racc_dbf,char *uri_field, char *cid_field, char *br_field, char *ftag_field, 
void racc_fifo_db_request (const char *);
void racc_fifo_write_final_data (char *cid_field, char *ftag_field, char *ttag_field, char *br_field,unsigned nCalltoken, int nIsFwdCall);
void racc_fifo_delete_records (char *callid, char *ftag, char *ttag);
void auto_disable_international_calls (struct wscallratedetails wscrd,char *site,char *f_field,char *t_field);
/*int parse_ws_fifo_uri_buffer(char * buffer, int nLength, struct ws_uri * wsuri);*/
int init_fiforeader(int dump);
/*---Added to convert the sitename into lowercase if any of the characters is in uppercase --09/12/2010--Priyam/KIMO--*/
void ws_set_to_lowercase(char *site);
/*--Added for query execution */
int ExecuteDbQuery(MYSQL **myDBhandler , char *cQuery , int nDbType , int nDBLogFlag);
int mysqlerr_no();
int retrymysqlerr_no(void);
int missedcall_details(int childno);
int get_agentid(char *agentid);
void Get_TimeZone(char *sitename,char *Tz_EndTime,int nLen,char *end_time);
int  Get_PhoneType(char *t_field,int *cPhone_Type);
void *start_presence_connection_pinger(void *);
int get_group_ext_did(int code, char *title, struct wscallratedetails *wscrd);
void get_Utc_timenow(char *cTime,char *cUtctime,int nUtctimeSize);
void GetCallerid(char *from,char *callername,char *callerid,int nLen);
void getProperAgent(char *cName, char *cBuf, int nLen);
void stir_racc_fifo_db_request();
#endif

