#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "wsfifo.h"
#include <sys/resource.h>
#include "property.h"
#include "dprint.h"
#include<time.h>
#include <sys/poll.h>

extern int errno;
MYSQL *db_live_handle;
MYSQL *db_wh_handle;
MYSQL *db_wspartners;
MYSQL *db_contact_handle;

/*configuration variables added by sharath*/
char livehost[32] = "";
char whhost[32] = "" ;
char liveuser[32] = "";
char whuser[32] = "";
char livepwd[32] = "";
char whpwd[32] = "";
char livedb[32] = "";
char whdb[32] = "";
char wsdb_fifo_name[256] = "";
int wsdb_fifo_count;
char who[25] = "";
/*configuration variables for alternate db -tulsi*/
char livehost1[32] = "";
char whhost1[32] = "" ;
char liveuser1[32] = "";
char whuser1[32] = "";
char livepwd1[32] = "";
char whpwd1[32] = "";
char livedb1[32] = "";
char whdb1[32] = "";
/*erron number maintains-tulsi*/
char errnumber[2048]="";
char retryerrnumber[2048]="";
int maxnoerr;
int retrymaxnoerr;
int queryretrycount = 1;
int errbuff[254];
int retryerrbuff[254];
/*These are for Billing Engine -Tulsi-26-07-09*/
float PulseRate=6.0;
/*International and internal billing flags --kkmurthy 2010-10-01*/
int intllookupflag=0;
int internalbilling = 0;
int singlecdrtable=0;   ///Added for VF
/* variables for syslog added by kkmurthy*/
int debug = 4;
int log_stderr = 0;
int log_facility = 2;
int nSpamRejectCode = 0;
int nCallerIdBlockRejectCode = 0;

char omsserver[25] = "";
char omsserver1[25] = "";

/*To Maintain active call records*/
int keepactivecallrecord = 0;
int nServiceid = 0;
int nDataCenterID = 0;

// Added variables for Acc Active table 
char _cAccActiveDbHost[32]  =  "" ;
char _cAccActiveDbUser[32]  =  "" ;
char _cAccActiveDbPass[32]  =  "" ;
char _cAccActiveDbName[32]  =  "" ;

char _cAccActiveOneDbHost[32]  =  "" ;
char _cAccActiveOneDbUser[32]  =  "" ;
char _cAccActiveOneDbPass[32]  =  "" ;
char _cAccActiveOneDbName[32]  =  "" ;

/* Added for missed call duration */
int missedcall_sleep = 0;
int missedcall_querytime = 0;
char user_agent[256]="";
int ws_ctccall_flag = 0;
int fwd_delay_time	= 0;

MYSQL * db_acc_active;

int Reporting_flag=0;

char servertz[32] = "";
char cTemplate_Table_For_LCR[32] = "";
char cPrefix_IntPstTable[32] = "";
int LcrCallType = 0;
int nRoutingPlanEXT_CallType = 0;
int nXferCallType = 0;
int nCancelAnotherCallType = 0;

/* Usage Billing Changes */
int Commonseat=0;
int Virtual_Seat=0;
int Business_Voice_Basic=0;

//Connection for wspresence
int connection_flag;
int server_sockfd;
struct sockaddr_in addr;
int _nIsRunning = 0;
int  _nLastPingResponse = 0;
int  pt_timer = 10;
int  failed_pt_timer = 5;
int  connectionflag = PRIMARY_CONNECTION;
char primary_presence_ip[128] = "";
int  primary_presence_port =  8892;
char secondary_presence_ip[128] = "";
int  secondary_presence_port =  8892;
int  _nonblockmode = 0;

/* Variables for WSPartner DB */
char cWsPartnersHost[32] = "";
char cWsPartnerUser[32] = "";
char cWsPartnerPwd[32] = "";
char cWsPartnerDB[32] = "";
 
/* Variables for Alternate WSPartner DB */
char cWsPartnersHost1[32] = "";
char cWsPartnerUser1[32] = "";
char cWsPartnerPwd1[32] = "";
char cWsPartnerDB1[32] = "";

/* Variables for CONTACTSDB */
char cContactsDB[32] = "";
char cContactsDBPwd[32] = "";
char cContactsDBHost[32] = "";
char cContactsDBUser[32] = "";

/* Variables for Alternate CONTACTSDB */
char cContactsDB1[32] = "";
char cContactsDBPwd1[32] = "";
char cContactsDBHost1[32] = "";
char cContactsDBUser1[32] = "";

int nEnableSleepPatch = 0;

/* Added for stir-shaken */
int stir_fifo_count;
char stir_fifo_name[256] = "";
char cEndPonitCdrTemplate[64] = "";
char cEndPonitCdrPrefix[64] = "";


int mysqlerr_no()
{
		char *errnum = NULL;
		int i=0;

		errnum = strtok(errnumber,",");
		if(errnum){
				errbuff[i]=atoi(errnum);
				i++;
				while(errnum = strtok(NULL,",")){
						errbuff[i]=atoi(errnum);
						i++;
				}
				maxnoerr = i;
				for(i=0;i<maxnoerr;i++){
				  LOG(L_ERR,"errornumber:%d\n",errbuff[i]);
				  }
				return 1;
		}
		return -1;
}

int retrymysqlerr_no()
{
		char *errnum = NULL;
		int i=0;

		errnum = strtok(retryerrnumber,",");
		if(errnum){
				retryerrbuff[i]=atoi(errnum);
				i++;
				while(errnum = strtok(NULL,",")){
						retryerrbuff[i]=atoi(errnum);
						i++;
				}
				retrymaxnoerr = i;
				for(i=0;i<retrymaxnoerr;i++){
				  	LOG(L_ERR,"retryerrornumber: %d\n",retryerrbuff[i]);
				}	
				return 1;
		}
		return -1;
}

