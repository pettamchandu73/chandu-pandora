#ifndef _RACC_H_
#define _RACC_H_

#define RACC_BRANCHID_LEN		200
#define RACC_TO_LEN             200
#define	RACC_FROM_LEN			200
#define RACC_URI_LEN			100
#define RACC_CALLID_LEN			200
#define	RACC_FROMTAG_LEN		200
#define RACC_TOTAG_LEN			200
#define RACC_WS_TOKEN_LEN		10
#define RACC_WS_CALLINFO_LEN	100
#define RACC_WS_CONTACT_LEN		200
#define RACC_WS_USERAGENT_LEN	256
#define WSWEBCALLINFO           13
#define WSACDWEB                10

/* Added for 3pcc calls*/
#define CTC_TAG "-3pcc"
#define CTCTYPE "ctc-"

/* Call Status Messages */
enum STATUS
{
    	RACC_INVITE,		/* INVITE 				*/
    	RACC_RINGING,		/* 180 Ringing 				*/
    	RACC_ACCEPTED,		/* 200 OK (for INVITE)			*/
    	RACC_ESTABLISHED,	        /* ACK for INVITE			*/
    	RACC_TIMEDOUT,		/* 408 Request Timed Out                */
    	RACC_CANCELLED,		/* CANCEL				*/
    	RACC_REJECTED,		/* 486 Busy Here / 603 Decline	        */
    	RACC_TERMINATING,	        /* BYE					*/
    	RACC_TEMPMOVED,         	/* 302 moved temporarily                */
		RACC_HOLD,			/*HOLD-METRICS*/
		RACC_UNDEF,			/* What if something else comes?	*/
		RACC_302_INVITE,     /*302 INVITE FOR IVR CALLS              */
		RACC_DEFAULT
}STATUS;

/*Alter this structure care fully. If any changes made here, similar changes need to be made in racc module and Presence Agent --Krishna Murthy Kandi 2011-04-22*/
struct racc_call_details {
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
	char user_agent	[RACC_WS_USERAGENT_LEN];
	int timestamp;
	/* adding for missed details saving*/
	int resp_cause;
	int restype;
	char macid[64];
	/*Added for storing CNAM value*/
	char callername[128];
	char DialedNumber[128];
	char QName[128];
	char AccUser[256];
	/* Added to impliment the routing plan Feature: "Allow extension from PSTN phone field" */
	int nRoutingPlanExtFlag;
    char cAgentId[256];
    char cGrpName[256];
    int nMissedCallFlag ;
    char cSix11extensionsitename [256];
    int nVoicmailCalleridBlock ;
    int nResCode;
    int reasoncode;
    int nLcrReject;
    int nWSacdaacallerid;
    char rpid[128];
	struct racc_call_details *prev;
	struct racc_call_details *next;
};

enum FUNTYPES{
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
		F_PICKUP=10,
		F_SERVICE_UNAVILABLE=14,
		F_CALL_REJECTED_DUE_TO_CALLER_ID_BLOCK=15,
		F_CALL_COMPLETED_ELSEWHERE=16,
		F_ROUTINGRINGTIMEOUT, /* This won't be in DB */
		F_CANCELFORWARD, /* This won't be in DB */
		F_SPAM_REJECT = 20, /* bad-reputation and user reject set option */
        F_BLACKLISTED,
        F_SPAM_PLAY_MSG_AND_DISCONNECT=27
};
struct racc_call_details racc_call;
struct racc_call_details *billing_data_head=NULL,*billing_data_leaf = NULL;
extern int (*_get_macid)(struct sip_msg* m, char *macid, char* macaddrlen);
int is_header_exists(struct sip_msg *msg,char *header_name,int header_name_len,char *header_value,int header_value_len);

/* Added for stir-shaken */
struct racc_stir_details {

		int timestamp;
		char callid [256];
		char req_uri[256];
		char lcontact_ip[56];
		char contact_ip[256];
		char via_ips[256];
		struct racc_stir_details *prev;
		struct racc_stir_details *next;
};

struct racc_stir_details racc_stir;
struct racc_stir_details *stir_data_head=NULL, *stir_data_leaf = NULL;

#endif
