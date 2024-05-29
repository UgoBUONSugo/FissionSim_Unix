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
#include "external.h"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int split_atom(int atomic_number, struct SimStats *shared_memory, int semid);

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

	exit(0);
}

int split_atom(int atomic_number, struct SimStats *shared_memory, int semid){
	if(atomic_number <= 1)
	{
		P(semid, 1);
		shared_memory->waste_count++;
		V(semid, 1);

		exit(0);
	}

	int kid_atomic_number;
	int file_pipes[2];
	pid_t kid_pid;

	if (pipe(file_pipes) == 0) 
	{ 
		switch (kid_pid = fork())
		{
			case -1:
				key_t key = ftok("master.c", 'y');
				pid_t *master_pid;
				int m_id = shmget(key, sizeof(*master_pid), 0600);
				master_pid = (pid_t*) shmat(m_id, NULL, 0);
				kill(*master_pid, SIGUSR2);
				break;

			case 0:
				char atomic_number_str[10];
				char *argv[4];

				close(file_pipes[1]);

				read(file_pipes[0], &atomic_number, sizeof(int));
				close(file_pipes[0]);
		
				sprintf(atomic_number_str, "%d", atomic_number);
				argv[0] = "atomo";
				argv[1] = atomic_number_str;
				argv[2] = "0";
				argv[3] = NULL;
				execve("atomo", argv, NULL); 
				break;

			default:  
				close(file_pipes[0]);  

				srand(kid_pid);
				kid_atomic_number = (rand()%(atomic_number-1)) +1; 
				atomic_number -= kid_atomic_number; 

				write(file_pipes[1], &kid_atomic_number, sizeof(int));
				int liberated_energy = atomic_number * kid_atomic_number - MAX(atomic_number, kid_atomic_number);

				P(semid, 1);
				shared_memory->liberated_energy += liberated_energy;
				shared_memory->split_count++;
				V(semid, 1);

				break;
		}
		close(file_pipes[1]); 
	}

	return atomic_number;
}