MYSQL *getconnection(MYSQL *db_handle,char *host, char *user,char *pwd,char *db)
{
		/* Closing existing connection before getting new one --vravi */
		if(db_handle != NULL){
			mysql_close(db_handle);
			db_handle = NULL;
		}
		if(!host || !user || !pwd || !db){
			LOG(L_ERR,"[getconnection] Either host or user or pwd or db  is NULL\n");
			return NULL;
		}	 
		db_handle = mysql_init(NULL);
		if ( db_handle == NULL){
				LOG(L_ERR,"ERROR live database initiazation failed:%s\n",mysql_error(db_handle));
				return NULL;
		}

		if(!mysql_real_connect (db_handle,host,user,pwd,db, 0, NULL, 0)) {
				LOG(L_ERR,"Error connecting to live Database :%s\n",mysql_error(db_handle));
				return NULL;
		}
		LOG(L_WARN,"[getconnection]db handle %p!\n",db_handle);
		return db_handle;
}

int dbconnection(int wchdb){
		int i;
		int ret;

		if(wchdb == LIVE){
				for(i=0;i<3;i++){
						db_live_handle = getconnection(db_live_handle,livehost, liveuser, livepwd, livedb);
						if(db_live_handle == NULL){
								LOG(L_ERR,"[LIVEDB 1][dbconnection]%d time failed!\n",i);
								continue;
						}
						LOG(L_ERR,"[LIVEDB 1][dbconnection]%d time connection Sucess %p!\n",i,db_live_handle);
						return 1;
				}
				for(i=0;i<3;i++){
						db_live_handle = getconnection(db_live_handle,livehost1, liveuser1, livepwd1, livedb1);
						if(db_live_handle == NULL){
								LOG(L_ERR,"[LIVEDB 2][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				LOG(L_ERR,"[dbconnection]Both livedb down!\n");
				return -1;
		}else if(wchdb == WARE){
				for(i=0;i<3;i++){
						db_wh_handle = getconnection(db_wh_handle,whhost, whuser, whpwd, whdb);
						if(db_wh_handle == NULL){
								LOG(L_ERR,"[WAREDB 1][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				for(i=0;i<3;i++){
						db_wh_handle = getconnection(db_wh_handle,whhost1, whuser1, whpwd1, whdb1);
						if(db_wh_handle == NULL){
								LOG(L_ERR,"[WAREDB 2][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				LOG(L_ERR,"[dbconnection]Both wharehousedb down!\n");
				return -1;
		}
		else if(wchdb == ACCACTIVE){
			 for(i=0;i<3;i++){
				 db_acc_active = getconnection(db_acc_active,_cAccActiveDbHost, _cAccActiveDbUser, _cAccActiveDbPass, _cAccActiveDbName);
				 if(db_acc_active == NULL){
					 LOG(L_ERR,"[ACCACTIVEDB][dbconnection]%d time: Connection failed with: '%s'!\n",i,_cAccActiveDbHost);
					 continue;
				 }
				 return 1;
			 }
			 for(i=0;i<3;i++){
				 db_acc_active = getconnection(db_acc_active,_cAccActiveOneDbHost, _cAccActiveOneDbUser, _cAccActiveOneDbPass, _cAccActiveOneDbName);
				 if(db_acc_active == NULL){
					 LOG(L_ERR,"[ACCACTIVEDB 1][dbconnection]%d time: failed with: '%s'!\n",i,_cAccActiveOneDbHost);
					 continue;
				 }
				 return 1;
			 }
			 LOG(L_ERR,"[ACCACTIVEDB1]Both acc active db down!\n");
			 return -1;
		}
		else if(wchdb == WSPARTNERS){
				for(i=0;i<3;i++){
						db_wspartners = getconnection(db_wspartners,cWsPartnersHost,cWsPartnerUser,cWsPartnerPwd,cWsPartnerDB);
						if(db_wspartners == NULL){
								LOG(L_ERR,"[WSPARTNERS][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				for(i=0;i<3;i++){
						db_wspartners = getconnection(db_wspartners,cWsPartnersHost1,cWsPartnerUser1,cWsPartnerPwd1,cWsPartnerDB1);
						if(db_wspartners == NULL){
								LOG(L_ERR,"[WSPARTNERS 1][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				LOG(L_ERR,"[dbconnection]Both WSPARTNERS DB Down!\n");
				return -1;
		} else if(wchdb == CONTACTSDB){
				for(i=0;i<3;i++){
						db_contact_handle = getconnection(db_contact_handle, cContactsDBHost, cContactsDBUser, cContactsDBPwd, cContactsDB ) ;
						if(db_contact_handle == NULL){
								LOG(L_ERR,"[CONTACTSDB1][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				for(i=0;i<3;i++){
						db_contact_handle = getconnection(db_contact_handle, cContactsDBHost1, cContactsDBUser1, cContactsDBPwd1, cContactsDB1) ;
						if(db_contact_handle == NULL){
								LOG(L_ERR,"[CONTACTSDB2][dbconnection]%d time failed!\n",i);
								continue;
						}
						return 1;
				}
				LOG(L_ERR,"[dbconnection] Both CONTACTSDB DB Down!\n");
				return -1;
		}
		return -1;
}

int ProcessConfiguration(char  * filename)
{
        property abc;
        char *ptr = NULL;

        init_property (filename, &abc);
        abc.parseproperty(&abc);

        ptr = abc.getsetvalue(&abc, "wsfifo", "name");
        if(ptr==NULL){
                return 0;
        }
        strncpy(wsdb_fifo_name,ptr,sizeof(wsdb_fifo_name)-1);
        ptr = abc.getsetvalue(&abc, "wsfifo", "count");
        if(ptr == NULL){
                return 0;
        }
        wsdb_fifo_count = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo", "mysqlerror");
		if(ptr == NULL){
			return 0;
		}
		strncpy(errnumber,ptr,sizeof(errnumber)-1);

		ptr = abc.getsetvalue(&abc, "wsfifo", "retrymysqlerror");
		if(ptr == NULL){
			return 0;
		}

		strncpy(retryerrnumber,ptr,sizeof(retryerrnumber)-1);

		ptr = abc.getsetvalue(&abc, "wsfifo", "queryretrycount");
		if(ptr == NULL){
			return 0;
		}
	
		queryretrycount = atoi(ptr);
	
		/*fetching missedcall_sleep time from configuration file */
		ptr = abc.getsetvalue(&abc, "wsfifo", "missedcall_sleep");
        if(ptr == NULL){
			LOG(L_ERR,"[ProcessConfiguration] NO missedcall_sleep value fetched !!!!\n");
           	return 0;
        }

        missedcall_sleep = atoi(ptr);	

		ptr = abc.getsetvalue(&abc, "wsfifo", "missedcall_querytime");
        if(ptr == NULL){
                LOG(L_ERR,"[ProcessConfiguration] NO missedcall_querytime value fetched !!!!\n");
                return 0;
        }

        missedcall_querytime = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo", "terminating_useragent");
        if(ptr == NULL){
                LOG(L_ERR,"[ProcessConfiguration] NO terminating_useragent value fetched !!!!\n");
                return 0;
        }
		memset(user_agent,0x00,sizeof(user_agent));
        strncpy(user_agent,ptr,sizeof(user_agent)-1);

		ptr = abc.getsetvalue(&abc, "wsfifo", "ws_ctccall_flag");
        if(ptr == NULL){
                LOG(L_ERR,"[ProcessConfiguration] NO ws_ctccall_flag value fetched !!!!\n");
                return 0;
        }

        ws_ctccall_flag = atoi(ptr);

        ptr = abc.getsetvalue(&abc, "live", "host");
        if(ptr==NULL){
                return 0;
        }
        strncpy(livehost,ptr,sizeof(livehost)-1);

        ptr = abc.getsetvalue(&abc, "live", "user");
        if(ptr == NULL){
                return 0;
        }
        strncpy(liveuser,ptr,sizeof(liveuser)-1);

        ptr = abc.getsetvalue(&abc, "live", "pwd");
        if(ptr == NULL){
                return 0;
        }
        strncpy(livepwd,ptr,sizeof(livepwd)-1);

        ptr = abc.getsetvalue(&abc, "live", "dbname");
        if(ptr == NULL){
                return 0;
        }
        strncpy(livedb, ptr,sizeof(livedb)-1);

        ptr = abc.getsetvalue(&abc, "warehouse", "host");
 		
		if(ptr == NULL){
                return 0;
        }
        strncpy(whhost,ptr,sizeof(whhost)-1);

        ptr = abc.getsetvalue(&abc, "warehouse", "user");
        if(ptr == NULL){
                return 0;
        }
        strncpy(whuser,ptr,sizeof(whuser)-1);

        ptr = abc.getsetvalue(&abc, "warehouse", "pwd");
        if(ptr == NULL){
                return 0;
        }
        strncpy(whpwd,ptr,sizeof(whpwd)-1);

        ptr = abc.getsetvalue(&abc, "warehouse", "dbname");
        if(ptr == NULL){
                return 0;
        }
        strncpy(whdb,ptr,sizeof(whdb)-1);

        ptr = abc.getsetvalue(&abc, "live1", "host");
        if(ptr==NULL){
                return 0;
        }
        strncpy(livehost1,ptr,sizeof(livehost1)-1);

        ptr = abc.getsetvalue(&abc, "live1", "user");
        if(ptr == NULL){
                return 0;
        }
        strncpy(liveuser1,ptr,sizeof(liveuser1)-1);

        ptr = abc.getsetvalue(&abc, "live1", "pwd");
        if(ptr == NULL){
                return 0;
        }
        strncpy(livepwd1,ptr,sizeof(livepwd1)-1);

        ptr = abc.getsetvalue(&abc, "live1", "dbname");
        if(ptr == NULL){
                return 0;
        }
        strncpy(livedb1, ptr,sizeof(livedb1)-1);

        ptr = abc.getsetvalue(&abc, "warehouse1", "host");
 		
		if(ptr == NULL){
                return 0;
        }
        strncpy(whhost1,ptr,sizeof(whhost1)-1);

        ptr = abc.getsetvalue(&abc, "warehouse1", "user");
        if(ptr == NULL){
                return 0;
        }
        strncpy(whuser1,ptr,sizeof(whuser1)-1);

        ptr = abc.getsetvalue(&abc, "warehouse1", "pwd");
        if(ptr == NULL){
                return 0;
        }
        strncpy(whpwd1,ptr,sizeof(whpwd1)-1);

        ptr = abc.getsetvalue(&abc, "warehouse1", "dbname");
        if(ptr == NULL){
                return 0;
        }
        strncpy(whdb1,ptr,sizeof(whdb1)-1);
		
		// Acc Active Db Changes
		
		ptr = abc.getsetvalue(&abc, "activetransactions", "host");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveDbHost,ptr,sizeof(_cAccActiveDbHost)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions", "user");
		if(ptr == NULL){
			return 0;
		}
	   	strncpy(_cAccActiveDbUser,ptr,sizeof(_cAccActiveDbUser)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions", "pwd");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveDbPass,ptr,sizeof(_cAccActiveDbPass)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions", "dbname");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveDbName,ptr,sizeof(_cAccActiveDbName)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions1", "host");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveOneDbHost,ptr,sizeof(_cAccActiveOneDbHost)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions1", "user");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveOneDbUser,ptr,sizeof(_cAccActiveOneDbUser)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions1", "pwd");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveOneDbPass,ptr,sizeof(_cAccActiveOneDbPass)-1);

		ptr = abc.getsetvalue(&abc, "activetransactions1", "dbname");
		if(ptr == NULL){
			return 0;
		}
		strncpy(_cAccActiveOneDbName,ptr,sizeof(_cAccActiveOneDbName)-1);

		// Acc Active Db Changes End

		ptr = abc.getsetvalue(&abc, "wsfifo", "who");
        if(ptr == NULL){
                return 0;
        }
        strncpy(who,ptr,sizeof(who)-1);

		ptr = abc.getsetvalue(&abc, "wsfifo", "omsserver");
		if(ptr == NULL){
		   return 0;
		}
		strncpy(omsserver,ptr,sizeof(omsserver)-1);

		ptr = abc.getsetvalue(&abc, "wsfifo", "omsserver1");
		if(ptr == NULL){
		   return 0;
		}
		strncpy(omsserver1,ptr,sizeof(omsserver1)-1);

		ptr = abc.getsetvalue(&abc, "wsfifo", "pulserate");
		if(ptr == NULL){
			return 0;
		}
		PulseRate = atof(ptr);
		ptr = abc.getsetvalue(&abc, "wsfifo", "intllookupflag");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING INTLLOOKUPFLAG FAILED!!!!!\n");
		    return 0;
		}
		intllookupflag = atoi(ptr);
		/*----------- Added for VF --------*/
		ptr = abc.getsetvalue(&abc, "wsfifo", "singlecdrtable");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING singlecdrtable FAILED!!!!!\n");
			return 0;
		}
		singlecdrtable = atoi(ptr);
		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo", "internalbilling");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING internalbilling is failed?\n");
		    return 0;
		}
		internalbilling = atoi(ptr);
		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo","keepactivecallrecord");
		if(ptr == NULL) {
			LOG(L_ERR,"GETTING keepactivecallrecords FAILED!!!!!\n");
			return 0;
		}
		keepactivecallrecord = atoi(ptr);
		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo","serviceid");
		if(ptr == NULL) {
			LOG(L_ERR,"GETTING ServiceId FAILED!!!!!\n");
			return 0;
		}
		nServiceid = atoi(ptr);
		LOG(L_ERR, "nServiceid is '%d' \n", nServiceid);

		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo","datacenter_id");
		if(ptr == NULL) {
			LOG(L_ERR,"GETTING datacenter_id FAILED!!!!!\n");
			return 0;
		}
		nDataCenterID  = atoi(ptr);
		LOG(L_ERR, "nDataCenterID is '%d' \n", nDataCenterID);

		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo","fwd_delay_time");
		if(ptr == NULL) {
			LOG(L_ERR,"GETTING fwd_delay_time FAILED!!!!!\n");
			return 0;
		}
		fwd_delay_time  = atoi(ptr);
		LOG(L_ERR, "fwd_delay_time is '%d' \n", fwd_delay_time);

		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo", "servertz");
		if(ptr == NULL){
			LOG(L_ERR,"Getting servertz Failed !!!!!\n");
			return 0;
		}
		memset(servertz,0,sizeof(servertz));
		strncpy(servertz,ptr,sizeof(servertz)-1);

		/*----------------------------------*/
		ptr = abc.getsetvalue(&abc, "wsfifo","Reporting_flag");
        if(ptr == NULL) {
            LOG(L_ERR,"Getting Reporting_flag Failed !!!!!\n");
            return 0;
        }
		Reporting_flag = atoi(ptr);
        LOG(L_ERR, "Reporting_flag is '%d' \n", Reporting_flag);

		/* Usage Billing Changes */
		ptr = abc.getsetvalue(&abc, "wsfifo", "Commonseat");
		if(ptr == NULL){
			LOG(L_ERR,"Getting Commonseat Failed !!!!!\n");
			return 0;
		}
		Commonseat = atoi(ptr);
	
		ptr = abc.getsetvalue(&abc, "wsfifo", "Virtual_Seat");
		if(ptr == NULL){
			LOG(L_ERR,"Getting Virtual_Seat Failed !!!!!\n");
			return 0;
		}
		Virtual_Seat = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo", "Business_Voice_Basic");
		if(ptr == NULL){
			LOG(L_ERR,"Getting Business_Voice_Basic Failed !!!!!\n");
			return 0;
		}
		Business_Voice_Basic = atoi(ptr);
		/* Usage Billing changes End */
		
		/* Reading template table for creating monthwise new table for inserting only INT/PSTN call records from all accounts-- 08-08-2018 */
		ptr = abc.getsetvalue(&abc, "wsfifo","IntTemplateTable");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting IntTemplateTable Failed !!!!!\n");
			return 0;
		}
		memset(cTemplate_Table_For_LCR,0,sizeof(cTemplate_Table_For_LCR));
		strncpy(cTemplate_Table_For_LCR,ptr,sizeof(cTemplate_Table_For_LCR)-1);

		/* Reading prefix of INT/PSTN Table--4-10-2018 */
		ptr = abc.getsetvalue(&abc, "wsfifo","IntTablePrefix");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting IntPstTable Failed !!!!!\n");
			return 0;
		}
		memset(cPrefix_IntPstTable,0,sizeof(cPrefix_IntPstTable));
		strncpy(cPrefix_IntPstTable,ptr,sizeof(cPrefix_IntPstTable)-1);
		/* END */

		ptr = abc.getsetvalue(&abc, "wsfifo","LcrCallType");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting LcrCallType Failed !!!!!\n");
			return 0;
		}
		LcrCallType = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo","nRoutingPlanEXT_calltype");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting nRoutingPlanEXT_calltype Failed !!!!!\n");
			return 0;
		}
		nRoutingPlanEXT_CallType = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo","TransferCallType");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting TransferCallType Failed !!!!!\n");
			return 0;
		}
		nXferCallType = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo","CancelAnotherCalltype");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting CancelAnotherCalltype Failed !!!!!\n");
			return 0;
		}
		nCancelAnotherCallType = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo","enable_sleep_patch");
		if(ptr == NULL) {
			LOG(L_ERR,"Getting enable_sleep_patch Failed !!!!!\n");
			return 0;
		}
		nEnableSleepPatch = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo", "spam_reject_code");
		if(ptr == NULL){
			return 0;
		}
		nSpamRejectCode = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "wsfifo", "callerid_block_reject_code");
		if(ptr == NULL){
			return 0;
		}
		nCallerIdBlockRejectCode = atoi(ptr);

		/***************Added ULM Variables*************************/
		ptr = abc.getsetvalue(&abc, "presence", "connectionflag");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING connectionflag FAILED!!!!!\n");
			return 0;
		}
		connection_flag = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "presence", "primary_presence_ip");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING primary_presence_ip FAILED!!!!!\n");
			return 0;
		}
		strncpy(primary_presence_ip , ptr , sizeof(primary_presence_ip)-1);

		ptr = abc.getsetvalue(&abc, "presence", "primary_presence_port");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING primary_presence_port FAILED!!!!!\n");
			return 0;
		}
		primary_presence_port = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "presence", "secondary_presence_ip");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING secondary_presence_ip FAILED!!!!!\n");
			return 0;
		}
		strncpy(secondary_presence_ip , ptr , sizeof(secondary_presence_ip)-1);

		ptr = abc.getsetvalue(&abc, "presence", "secondary_presence_port");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING secondary_presence_port FAILED!!!!!\n");
			return 0;
		}
		secondary_presence_port = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "presence", "ping_interval");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING ping_interval FAILED!!!!!\n");
			return 0;
		}
		pt_timer = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "presence", "failure_ping_interval");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING failure_ping_interval FAILED!!!!!\n");
			return 0;
		}
		failed_pt_timer = atoi(ptr);

		ptr = abc.getsetvalue(&abc, "presence", "nonblockmode");
		if(ptr == NULL){
			LOG(L_ERR,"GETTING nonblockmode FAILED!!!!!\n");
			return 0;
		}
		_nonblockmode = atoi(ptr);

	/* Geeting wspartners DB details */
	ptr = abc.getsetvalue(&abc, "wspartners", "host");
	if(ptr == NULL){
		return 0;
	}
	strncpy(cWsPartnersHost,ptr,sizeof(cWsPartnersHost)-1);

	ptr = abc.getsetvalue(&abc, "wspartners", "user");
	if(ptr == NULL){
		return 0;
	}
	strncpy(cWsPartnerUser,ptr,sizeof(cWsPartnerUser)-1);
	
	ptr = abc.getsetvalue(&abc, "wspartners", "pwd");
	if(ptr == NULL){
		return 0;
	}
	strncpy(cWsPartnerPwd,ptr,sizeof(cWsPartnerPwd)-1);

	ptr = abc.getsetvalue(&abc, "wspartners", "dbname");
	if(ptr == NULL){
		return 0;
	}
	strncpy(cWsPartnerDB,ptr,sizeof(cWsPartnerDB)-1);
	
	/* Geeting Alternate wspartners DB details */
	ptr = abc.getsetvalue(&abc, "wspartners1", "host");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cWsPartnersHost1,ptr,sizeof(cWsPartnersHost1)-1);

	ptr = abc.getsetvalue(&abc, "wspartners1", "user");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cWsPartnerUser1,ptr,sizeof(cWsPartnerUser1)-1);

	ptr = abc.getsetvalue(&abc, "wspartners1", "pwd");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cWsPartnerPwd1,ptr,sizeof(cWsPartnerPwd1)-1);

	ptr = abc.getsetvalue(&abc, "wspartners1", "dbname");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cWsPartnerDB1,ptr,sizeof(cWsPartnerDB1)-1);

	/* Geeting CONTACTS DB details */
	ptr = abc.getsetvalue(&abc, "contacts", "host");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDBHost, ptr, sizeof(cContactsDBHost)-1);

	ptr = abc.getsetvalue(&abc, "contacts", "user");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDBUser, ptr, sizeof(cContactsDBUser)-1);

	ptr = abc.getsetvalue(&abc, "contacts", "pwd");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDBPwd, ptr, sizeof(cContactsDBPwd)-1);

	ptr = abc.getsetvalue(&abc, "contacts", "dbname");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDB, ptr, sizeof(cContactsDB)-1);
	
	/* Geeting CONTACTS Alternate DB details */
	ptr = abc.getsetvalue(&abc, "contacts1", "host1");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDBHost1, ptr, sizeof(cContactsDBHost1)-1);

	ptr = abc.getsetvalue(&abc, "contacts1", "user1");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDBUser1, ptr, sizeof(cContactsDBUser1)-1);

	ptr = abc.getsetvalue(&abc, "contacts1", "pwd1");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDBPwd1, ptr, sizeof(cContactsDBPwd1)-1);

	ptr = abc.getsetvalue(&abc, "contacts1", "dbname1");
	if(ptr == NULL){
			return 0;
	}
	strncpy(cContactsDB1, ptr, sizeof(cContactsDB1)-1);

	/* Added for stir-shaken */
	ptr = abc.getsetvalue(&abc, "wsfifo", "stir_name");
	if(ptr==NULL){
		LOG(L_ERR,"Getting stir_name Failed !!!!!\n");
		return 0;
	}
	strncpy(stir_fifo_name,ptr,sizeof(stir_fifo_name)-1);

	ptr = abc.getsetvalue(&abc, "wsfifo", "stir_count");
	if(ptr == NULL){
		LOG(L_ERR,"Getting stir_count Failed !!!!!\n");
		return 0;
	}
	stir_fifo_count = atoi(ptr);

	/* Reading prefix-table-name from configuration for inserting end-point cdr records*/
	ptr = abc.getsetvalue(&abc, "wsfifo", "endpoint_cdr_prefix");
	if(ptr == NULL){
			LOG(L_ERR,"Getting endpoint_cdr_prefix Failed !!!!!\n");
			return 0;
	}
	memset(cEndPonitCdrPrefix,0,sizeof(cEndPonitCdrPrefix));
	strncpy(cEndPonitCdrPrefix,ptr,sizeof(cEndPonitCdrPrefix)-1);

	/* Reading Template table name */
	ptr = abc.getsetvalue(&abc, "wsfifo", "endpoint_cdr_template_name");
	if(ptr == NULL){
			LOG(L_ERR,"Getting  endpoint_cdr_template_name Failed !!!!!\n");
			return 0;
	}
	memset(cEndPonitCdrTemplate,0,sizeof(cEndPonitCdrTemplate));
	strncpy(cEndPonitCdrTemplate,ptr,sizeof(cEndPonitCdrTemplate)-1);


	abc.destroy(&abc);
        return 1;
}

