#ifndef _WSFIFODB_H_
#define _WSFIFODB_H_

#define QUERY_ACCOUNTNAME " INSERT INTO acc_%s (u_id, from_uri, to_uri, duration, start_time, end_time, calltype, token, r_uri,assignedto,owner,siteid,pbxid,serviceid,callcost,callrate,hold_count,hold_time,ringing_time,ringing_duration,agentid,npa_flag,callrecord,contactip,macid,grouptitle,callername,groupcode,ownerdid,agent_id,sip_callid,dialednumber,route_type,call_type,orgname,poolname,didalias,acc_endtime,Phone_Type,transfer_token,isacdcall,callsource,direction,callerid,hold_stats,zonecode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall,max_hold_time,missed_call_cause,Ivr_Destination,Ivr_Time,AAduration, spam, is_verified, imported_contactid, rec_format)   VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %u, '%s',%d,%d,%d,%d,%d,%f,%f,%d,%d,'%s',%d,'%s',%d,%d,'%s','%s','%s',\"%s\",%d,'%s',%d,'%s','%s',%d,%d,'%s','%s','%s',convert_tz('%s','%s','%s'),%d,%u,%d,%d,%d,\"%s\",'%s','%s',%d,%d,'%s',\"%s\",%d,%d,%d,'%s','%s',%d, %d, %d, %d, %d)"

#define QUERY_CTCACCOUNTNAME " INSERT INTO acc_%s (u_id, from_uri, to_uri, duration, start_time, end_time, calltype, token, r_uri,assignedto,owner,siteid,pbxid,serviceid,callcost,callrate,hold_count,hold_time,ringing_time,ringing_duration,agentid,npa_flag,callrecord,contactip,macid,grouptitle,callername,groupcode,ownerdid,agent_id,sip_callid,dialednumber,route_type,call_type,another_calltype,missed_call_cause,orgname,poolname,didalias,hold_stats,max_hold_time,imported_contactid,rec_format)   VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %u, '%s',%d,%d,%d,%d,%d,%f,%f,%d,%d,'%s',%d,'%s',%d,%d,'%s','%s','%s',\"%s\",%d,'%s',%d,'%s','%s',%d,%d,%d,%d,'%s','%s','%s','%s',%d, %d, %d)"

#define QUERY_RACC_ACCPTED "UPDATE acc_active SET call_status = %d, to_tag = '%s', time2='%s',is_active = 1,contactip = '%s',macid= '%s',ismissed_call = if((is_voicemail = 0),0,ismissed_call), transferred_time=if((is_transfer=1),'%s',transferred_time) WHERE (calltoken = %u OR sip_callid = '%s') AND serviceid = %d AND request_branch='%s' AND time2='0000-00-00 00:00:00'"

#define QUERY_MISSEDCALLS_CDRS "INSERT INTO acc_%s (sip_callid,pbxid,r_uri,from_uri,to_uri,start_time,end_time,missed_call_cause,siteid,serviceid,token,ringing_time,calltype,agentid,usercallstate,ringing_duration,grouptitle,callername,groupcode,owner,ownerdid,agent_id,dialednumber,call_type,orgname,poolname,didalias,acc_endtime,transfer_token,callsource,direction,callerid,duration,another_calltype,Phone_Type,zonecode,callflag_tax,offnet_call,originalcallerid,originalcnam,ispickupcall, spam, is_verified, reject_code, imported_contactid, rec_format) VALUES ('%s',%d,'%s','%s','%s','%s','%s','%d',%d,%d,%u,'%s',%d,'%s',2,%d,'%s',\"%s\",%d,%d,'%s',%d,'%s',%d,'%s','%s','%s',convert_tz('%s','%s','%s'),%u,%d,%d,\"%s\",(CASE WHEN '%s'>0 AND '%s'>0 THEN TIME_TO_SEC(TIMEDIFF('%s', '%s')) ELSE 0 END),%d,%d,'%s',%d,%d,'%s',\"%s\",%d, %d, %d, %d, %d, %d)"

