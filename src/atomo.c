#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "../include/external.h"
#include "../include/sem_sig_lib.h"
#include "../include/atoms_action_lib.h"

int main(int argc, char* argv[]){
	(void)argc;
	int init;
	int m_id;
	int semid;
	int queid;
	int atomic_number;
	key_t key;
	struct SimStats *shared_memory;
	
	atomic_number = atoi(argv[1]);
	init = atoi(argv[2]);

	key = ftok("master.c", 'x');

	m_id = shmget(key, sizeof(*shared_memory), 0600); 
	shared_memory = (struct SimStats*) shmat(m_id, NULL, 0); 

	semid = semget(key, 1, 0600);
	queid = msgget(key, 0600);

	if(init) 			//Init=1 -> atom process created during exe of the program -> no need for synchronization
	{     
		P(semid, 0); 
		wait_for_zero(semid, 0);		
	}

	while(true)
	{
		msgrcv(queid, NULL, 0, 1, 0);
		atomic_number = split_atom(atomic_number, shared_memory, semid);
	}

}