#include "nested_call_shm.h"


int shm_clearid_key = 0;
SHMCLRIDS * shmclearid_ptr = NULL;
int shm_Clear_id = 0;
int no_of_shmunits = 2000;
int timer = 60;
int deletetimer = 0;
int watcher_sleep = 2;
SHMCLRIDS * list= NULL;

int nested_calls_shm_clear()
{

	int i = 0,count = 0;
	time_t cur_time;

	timer = *(( int* )find_param_export("worksmart", "shmcleartimer", INT_PARAM) );
	deletetimer = *(( int* )find_param_export("worksmart", "shmdeletetimer", INT_PARAM) );
	no_of_shmunits = *(( int* )find_param_export("worksmart","shmsegments",INT_PARAM));
	watcher_sleep = *(( int* )find_param_export("worksmart","watchersleep",INT_PARAM));
	if(timer <= 0){
		timer = 60;
		LOG(L_ERR,"[nested_calls_shm_clear] did not configuration for shmcleartimer so setting default value: %d \n", timer);
	}
	if(no_of_shmunits <= 0){
		no_of_shmunits = 2000;
		LOG(L_ERR,"[nested_calls_shm_clear] did not configuration for shmsegments so setting default value: %d \n", no_of_shmunits);
	}
	if(deletetimer <= 0){
		deletetimer = 10;
		LOG(L_ERR,"[nested_calls_shm_clear] did not configuration for deletetimer so setting default value: %d \n", deletetimer);
	}
	if(watcher_sleep <= 0){
		watcher_sleep = 2;
		LOG(L_ERR,"[nested_calls_shm_clear] did not configuration for watchersleep so setting default value: %d \n", watcher_sleep);
	}
	
	init_shm();

	LOG(L_ERR,"[nested_calls_shm_clear] Watcher Process started, monitoring %d nodes sleep:%d id:%d addr:%p  \n", no_of_shmunits, watcher_sleep, shm_Clear_id, shmclearid_ptr);
	/*print_shm();*/
	while(1){
	    count = 0;	
		cur_time = time(0);
		i = SHM_OFFSET + sizeof(SHMCLRIDS);
		list = (char  *)shmclearid_ptr + i;
		while(count < no_of_shmunits-2){
			if( list && (list->in_use == 1) && ((cur_time - list->insert_time) > timer)){
				list->in_use = -1;
				LOG(L_ERR,"[nested_calls_shm_clear] Timeout for shm_id: %d insert_time: %d cur_time:%ld in_use: %d c:%d \n",
							list->shmid, list->insert_time, cur_time, list->in_use, count);
				list->insert_time = cur_time;
			}else if(list && ((list->in_use == -1)) && (cur_time - list->insert_time) > deletetimer ){
				shmctl(list->shmid,IPC_RMID,NULL);
				list->shmid = 0;
				list->insert_time = 0;
				list->in_use = 0;
			}
			count = count + 1;
			list+=1;
		}
		sleep(watcher_sleep);
	}
	return 0;
}	

int init_shm()
{

	shm_clearid_key = ftok("/tmp/fgrp_shmdetails",0x23ABCE);
	if (( shm_Clear_id = shmget(shm_clearid_key, SHMSEGMENTSIZE , 0666|IPC_CREAT)) < 0) {
		LOG(L_ERR , "[init_shm]Cann't create Shared Memory  %s\n\n" , strerror(errno));
		return -1;
	}

	if ((shmclearid_ptr  = (SHMCLRIDS *) shmat (shm_Clear_id, (char *) 0, 0)) == ( SHMCLRIDS *) -1) {
		LOG(L_ERR , "[init_shm]Cann't attach Shared Memory\n\n");	
		return  -1;
	}	
	return 0;
}
