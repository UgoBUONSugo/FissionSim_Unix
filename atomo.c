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
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

struct msgbuf{
	long mtype;
} buf;

int split_atom(int atomic_number);

int main(int argc, char* argv[]){
	(void)argc;

	int atomic_number = atoi(argv[1]);
	int init = atoi(argv[2]);

	key_t key = ftok("/master.c", 'x'); //Coda di messaggi
	int semid = semget(key, 1, 0600);
	int queid = msgget(key, 0600);

	if(init == 0){
		P(semid, 0);     							  //Sync
		wait_for_zero(semid, 0);					  //Inizio SIM
	}

	while(true){
		msgrcv(queid, &buf, sizeof(buf), 1, 0);
		atomic_number = split_atom(atomic_number);
	}

	exit(0);
}

int split_atom(int atomic_number){

	if(atomic_number == 1){
		return -1; //Scoria
	}

	int file_pipes[2];
	pid_t kid_pid;
	int kid_atomic_number;

	if (pipe(file_pipes) == 0) { 
		switch (kid_pid = fork()) {
			case -1:
				printf("GHWA8IGHWA8UGHWAIGW");
				break;

			case 0:
				close(file_pipes[1]);

				read(file_pipes[0], &atomic_number, sizeof(int));
				printf("Processo Figlio AN %d: %d\n", getpid(), atomic_number);printf("----------------------------\n");
				close(file_pipes[0]);  

				char atomic_number_str[10];
				sprintf(atomic_number_str, "%d", atomic_number);
				char *argv[] = {"atomo", atomic_number_str, "1", NULL};
				execve("atomo", argv, NULL); 
				break;

			default:  
				close(file_pipes[0]);  

				srand(kid_pid);
				kid_atomic_number = (rand()%(atomic_number-1)) +1; 
				atomic_number -= kid_atomic_number; 

				write(file_pipes[1], &kid_atomic_number, sizeof(int));
				int liberated_energy = atomic_number * kid_atomic_number - MAX(atomic_number, kid_atomic_number);printf("----------------------------\n");
				printf("Processo Padre AN %d: %d\n", getpid(), atomic_number);
				printf("Energia liberata: %d\n", liberated_energy);
				break;
		}
		close(file_pipes[1]); 
	}

	return atomic_number;
}