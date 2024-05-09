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
#include "external.h"

#define REFUEL_Q 1
#define TOT_SEC 2
#define TOT_NSEC 0

#define N_ATOM_MAX 20

void init_atom(int n);


int main(){
	key_t key = ftok("/master.c", 'x');
	int semid = semget(key, 1, 0600);

	struct timespec timer;
	timer.tv_sec = TOT_SEC;
	timer.tv_nsec = TOT_NSEC;

	P(semid, 0);
	wait_for_zero(semid, 0);

	while(true){
		init_atom(REFUEL_Q);
		nanosleep(&timer, NULL);
	}

}


void init_atom(int n){
  int atomic_number;
	int file_pipes[2];
	pid_t kid_pid;

	if (pipe(file_pipes) == 0)
	{ 
		for(int i = 0; i < n; i++) {
			switch (kid_pid = fork()) {
				case -1:
					//TEST_ERRORS ---> MELTDOWN
					break;

				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					//printf("Numero atomico del processo %d: %d\n", getpid(), atomic_number);
					close(file_pipes[0]);  
					char atomic_number_str[10];
					sprintf(atomic_number_str, "%d", atomic_number);
					char *argv[] = {"atomo", atomic_number_str, "1", NULL};
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

}