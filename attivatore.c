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

#define TOT_NSEC 0
#define N_ACTIVATIONS 3

struct msgbuf{
	long mtype;
};

int main(int argc, char* argv[]){
	(void)argc;
	int STEP_ATTIVATORE = atoi(argv[0]);

	key_t key = ftok("/master.c", 'x');
	int semid = semget(key, 1, 0600);
	int msgid = msgget(key, 0600);

	struct msgbuf buf;
	buf.mtype = 1;

	struct timespec timer;
	timer.tv_sec = STEP_ATTIVATORE;
	timer.tv_nsec = TOT_NSEC;

	struct SimStats *shared_memory;
	int m_id = shmget(key, sizeof(*shared_memory), 0600); 
	shared_memory = (struct SimStats*) shmat(m_id, NULL, 0);

	P(semid, 0);
	wait_for_zero(semid, 0);
	
	while(true)
	{
		for(int i = 0; i < N_ACTIVATIONS; i++){
			msgsnd(msgid, &buf, sizeof(buf), 0);
		}

		P(semid, 1);
 		shared_memory->activation_count += N_ACTIVATIONS;
		V(semid, 1);

		nanosleep(&timer, NULL);
	}
}