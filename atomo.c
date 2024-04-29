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

#define N_ATOM_MAX 118

int split_atom(int atomic_number);

int main(int argc, char* argv[]){
	(void)argc;

	int atomic_number = atoi(argv[1]);

	key_t key = ftok("/master.c", 'x'); //Coda di messaggi
	int semid = semget(key, 1, 0600);

	P(semid, 0);     								 	  //Sync
	wait_for_zero(semid, 0);

	printf("NUM: %d, ATOM: %d\n", getpid(), atomic_number);

	atomic_number = split_atom(atomic_number);
	printf("Parent split del processo %d, nuovo AN: %d\n", getpid(), atomic_number);
	exit(0);
}

int split_atom(int atomic_number){
	int file_pipes[2];
	pid_t kid_pid;
	int kid_atomic_number;

	if (pipe(file_pipes) == 0) { //METTI ANCHE QUA UN TEST_ERROR
			switch (kid_pid = fork()) {
				case -1:
					//TEST_ERRORS --> Meltdown
					break;

				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					printf("Split del processo %d: %d\n", getpid(), atomic_number);
					close(file_pipes[0]);  
					char atomic_number_str[10];
					sprintf(atomic_number_str, "%d", atomic_number);
					//char *argv[] = {"atomo", atomic_number_str, NULL};
					//execve("atomo", argv, NULL); //Secondo parametro = args (array di char)
					exit(0); //QUANDO FAI L'EXECVE COMMENTA STO EXIT
					break;

				default:  
					close(file_pipes[0]);  

					srand(kid_pid);
					kid_atomic_number = (rand()%(atomic_number-1)) +1; 
					atomic_number -= kid_atomic_number; 

					write(file_pipes[1], &kid_atomic_number, sizeof(int));
					break;
			}
		close(file_pipes[1]); // ? Assicurarsi che dopo questa istruzione la PIPE non ci sia piu'
	}

	return atomic_number;
}