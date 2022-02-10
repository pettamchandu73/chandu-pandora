#include<time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include"../sr_module.h"

#define  SHM_OFFSET 24
#define SHMSEGMENTSIZE (SHM_OFFSET + ( no_of_shmunits * sizeof(SHMCLRIDS)))


typedef struct shm_clear_ids{
	int shmid;
	int insert_time;
	int in_use;
} SHMCLRIDS;

int nested_calls_shm_clear();
int init_shm();