#define QUERY_MISSEDCALLS_DELETE_ACCACTIVE "DELETE FROM acc_active WHERE missedcall_time BETWEEN '%s' AND '%s' AND ismissed_call = 1 AND serviceid = %d AND (is_ctcflag = 0 OR (is_ctcflag = 1 AND is_voicemail = 1 and is_active  != 1))"
	
#define QUERY_MISSEDCALLS_UPDATE_ACCACTIVE "UPDATE acc_active set missedcall_cause = if((missedcall_cause = %d),%d,missedcall_cause) WHERE calltoken = %u and is_active=0 and serviceid = %d"

#define QUERY_MISSEDCALLS_SELECT "SELECT sip_callid,pbxid,request_uri,sip_from,sip_to,min(time1),missedcall_time,missedcall_cause,siteid,token,ringing_time,(CASE WHEN missedcall_time>0 AND ringing_time>0 THEN TIME_TO_SEC(TIMEDIFF(missedcall_time,ringing_time)) ELSE NULL END),calltoken,is_voicemail,agentid,grouptitle,callername,groupcode,dialednumber,owner,ownerdid,agent_id,transfer_token,convert_tz(min(time1),'%s','+00:00'),convert_tz(missedcall_time,'%s','+00:00'),acrossuser,time3,is_direct_call,is_transfer, call_type, sip_rpid,originalcallerid,originalcnam,ispickupcall,sitename,reject_code,iscalleriddisplay,rec_format FROM acc_active WHERE ismissed_call = 1 AND serviceid = %d AND (is_ctcflag = 0 OR (is_ctcflag = 1 AND is_voicemail = 1 and is_active  != 1)) AND missedcall_time < timestampadd(second,%d,now()) group by sip_callid,pbxid order by missedcall_time"

#define QUERY_INSERT_ACCACTIVE_DIGIT "INSERT INTO acc_active (request_uri, sip_callid, request_branch, from_tag, sip_from, sip_to, sip_rpid, time1, call_status, token, old_uri,contact, who ,siteid,pbxid,calltoken,serviceid,agentid,grouptitle,callername,groupcode,dialednumber,owner,ownerdid,agent_id,agent_serviceid,is_ctcflag,dcid,acrossuser,is_direct_call,call_type)  VALUES ('%s','%s','%s','%s','%s','%s','%s','%s', %d, '%s', '%s','%s','%s',%d,%d,%u,%d,'%s','%s',\"%s\",%d,'%s',%d,'%s',%d,%d,%d,%d,'%s',%d,%d)"

#define QUERY_INSERT_ACCACTIVE  "INSERT INTO acc_active(request_uri, sip_callid, request_branch, from_tag,sip_from, sip_to, sip_rpid, time1, call_status, token,contact, who , siteid, pbxid,calltoken,serviceid,agentid,grouptitle,callername,groupcode,dialednumber,owner,ownerdid,agent_id,agent_serviceid,is_ctcflag,dcid,acrossuser,missedcall_cause, call_type, iscalleriddisplay) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s', %d, '%s','%s','%s',%d, %d,%u,%d,'%s','%s',\"%s\",%d,'%s',%d,'%s',%d,%d,%d,%d,'%s','%d','%d', '%d')"

#define INSERT_INTO_ENDPOINT_CDR "INSERT INTO %s (sip_callid,transaction_id , endpointip ,lcontact_ip,transport_type, who, dcid) VALUES ('%s', %s , '%s', '%s','%s','%s', %d)"

#define QUERY_RINGING_UPDATE_ACCACTIVE "UPDATE acc_active SET call_status = %d, ringing_time='%s' WHERE sip_callid='%s' and from_tag='%s' and request_branch='%s' AND ringing_time='0000-00-00 00:00:00' AND who='%s'"