int readfifo(int childno)
{
	int len = 0;
	int wsfifo_fd;
	char outagefile[64]="",*ptr = NULL,hm_buf[64] = "";
		
	memset(outagefile, 0 , sizeof(outagefile));
	snprintf(outagefile,sizeof(outagefile)-1,"/home/wsadmin/wsmsicp/outage-%d-%d",getppid(),getpid());

	if ((outfd = open(outagefile,O_CREAT|O_WRONLY,0777)) <=0 ){
			LOG(L_ERR,"Error opening outage file\n");
			return -1 ;
	}

	/*This is temporarly logic. if any suggest me -- tulsi*/
	/*1-livepbx 2-wharehousepbx*/
	dbconnection(LIVE);
	dbconnection(WARE);
	dbconnection(ACCACTIVE);        // Added for Acc active DB
	dbconnection(WSPARTNERS); 	// Added for wspartners DB
	dbconnection(CONTACTSDB); 	// Added for wspartners DB


#if 0
	db_live_handle = mysql_init(NULL);
	if ( db_live_handle == NULL){
		LOG(L_ERR,"ERROR live database initiazation failed:%s\n",mysql_error(db_live_handle));
		return -1;
	}
	
	if(!mysql_real_connect (db_live_handle, livehost, liveuser, livepwd, livedb, 0, NULL, 0)) {
		LOG(L_ERR,"Error connecting to live Database :%s\n",mysql_error(db_live_handle));
		return -1;
	}

	db_wh_handle = mysql_init(NULL);
	if ( db_wh_handle == NULL){
		LOG(L_ERR,"ERROR warehouse database initiazation failed:%s\n",mysql_error(db_live_handle));
		return -1;
	}
	
	if(!mysql_real_connect (db_wh_handle, whhost, whuser, whpwd, whdb, 0, NULL, 0)) {
		LOG(L_ERR,"Error connecting to warehouse Database :%s\n",mysql_error(db_wh_handle));
		return -1;
	}
#endif

	if ( (wsfifo_fd = open(wsdb_fifo_name , O_RDONLY, 0777) ) <= 0) {
		LOG( L_ERR, "Error opening fifo for reading ERROR:%s\n",strerror(errno));
		mysql_close(db_live_handle);
		mysql_close(db_wh_handle);
		mysql_close(db_acc_active);
		mysql_close(db_wspartners);
		mysql_close(db_contact_handle);
		return -1;
	}
	
	while(1) {
		memset(&wsacc_call, 0 ,sizeof(struct wsacc_fifo_call_details) );
		if ((len = read(wsfifo_fd, (char*)&wsacc_call , sizeof(struct wsacc_fifo_call_details))) < 0) {
			LOG( L_ERR, "Error reading from fifo:%s\n",strerror(errno));
			break;
		}

		/*Added condition to avoid billing for nested hunt group uri's --Anusha */
		if(wsacc_call.method != RACC_CANCELLED && !strncmp(wsacc_call.uri , "fgrp-",5)) {
			continue;
		}
		if(len > 0){
			if(wsacc_call.method == RACC_HOLD && ((ptr = strstr(wsacc_call.contact,"wschm;")))) {
				strncpy(hm_buf,ptr,sizeof(hm_buf)-1);
				*ptr = '\0';
			}
			/*Added if condition to fix the billing issue incase of IVR calls(My SQL Error : 1146) & checking for 911 calls to get siteid & pbxid*/
			if(wsacc_call.uri && strncmp(wsacc_call.uri , "911_" , 4) && strncmp(wsacc_call.uri ,"sip:did-",8)) {
				ptr = strchr(wsacc_call.uri , '_');
				if(ptr) {
					*ptr = '\0';
				}
			}
			racc_fifo_db_request(hm_buf);
		}
	}
	mysql_close(db_live_handle);
	mysql_close(db_wh_handle);
	mysql_close(db_acc_active);
	mysql_close(db_wspartners);
	mysql_close(db_contact_handle);
	LOG( L_ERR, "ERROR: fiforeader is failed %d and  closed the mysql connections\n",childno);
	return 0;
}

