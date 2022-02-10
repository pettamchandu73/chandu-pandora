#!/bin/sh

# The main objective of this script is to restart the services when there process(es) are inactive 
# The services will be restarted when the active number of process(es) are less than required number of precess(es)
# If any dependences services are present then there will be restarted along with the main service if there are inactive

. /etc/rc.d/init.d/functions

HOST="10.20.8.132"
SLEEP_TIME="5"
MAX_TIME="55"
NETWORK="PRODUCTION"
FLAG=0
SPL_CASE=0
mail_count=0
PROCESS_FD_LIMIT=8192

MSICP="/home/wsadmin/wsmsicp/sbin/msicp"
MSICPFIFOREADER="/home/wsadmin/wsmsicp/msicpfiforeader/msicpfiforeader"
ICP_CONF="/home/wsadmin/wsmsicp/etc/msicp/msicp.conf"
FIFO_CONF="/home/wsadmin/wsmsicp/etc/msicp/msicpfiforeader.conf"
msicp_exe=/home/wsadmin/wsmsicp/sbin/msicp
prog=msicp
RETVAL=0
OPTIONS="-T -n 10 -M 128"

NOP_ICP=24
NOP_FIFO=11

TO_MAIL="deployment@panterranetworks.com"
#TO_MAIL="deployment@panterranetworks.com,gopinath@panterranetworks.com"
#TO_MAIL="sarath@panterranetworks.com,kkmurthy@panterranetworks.com,harikrishna@panterranetworks.com"
#TO_MAIL="pbxdev@panterranetworks.com,qa@panterranetworks.com"
FROM_MAIL="ws_service_alert@panterranetworks.com"

################################################
##### FUNCTIONS DEFINITION startS FROM HERE#####
################################################

stopaccfifo() {
        echo -n $"Stopping msicpfiforeader"
        killproc msicpfiforeader -HUP
        RETVAL=$?
        return $RETVAL
}

start() {
        echo -n $"Starting $prog: "
        touch /tmp/fgrp_shmdetails
        daemon $msicp_exe $OPTIONS
        RETVAL=$?
        echo
        [ $RETVAL = 0 ] && touch /var/lock/subsys/msicp
        return $RETVAL
}

stop() {
        echo -n $"Stopping $prog: "
        killproc $msicp_exe
        RETVAL=$?
        echo
        [ $RETVAL = 0 ] && rm -f /var/lock/subsys/msicp /var/run/msicp.pid
}

SENDMAIL() # This function call will helps in mailing the reqired data
{
		if [ $SPL_CASE -eq 1 ];then
                subject="$2 Service has been restarted in $NETWORK Network <$HOST> after recovery"
		elif [ $FLAG -eq 1 ];then
                subject="Unable to start $2 service, please verify it in $NETWORK Network <$HOST>"
                echo -e "\n\nWARNING: UNABLE TO START $2 SERVICE. PLEASE VERIFY IT IN $NETWORK NETWORK <$HOST>\n\n" > MSG
        else
                subject="$2 has been restarted in $NETWORK Network <$HOST>"
        fi
        echo -e "$2 Service has been restarted due to following reasons\n\nThe number of active process is $1 \n\nRequired number active process is $3" >> MSG
	echo -e "\n\n Below are the list of processes before restarting the  $2 services\n\n" >> MSG
	echo -e "ps -eaf | grep $2\n" >> MSG
	while read line
        do
                echo "$line" >> MSG
        done < msicp_msg

     body=`cat MSG`
     {
	       echo -e "To: $TO_MAIL"
	       echo -e "From: $FROM_MAIL"
	       echo -e "Subject: $subject"
	       echo -e ""
	       echo -e "$body"
           echo -e ""
     } | /usr/sbin/sendmail -oi -t

    rm -f MSG wsacd_msg	
}

# The following function is called when a particular service is inactive or required number of process(es) are not running
# Send a mail to the adminstrator about the restarting the service 

SERVICES() 
{
	down_flag=0

        count_icp=0
        count_icp=`ps -ef | grep -w $MSICP | grep -v grep | wc -l` # Checks the number of active process(es) running

        count_fifo=0
        count_fifo=`ps -ef | grep -w $MSICPFIFOREADER | grep -v grep | wc -l` # Checks the number of active process(es) running
	if [ "$NOP_FIFO" -gt "0" ] && [ "$count_fifo" -ne "$NOP_FIFO" ] && [ "$NOP_ICP" -gt "0" ] && [ "$count_icp" -ne "$NOP_ICP" ];then
                process="msicp & msicpfiforeader"
		service="wsmsicp"
                req_count=$(($NOP_FIFO+$NOP_ICP))
                count=$(($count_fifo+$count_icp))
                down_flag=1
        elif [ "$NOP_FIFO" -gt "0" ] && [ "$count_fifo" -ne "$NOP_FIFO" ];then
                process="msicpfiforeader"
		service="msicpfiforeader"
                req_count=$NOP_FIFO
                count=$count_fifo
                down_flag=1
        elif [ "$NOP_ICP" -gt "0" ] && [ "$count_icp" -ne "$NOP_ICP" ];then
                process="msicp"
		service="msicp"
                req_count=$NOP_ICP
                count=$count_icp
                down_flag=1
	else
		down_flag=0
        fi

	if [ -e "/home/wsadmin/stop_mail_msicp" ] && [ $down_flag -eq 0 ]; then
	      SPL_CASE=1
	      FLAG=0
	      SENDMAIL $count "$process" $req_count
	      SPL_CASE=0
	      rm -rf /home/wsadmin/stop_mail_msicp
	fi

	if [ "$down_flag" -eq "1" ];then
            echo `ps -eaf | grep $service` > msicp_msg
			stopaccfifo
            stop

	    ulimit -n $PROCESS_FD_LIMIT
	    ulimit -c unlimited

            start

			let mail_count++
			if [ ! -e "/home/wsadmin/stop_mail_msicp" ];then
                if [ $mail_count -gt 2 ];then
                        let FLAG++
                        if [ $FLAG -eq 1 ]; then
                                SENDMAIL $count "$process" $req_count
								echo 1 > /home/wsadmin/stop_mail_msicp
                        fi
                else
                        SENDMAIL $count "$process" $req_count
                fi
			fi
        else
                mail_count=0
        fi

}


##############################################
##### FUNCTIONS DEFINITION ENDS FROM HERE#####
##############################################

################################
##### MAIN startS FROM HERE#####
###############################
if [ -e "$MSICP" ] && [ -e "$MSICPFIFOREADER" ] && [ -e "$ICP_CONF" ] && [ -e "$FIFO_CONF" ]; then
	count_time="0"
	while [ "$count_time" -lt "$MAX_TIME" ]
	do
	        count_time=$(($count_time+$SLEEP_TIME))
	#      	echo $count_time
		SERVICES
	        sleep $SLEEP_TIME
	done
fi
##############################
##### MAIN ENDS FROM HERE#####
##############################
