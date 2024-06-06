#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "../include/external.h"
#include "../include/sem_sig_lib.h"

void init_atom(int n, int N_ATOM_MAX, char *init){
  pid_t kid_pid;
  int atomic_number;
	int file_pipes[2];

	if (pipe(file_pipes) == 0)
	{
		for(int i = 0; i < n; i++)
		{
			switch (kid_pid = fork())
			{
				case -1:
				{
					key_t key = ftok("master.c", 'y');
					pid_t *master_pid;
				  int m_id = shmget(key, sizeof(*master_pid), 0600);
				  master_pid = (pid_t*) shmat(m_id, NULL, 0);
				  kill(*master_pid, SIGUSR2);
					break;
				}

				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					close(file_pipes[0]);  

					char atomic_number_str[10];
					sprintf(atomic_number_str, "%d", atomic_number);
					char *argv[] = {"atomo", atomic_number_str, init, NULL};
					execve("atomo", argv, NULL);
					break;

				default:  
					srand(kid_pid);
					atomic_number = (rand()%N_ATOM_MAX) + 1;
					write(file_pipes[1], &atomic_number, sizeof(int));
					break;
			}
		}
		close(file_pipes[0]);  
		close(file_pipes[1]); 
	}
	TEST_ERROR
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
			{
				key_t key = ftok("master.c", 'y');
				pid_t *master_pid;
				int m_id = shmget(key, sizeof(*master_pid), 0600);
				master_pid = (pid_t*) shmat(m_id, NULL, 0);
				kill(*master_pid, SIGUSR2);
				break;
			}

			case 0:
			{
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
			}

			default:  
				close(file_pipes[0]);  

				srand(kid_pid);
				kid_atomic_number = ( rand()%(atomic_number-1) ) +1; 
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
	else
	{
		struct msgbuf buf;
		buf.mtype = 1;
		msgsnd(msgget(ftok("master.c", 'x'), 0600), &buf, 0, 0);
	}

	return atomic_number;
}