int stir_readfifo(int childno)
{
	int len = 0, stir_fifo_fd, res = 0;
	char outagefile[64]="",*dstip = NULL , *ptr = NULL,hm_buf[64] = "";

	memset(outagefile, 0 , sizeof(outagefile));
	snprintf(outagefile,sizeof(outagefile)-1,"/home/wsadmin/wsmsicp/outage-%d-%d",getppid(),getpid());

	if ((outfd = open(outagefile,O_CREAT|O_WRONLY,0777)) <=0 ){
		LOG(L_ERR,"Error opening outage file\n");
		return -1 ;
	}

	/*1-for livepbx 2-for wharehousepbx*/
	dbconnection(1);
	dbconnection(2);

	if ( (stir_fifo_fd = open(stir_fifo_name , O_RDONLY, 0777) ) <= 0) {
		LOG( L_ERR, "Error opening fifo for reading ERROR:%s\n",strerror(errno));
		res = -1;
		goto out;
	}

	while(1) {
		memset(&wsacc_stir, 0 ,sizeof(struct wsacc_fifo_stir_details) );
		if ((len = read(stir_fifo_fd, (char*)&wsacc_stir , sizeof(struct wsacc_fifo_stir_details))) < 0) {
			LOG( L_ERR, "Error reading from fifo:%s\n",strerror(errno));
			break;
		}
		if(len > 0){
			stir_racc_fifo_db_request();
		}
	}

out:
	close(outfd);
	close(stir_fifo_fd);
	mysql_close(db_live_handle);
	mysql_close(db_wh_handle);
	LOG( L_ERR, "ERROR: fiforeader is failed %d and  closed the mysql connections\n",childno);
	return res;
}