#define QUERY_RINGING_ACCACTIVE_URI "UPDATE acc_active SET call_status = %d, ringing_time='%s' WHERE calltoken = %u AND serviceid= %d AND request_branch = '%s' AND ringing_time='0000-00-00 00:00:00'"

#define QUERY_ESTABLISHED_ACCACTIVE "UPDATE acc_active SET call_status = %d, time2='%s', to_tag='%s',request_branch='%s' WHERE sip_callid = '%s' and from_tag = '%s' AND who='%s'"

#define QUERY_ESTABLISHED_ACCACTIVE_URI  "UPDATE acc_active SET call_status = %d, time2='%s', to_tag='%s', request_branch='%s' WHERE calltoken = %u and serviceid = %d"

#define QUERY_TMPMOVED_ACCACTIVE_CNF  "UPDATE acc_active SET call_status = %d,request_uri = '%s', time1='%s' where calltoken = %u"

//#define QUERY_TMPMOVED_MISSED_UPDATE "UPDATE acc_active SET mruri = '%s',ismissed_call=1,missedcall_time='%s',is_voicemail = 1 WHERE calltoken = %u and is_active = 0 and serviceid = '%d'"
//#define QUERY_TMPMOVED_MISSED_UPDATE "UPDATE acc_active SET mruri = '%s',missedcall_time='%s',is_voicemail = 1 WHERE calltoken = %u and is_active = 0 and serviceid = '%d'"
#define QUERY_TMPMOVED_MISSED_UPDATE "UPDATE acc_active SET mruri = '%s',missedcall_time='%s',is_voicemail = 1 WHERE sip_callid = '%s' and is_active = 0 and serviceid = '%d'"

#define QUERY_TMPMOVED_MISSED_UPDATE_COS "UPDATE acc_active SET mruri = '%s',missedcall_time='%s',is_voicemail = 1,missedcall_cause=%d WHERE sip_callid = '%s' and is_active = 0 and serviceid = '%d'"

#define QUERY_VML_URI_UPDATE "UPDATE acc_active SET mruri = '%s' WHERE calltoken = %u "

#define QUERY_TMPMOVED_DELETE_ACCACTIVE "DELETE FROM acc_active where sip_callid = '%s' AND ismissed_call != 1"

#define QUERY_TERMINATING_UPDATE_ACCACTIVE "UPDATE acc_active SET call_status = %d, time3 = '%s' WHERE sip_callid = '%s' AND (( from_tag = '%s' AND to_tag = '%s' ) OR ( from_tag = '%s' AND to_tag = '%s' )) AND who = '%s' AND time3 ='0000-00-00 00:00:00'"

#define QUERY_TERMINATING_UPDATE_ACCACTIVE_URI "UPDATE acc_active SET call_status = %d, time3 = '%s' WHERE calltoken = %u AND serviceid=%d and sip_callid = '%s' AND (( from_tag = '%s' AND to_tag = '%s' ) OR ( from_tag = '%s' AND to_tag = '%s' )) AND who = '%s' AND time3 ='0000-00-00 00:00:00'"

#define QUERY_ACCEPT_MISSED_DELETE "DELETE FROM acc_active WHERE sip_callid = '%s' AND  request_branch != '%s' AND is_active = 0"

#define QUERY_MISSEDCALL_UPDATE " UPDATE acc_active SET missedcall_cause=(case when missedcall_cause!=14 then %d else 14 end)  WHERE sip_callid = '%s' AND serviceid =%d AND request_branch='%s'"

#define QUERY_REJECTED_DELETE "DELETE FROM acc_active WHERE sip_callid = '%s' AND request_branch='%s' AND serviceid=%d"

#define QUERY_CANCELLED_MISSED_UPDATE "UPDATE acc_active SET ismissed_call=1,missedcall_time='%s',missedcall_cause =(case when missedcall_cause!=14 then  if((missedcall_cause in(%d,%d)),%d,missedcall_cause) else 14 end ) WHERE sip_callid = '%s' AND serviceid =%d"

