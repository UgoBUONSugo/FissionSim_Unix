#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "external.h"

#define REFUEL_Q 2
#define TOT_NSEC 0

int N_ATOM_MAX;

int main(int argc, char* argv[]){
	key_t key;
	int STEP_ALIMENTAZIONE;
	int semid;
	struct timespec timer;
	(void)argc;

	STEP_ALIMENTAZIONE = atoi(argv[0]);
	N_ATOM_MAX = atoi(argv[1]);

	key = ftok("master.c", 'x');
	semid = semget(key, 1, 0600);

	timer.tv_sec = STEP_ALIMENTAZIONE;
	timer.tv_nsec = TOT_NSEC;

	P(semid, 0);
	wait_for_zero(semid, 0);

	while(1){
		nanosleep(&timer, NULL);
		init_atom(REFUEL_Q, N_ATOM_MAX);
	}
}