/*------------ULM Changes---------------*/
void *start_presence_connection_pinger(void *ptr)
{
	int ct_time = 0,diff_time = 0,nRet = 0;
        char xmlprotobuf[256]="";
	char cBuf[256]="";
	time_t tLastPingTime = 0, tFailedPingTime = 0;
	
	if(connection_flag != 2){
	    // ULM Notification disabled
	    LOG(L_ERR , "[start_presence_connection_pinger] ULM Notification is disabled\n");
	    return NULL;
	}

    while(1){

		if(!_nIsRunning) {
			if(tFailedPingTime == 0 || time(NULL) - tFailedPingTime > failed_pt_timer){
                                if(server_sockfd > 0)
                                        close(server_sockfd);
				GetReConnection();
				tFailedPingTime = time(NULL);
			}
			usleep(1000);	
			continue;
		}

		if(tLastPingTime == 0 || time(NULL) - tLastPingTime > pt_timer)
		{
	    		memset(xmlprotobuf,0x00,sizeof(xmlprotobuf));
	    		snprintf(xmlprotobuf , sizeof(xmlprotobuf)-1 , XML_PINGPROTOCOL , who , "\0");
	    		send_to_server(xmlprotobuf,strlen(xmlprotobuf)+1);
			tLastPingTime = time(NULL);
		}

		memset(cBuf , 0 , sizeof(cBuf));
                nRet = ReceiveData(server_sockfd , cBuf , sizeof(cBuf) -1);
		if(nRet > 0)
		{
			_nLastPingResponse = time(NULL);
		}else if(nRet == 0)
		{
			//Got No Data
		}else
		{
			 _nIsRunning = 0;
			close(server_sockfd);
		}

	    ct_time = time(NULL);
	    diff_time = ct_time - _nLastPingResponse;
	    if(diff_time > pt_timer * 2 && _nIsRunning == 1)
		{
	       LOG(L_ERR,"[start_engine_pinger] ping response is delayed so connectin secondary ct_time :%d _nLastPingResponse:%d ping timer:%d diff:%d , connectionflag:'%d'\n",ct_time,_nLastPingResponse,pt_timer,diff_time ,connectionflag);
	       _nIsRunning = 0;
	       close(server_sockfd);
	       continue;
	    }
		else if(!_nIsRunning)
		{
			_nLastPingResponse = time(NULL);
		}

		usleep(1000);
	}
	return NULL;
}	