#define QUERY_HOLD_UPDATE "UPDATE acc_active SET hold_count=%d,hold_time=%d,hold_start_time='%s' WHERE sip_callid = '%s' AND (( from_tag = '%s' AND to_tag = '%s' ) OR ( from_tag = '%s' AND to_tag = '%s' )) AND who = '%s'"

#define QUERY_HOLD_UPDATE_TOKEN	"UPDATE acc_active SET hold_count=%d,hold_time=%d,hold_start_time='%s' WHERE calltoken = %u AND serviceid = %d "

#define QUERY_ACCACTIVE_SELECT_FINAL_DATA "SELECT request_uri, sip_from, sip_to, time2, time3, calltoken, call_status, old_uri, time1,contact,(CASE WHEN time3>0 AND time2>0 THEN TIME_TO_SEC(TIMEDIFF(time3,time2)) ELSE NULL END),hold_count,hold_time,ringing_time,(CASE WHEN time2>0 AND ringing_time>0 THEN TIME_TO_SEC(TIMEDIFF(time2,ringing_time)) ELSE NULL END),hold_start_time,(CASE WHEN time3>0 AND hold_start_time>0 THEN TIME_TO_SEC(TIMEDIFF(time3,hold_start_time)) ELSE NULL END),callrecord,contactip,macid,grouptitle,callername,groupcode,dialednumber,token,siteid,pbxid,owner,ownerdid,agent_id,agent_serviceid,agentid,transfer_token,acrossuser,mruri,hold_stats,is_direct_call,is_transfer,transferred_time,(CASE WHEN time3>0 AND transferred_time>0 THEN TIME_TO_SEC(TIMEDIFF(time3,transferred_time)) ELSE NULL END),transfer_number,call_type,originalcallerid,originalcnam,ispickupcall,max_hold_time,missedcall_cause,Ivr_Destination,Ivr_Time,AAduration,iscalleriddisplay,sip_rpid,rec_format FROM acc_active WHERE sip_callid='%s'  AND (( from_tag ='%s' AND to_tag='%s' ) OR ( from_tag ='%s' AND to_tag='%s' )) AND time2 <> '0000-00-00 00:00:00' AND time3 <> '0000-00-00 00:00:00' AND call_status = 7 AND TRIM(LENGTH(request_uri))<>0  and is_active = 1 AND who='%s'" 

#define QUERY_ACCACTIVE_CTC_SELECT_FINAL_DATA "SELECT request_uri, sip_from, sip_to, time2, time3, calltoken, call_status, old_uri, time1,contact,(CASE WHEN time3>0 AND time2>0 THEN TIME_TO_SEC(TIMEDIFF(time3,time2)) ELSE NULL END),hold_count,hold_time,ringing_time,(CASE WHEN time2>0 AND ringing_time>0 THEN TIME_TO_SEC(TIMEDIFF(time2,ringing_time)) ELSE NULL END),hold_start_time,(CASE WHEN time3>0 AND hold_start_time>0 THEN TIME_TO_SEC(TIMEDIFF(time3,hold_start_time)) ELSE NULL END),callrecord,contactip,macid,grouptitle,callername,groupcode,dialednumber,token,siteid,pbxid,owner,ownerdid,agent_id,agent_serviceid,agentid,mruri,missedcall_cause,hold_stats,max_hold_time,missedcall_cause,Ivr_Destination,Ivr_Time,AAduration,iscalleriddisplay,sip_rpid FROM acc_active WHERE sip_callid='%s'  AND (( from_tag ='%s' AND to_tag='%s' ) OR ( from_tag ='%s' AND to_tag='%s' )) AND time2 <> '0000-00-00 00:00:00' AND time3 <> '0000-00-00 00:00:00' AND call_status = 7 AND TRIM(LENGTH(request_uri))<>0  and is_active = 1 AND who='%s'" 