int ReceiveData(int nSockfd , char *cpData , int nSize)
{
	 int nRet = 0;
     struct pollfd fds[1];

	 memset(&fds , 0 , sizeof(fds));
	 fds[0].fd = nSockfd;
     fds[0].events = POLLIN | POLLERR | POLLHUP;
     fds[0].revents = 0;

	 nRet = poll(fds , 1 , 100);
     if(nRet < 0)
	 {
	 	if(errno == EINTR)
		{
			LOG(L_ERR, "Polling not done for this\n");	
			return -1;
		}
	 }
	 if(fds[0].revents & POLLIN || fds[0].revents & POLLERR)
	 {
	 	nRet = recv(nSockfd , cpData , nSize , MSG_DONTWAIT);
		if(nRet <= 0)
		{
			LOG(L_ERR , "[ReceiveData] Data Not Received here...some problem occured , socket fd : %d\n" , nSockfd);
			return -1;
		}
		return nRet;
	 }
	 else
	 {
	 	return 0;
	 }
	 return 0;
}

/* To set the Socket into NON_BLOCKING mode*/
int Setting_nonblocking_mode(int fd)
{
	 int flags;
	 if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
	      flags = 0;
	 return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


int ConnectToPresence(const char *serverip,int serverport)
{
	int nRet = 0;

    if(!serverip){
	    LOG(L_ERR,"[ConnectToPresence] server IP address is NULL\n");
	    return -1;
	}
	if(connection_flag != 2){
	    // ULM Notification disabled
	    LOG(L_ERR , "[ConnectToPresence] ULM Notification is disabled\n");
	    return -1;
	}
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);  // TCP connection
	if(server_sockfd <= 0){
	    LOG(L_ERR,"[ConnectToPresence]Socket creation is failed\n");
	    return -1;
	}
	addr.sin_addr.s_addr = inet_addr(serverip);
	addr.sin_port = htons(serverport);
	addr.sin_family = AF_INET;
	
	if(connect(server_sockfd,(struct sockaddr *)&addr,sizeof(addr))<0)     // connection part of TCP client
	{
	     LOG(L_ERR , "[ConnectToPresence]Connect for the socket failed\n");
	     return -1;
	}
	/*SETING THE SOCKET INTO NON-BLOCKING MODE*/
	if(_nonblockmode == 1){
	     LOG(L_ERR,"[ConnectToPresence]Setting non block to fd : '%d'\n",server_sockfd);
	     Setting_nonblocking_mode(server_sockfd);
	}
	_nLastPingResponse = time(NULL);
	_nIsRunning = 1;
	/*To intimate the server to which client it is connected */
	memset(cXMLBuf , 0 , sizeof(cXMLBuf));
	snprintf(cXMLBuf , sizeof(cXMLBuf) , XML_LOGINPROTOCOL , who , "\0");
	write(server_sockfd , cXMLBuf , strlen(cXMLBuf)+1);
	
	return 0;
}