#define QUERY_ACCACTIVE_SELECT_FINAL_DATA_TOKEN "SELECT request_uri, sip_from, sip_to, time2, time3, calltoken, call_status, old_uri, time1,contact,(CASE WHEN time3>0 AND time2>0 THEN TIME_TO_SEC(TIMEDIFF(time3,time2)) ELSE NULL END),hold_count,hold_time,ringing_time,(CASE WHEN time2>0 AND ringing_time>0 THEN TIME_TO_SEC(TIMEDIFF(time2,ringing_time)) ELSE NULL END),hold_start_time,(CASE WHEN time3>0 AND hold_start_time>0 THEN TIME_TO_SEC(TIMEDIFF(time3,hold_start_time)) ELSE NULL END),callrecord,contactip,macid,grouptitle,callername,groupcode,dialednumber,token,siteid,pbxid,owner,ownerdid,agent_id,agent_serviceid,agentid,transfer_token,acrossuser,mruri,hold_stats,is_direct_call,is_transfer,transferred_time,(CASE WHEN time3>0 AND transferred_time>0 THEN TIME_TO_SEC(TIMEDIFF(time3,transferred_time)) ELSE NULL END),transfer_number,call_type,originalcallerid,originalcnam,ispickupcall,max_hold_time,missedcall_cause,Ivr_Destination,Ivr_Time,AAduration,iscalleriddisplay,sip_rpid,rec_format FROM acc_active WHERE calltoken=%u AND time2 <> '0000-00-00 00:00:00' AND time3 <> '0000-00-00 00:00:00' AND call_status = 7 AND TRIM(LENGTH(request_uri))<>0  and is_active = 1 AND serviceid=%d and sip_callid = '%s'" 

#define QUERY_ACCACTIVE_CTC_SELECT_FINAL_DATA_TOKEN "SELECT request_uri, sip_from, sip_to, time2, time3, calltoken, call_status, old_uri, time1,contact,(CASE WHEN time3>0 AND time2>0 THEN TIME_TO_SEC(TIMEDIFF(time3,time2)) ELSE NULL END),hold_count,hold_time,ringing_time,(CASE WHEN time2>0 AND ringing_time>0 THEN TIME_TO_SEC(TIMEDIFF(time2,ringing_time)) ELSE NULL END),hold_start_time,(CASE WHEN time3>0 AND hold_start_time>0 THEN TIME_TO_SEC(TIMEDIFF(time3,hold_start_time)) ELSE NULL END),callrecord,contactip,macid,grouptitle,callername,groupcode,dialednumber,token,siteid,pbxid,owner,ownerdid,agent_id,agent_serviceid,agentid,mruri,missedcall_cause,hold_stats,max_hold_time,missedcall_cause,Ivr_Destination,Ivr_Time,AAduration,iscalleriddisplay,sip_rpid FROM acc_active WHERE calltoken=%u AND time2 <> '0000-00-00 00:00:00' AND time3 <> '0000-00-00 00:00:00' AND call_status = 7 AND TRIM(LENGTH(request_uri))<>0  and is_active = 1 AND serviceid=%d and sip_callid = '%s'" 

#define QUERY_ACCACTIVE_DELETE " DELETE FROM acc_active WHERE sip_callid = '%s' AND ( from_tag = '%s' OR from_tag = '%s') AND ( to_tag = '%s' OR to_tag = '%s' OR to_tag is NULL OR length(to_tag) <= 0 ) AND serviceid = %d AND (ismissed_call != 1 OR is_ctcflag = 1)" 

#define QUERY_TMPMOVED_MISSED_CALLERSIDETRANSFER_UPDATE "UPDATE acc_active SET ismissed_call=1,missedcall_time='%s' WHERE calltoken = %u and serviceid = %d and is_active =0"

#define AGENTQRYONE "SELECT 1,p.id,p.siteid,p.extensions,'' from pbxusers p ,siteinfo_new s where p.siteid = s.siteid and p.agentid = '%s'"