int send_to_server(const char *buffer,int length)
{
	const char *serverip = NULL;
    int serverport=8892;
    int nRet = 0;
    if(!buffer){
         LOG(L_ERR,"[send_to_server] buffer is empty \n");
         return -1;
    }
    if(connection_flag != 2){
        LOG(L_ERR,"[send_to_server] Presence notifying is not enabled\n");
        return -1;
    }
    if((_nIsRunning == 1) && (send(server_sockfd , buffer , length , MSG_NOSIGNAL) == -1)){
        _nIsRunning = 0;
        close(server_sockfd); // Added by murali to fix the too may socket's issue. Need to add Read Handling
    }
    return 0;
}

int GetReConnection(void)
{
	const char *serverip = NULL;
    	int serverport=8892;

	if(connectionflag == PRIMARY_CONNECTION){
 	       serverip = primary_presence_ip;
	       serverport = primary_presence_port;
	       connectionflag = PRIMARY_CONNECTION;
	    }else{
	       serverip = secondary_presence_ip;
	       serverport = secondary_presence_port;
	       connectionflag = SECONDARY_CONNECTION;
	}
	if(ConnectToPresence(serverip,serverport)){
	       LOG(L_ERR,"[send_to_presence]Re-connection to the existing connection got failed cause:%s\n",strerror(errno));
	       if(connectionflag == PRIMARY_CONNECTION){
	          serverip = secondary_presence_ip;
	          serverport = secondary_presence_port;
	          connectionflag = SECONDARY_CONNECTION;
	       }else{
	          serverip = primary_presence_ip;
	          serverport = primary_presence_port;
	          connectionflag = PRIMARY_CONNECTION;
	       }
	       if(ConnectToPresence(serverip,serverport)){
	          LOG(L_ERR,"[send_to_presence]Re-connection to alternate server got failed cause:%s\n",strerror(errno));
		  _nIsRunning = 0;
	          return 0;
	       }
	}
	_nIsRunning = 1;
	return 1;
}