#define AGENTQRYTWO  "SELECT 1,p.id , p.siteid , p.extensions,'' from pbxusers p , siteinfo_new s  WHERE s.sitename = '%s' AND p.siteid = s.siteid AND p.extensions = '%s' and p.extensions!=0 UNION SELECT 2,p.id , p.siteid ,p.extensions,'' from pbxusers p ,siteinfo_new s ,accountphonenumbers a  where p.siteid = s.siteid AND p.extensions = a.extensions  AND a.siteid=p.siteid AND s.siteid=a.siteid AND s.sitename='%s' AND (a.phnumber='%s' OR a.phnumber = '1%s') UNION  SELECT 3,'',sp.siteid ,sp.code,sp.title from special sp ,siteinfo_new s ,accountphonenumbers a  where sp.siteid = s.siteid AND (sp.code = a.extensions OR sp.extensions = a.extensions) AND a.extensions!=0 AND a.siteid=sp.siteid AND s.siteid=a.siteid  AND s.sitename='%s' and (a.phnumber='%s' OR a.phnumber = '1%s')  UNION  SELECT 4,p.id,sp.siteid ,sp.code,sp.title  FROM special sp ,siteinfo_new s, pbxusers p WHERE sp.siteid = s.siteid AND s.siteid=p.siteid AND s.sitename='%s' AND ( sp.extensions = '%s' or sp.code = '%s' ) and p.agentid='%s' and sp.code!=0 and sp.extensions != 0"

#define QUERYFOREXT "SELECT a.serviceid,a.extensions,a.agentid,p.id,coalesce((select phnumber from accountphonenumbers ap where ap.siteid=a.siteid and ap.extensions=a.extensions order by didvisibility desc limit 1),0)phnumber,a.id FROM agent a,pbxusers p WHERE a.siteid = %d  AND (a.extensions = '%s') AND a.siteid = p.siteid AND a.extensions = p.extensions"

#define QUERYFORAGENTID "SELECT a.serviceid,a.extensions,a.agentid,p.id,coalesce((select phnumber from accountphonenumbers ap where ap.siteid=a.siteid and ap.extensions=a.extensions order by didvisibility desc limit 1),0)phnumber,a.id FROM agent a,pbxusers p WHERE a.siteid = %d AND (a.agentid = '%s') AND a.siteid = p.siteid AND a.extensions = p.extensions"

#define QUERY_GET_GRPTITITLE_DID "SELECT title,code FROM special sp,siteinfo_new s WHERE (extensions = '%s' OR code = '%s') AND sp.siteid = s.siteid AND s.sitename='%s' UNION SELECT sp.title,sp.code  from special sp ,siteinfo_new s ,accountphonenumbers a  where sp.siteid = s.siteid AND (sp.code = a.extensions OR sp.extensions = a.extensions) AND a.extensions!= 0 AND a.siteid=sp.siteid AND s.siteid=a.siteid  AND s.sitename='%s' and (a.phnumber='%s' OR a.phnumber='1%s')"

#define QUERY_GET_GRPTITITLE_TO_ACD "SELECT '',code FROM special sp,siteinfo_new s WHERE title = '%s' AND sp.siteid = s.siteid AND s.sitename='%s' and title != ''"

#define QUERY_GET_POOL_DETAILS "SELECT orgname,poolname,aliasname FROM orgpooldids WHERE siteid = %d AND (phnumber = '%s' OR phnumber = '1%s')"

#define CALLERID_QUERY "SELECT callerid FROM pbxusers p , subscriber s WHERE s.username = '%s' AND p.agentid = s.agentid"

#define IVR_OPR_QUERY "SELECT operator from special s, siteinfo_new si WHERE si.siteid = s.siteid AND code = %s AND si.sitename = '%s' and acctype!=2 and acctype!=4"