int main(int argc , char *argv[])
{
	int i;  
	int pid;
	struct rlimit rl;
	
	rl.rlim_cur = RLIM_INFINITY;
	rl.rlim_max = RLIM_INFINITY;
	setrlimit (RLIMIT_CORE , &rl);
	if(!(ProcessConfiguration("/home/wsadmin/wsmsicp/etc/msicp/msicpfiforeader.conf")))
	{
		LOG(L_WARN,"Check msicpfiforeader.conf file!\n");
		return -1;
	}
	mysqlerr_no();
	retrymysqlerr_no();
	openlog(argv[0], LOG_PID|LOG_CONS, log_facility);
	LOG(L_INFO,"wsfifo: info %s %d  \n", wsdb_fifo_name, wsdb_fifo_count);

	for(i=0 ; i < wsdb_fifo_count ; i++){
		if((pid = fork())< 0){
			LOG(L_ERR,"ERROR createing new %d FIFO process is failed,terminating the process\n",i);
			closelog();
		  	return 0;
		}
		if(pid == 0)
		{
			if(i == wsdb_fifo_count-1)
			{
				/* Calling missed call processing funtion as seperate process --vravi/saidulu */
				if(missedcall_details(i)){
					LOG(L_ERR,"ERROR IN MISSED CALL PROCESS , TERMINATING IT ...\n");
					closelog();
					exit(0);
				}		

			}else{
				if(readfifo(i))
				{
					LOG(L_ERR,"ERROR FIFO reader%d initialization is failed,terminating the process\n",i);
					closelog();
					exit(0);
				}
			}	
		}		
	}

	for(i=0 ; i < stir_fifo_count ; i++){
		if((pid = fork())< 0){
			LOG(L_ERR,"ERROR createing new %d FIFO process is failed,terminating the process\n",i);
			closelog();
			return 0;
		}
		if(pid == 0){
			if(stir_readfifo(i)){
				LOG(L_ERR,"ERROR FIFO reader%d initialization is failed,terminating the process\n",i);
				closelog();
				exit(0);
			}
		}
	}

	closelog();
	return 0;	
}