#define IVR_CMD_QUERY "SELECT sp.title,sp.code,1 FROM special sp, extendedpbxivrcommand e, siteinfo_new s WHERE sp.siteid = e.siteid AND e.siteid = s.siteid  AND sp.siteid=s.siteid AND  s.sitename='%s' AND e.groupcode = %s AND sp.code = e.extensiongroupcode AND command = %s  AND is_user=0 AND acctype!=2 AND acctype!=4  UNION  SELECT '','',2 from extendedpbxivrcommand e , siteinfo_new si WHERE e.siteid = si.siteid AND si.sitename = '%s' AND groupcode = '%s' AND command = '%s' AND isoperator = 1 AND acctype!=2 AND acctype!=4 UNION SELECT '','',3 FROM pbxusers p,extendedpbxivrcommand e, siteinfo_new s  WHERE e.siteid = s.siteid AND p.siteid = s.siteid AND p.siteid = e.siteid AND s.sitename = '%s' AND e.extensiongroupcode = p.id AND e.groupcode = %s  AND command = %s AND is_user=1 AND acctype!=2 AND acctype!=4"

#define SERVICEID_QUERY "SELECT serviceid FROM agent WHERE agentid='%s' AND siteid='%d'"

#define QUERY_302ACCOUNTNAME " INSERT INTO acc_%s (from_uri, start_time,  calltype, token,sip_callid,Ivr_Destination,grouptitle,groupcode,callername,siteid,end_time,callerid,dialednumber)   VALUES ('%s','%s', %d, %u, '%s','%s','%s',%d,\"%s\",%d,'%s','%s',%d)"

#define QUERY_REDUCTION "INSERT IGNORE INTO reduction_cdrs (token,cdr_table_name,etime,isprocessed,siteid,dcid) VALUES (%u, '%s', '%s', 0, %d,%d)"

#define QUERY_GET_GRPEXT_DID "SELECT a.extensions, a.phnumber, sp.title FROM special sp, accountphonenumbers a WHERE a.extensions = sp.extensions and sp.code = '%d' and sp.title = '%s' and a.siteid = sp.siteid and sp.siteid = '%d'"

#define QUERY_GET_GRPEXT_DID_ONE "SELECT a.extensions, a.phnumber, sp.title FROM special sp, accountphonenumbers a WHERE (a.extensions = sp.extensions OR a.billing_extensions = sp.extensions) and sp.code = '%d' and a.siteid = sp.siteid and sp.siteid = '%d'"

#define UPDATE_VML_ENDTIME "UPDATE acc_active SET time3 = '%s',ismissed_call = 1 where calltoken = %u AND is_voicemail = 1"

#define  QUERY_GET_ACTIVEDETAILS  "SELECT 1 from acc_active where sip_callid = '%s' and serviceid = %d"

#define SELECT_INTERACCOUNT_DETAIL "SELECT 1 FROM acc_%s WHERE DATE(start_time) = '%s' and token = %u and calltype in(2,3,4,9)" 

#define UPDATE_CALLERNAME "UPDATE acc_active SET callername = \"%s\" ,is_initial = 0 WHERE calltoken=%u AND serviceid != %d AND is_initial = 1"

#define UPDATE_PHONE_FORWARD "UPDATE acc_active SET call_status = %d , time2='%s',time3='%s' , is_active = 1 , ismissed_call = if((is_voicemail = 0),0,ismissed_call) WHERE calltoken = %u AND sip_callid = '%s' AND serviceid = %d AND request_branch='%s' AND time2='0000-00-00 00:00:00' AND time3='0000-00-00 00:00:00'"

#define SELECT_SERVICETYPE_QUERY "SELECT sid,is_pooled FROM services WHERE serviceid = %d "

#define SELECT_CALLRATE_PULSERATE_QUERY "SELECT callrate,pulserate FROM calltypes_v1 WHERE calltypecode = %d and usage_rate_sheet_id = %d"

#define SELECT_SID_QUERY "SELECT sid FROM state_codes WHERE code = '%s'"

#define SELECT_CALLERID_QUERY "SELECT callerid FROM pbxusers WHERE agentid = '%s' and siteid = %d"

#define SELECT_ZONECODE_QUERY "SELECT zonecode FROM international_tariff b WHERE SUBSTRING('%s', 1, 8) = b.zonecode OR SUBSTRING('%s', 1, 7) = b.zonecode OR SUBSTRING('%s', 1, 6) = b.zonecode OR SUBSTRING('%s', 1, 5) = b.zonecode OR SUBSTRING('%s', 1, 4) = b.zonecode OR SUBSTRING('%s', 1, 3) = b.zonecode OR SUBSTRING('%s', 1, 2) = b.zonecode ORDER BY  b.zonecode DESC LIMIT 1"

#define CREATE_TABLE_QRY "CREATE TABLE acc_%s LIKE %s"

#define DELETE_LCR_ERROR_RESPONSES_486_QUERY "DELETE FROM acc_active WHERE sip_callid = '%s' AND call_type in(%d, %d)"

#define DELETE_LCR_ERROR_RESPONSES_QUERY "DELETE FROM acc_active WHERE sip_callid = '%s' AND call_type = %d"

#define AGENT_NAME_QUERY "select 1,siteid,agentid,extensions,coalesce((select phnumber from accountphonenumbers p where p.siteid=a.siteid and p.extensions=a.extensions and assignedto=2 order by didvisibility desc limit 1),0)did from agent a where siteid=%d and extensions='%s' union select 2,siteid,title,extensions,coalesce((select phnumber from accountphonenumbers p where p.siteid=s.siteid and (p.extensions=s.code or p.extensions=s.extensions) and assignedto in (1,9) order by didvisibility desc limit 1),0)did from special s where siteid=%d and extensions='%s'"

#define SELECT_USAGE_RATE_SHEET_ID_QUERY "SELECT usage_rate_sheet_id FROM sitepartners WHERE siteid = %d"

#define SELECT_GRP_EXT_QUERY "SELECT s.extensions,s.title FROM special s,siteinfo_new sp WHERE s.code='%s' and sp.siteid=s.siteid and sp.sitename='%s'"

#define QUERY_INSERT_ACCACTIVE_MISSED_CALL  "INSERT INTO acc_active(request_uri, sip_callid, request_branch, from_tag,sip_from, sip_to, sip_rpid, time1, call_status, token,contact, who , siteid, pbxid,calltoken,serviceid,agentid,grouptitle,callername,groupcode,dialednumber,owner,ownerdid,agent_id,agent_serviceid,is_ctcflag,dcid,acrossuser,missedcall_cause, call_type,ismissed_call,time2,is_voicemail,missedcall_time,sitename, reject_code) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s', %d, '%s','%s','%s',%d, %d,%u,%d,'%s','%s',\"%s\",%d,'%s',%d,'%s',%d,%d,%d,%d,'%s','%d','%d',%d,'%s',%d, '%s', '%s',%d )"

#define LCR_UPDATE_QUERY "UPDATE acc_active set ismissed_call = 1 , missedcall_time = now(), missedcall_cause = %d, reject_code = %d WHERE sip_callid = '%s'"

#define QUERY_IMPORTCONTACT_DETAILS " SELECT cc.id FROM c_contacts cc, c_contactowners cco, c_owners co where cc.id=cco.contactid and cco.ownerid=co.id and co.siteid=%d and co.owner IN('%s','%s') and (cc.mobilephone IN ('%s','%s') OR cc.homephone IN ('%s','%s') OR cc.officePhone IN ('%s','%s') OR cc.companyphone IN ('%s','%s')) ORDER BY creationdate DESC LIMIT 1 "

#define GET_AGENTID_QUERY "select ag.agentid from accountphonenumbers ac, agent ag  where ag.siteid = ac.siteid AND ac.extensions = ag.extensions AND ac.siteid = %d and ac.phnumber = '%s'"

#endif 
