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

#define TOT_NSEC 0
#define N_ACTIVATIONS 5

void inhib_switch(int signum);

int inhibitor_pid;
struct msgbuf buf;

int main(int argc, char* argv[]){
	(void)argc;
	key_t key;
	int m_id;
	int msgid;
	int semid;
	int STEP_ATTIVATORE;
	struct sigaction sa;
	struct timespec timer;
	struct SimStats *shared_memory;

	STEP_ATTIVATORE = atoi(argv[1]);
	inhibitor_pid = atoi(argv[2]); 

	if(*argv[3] == 'y') buf.mtype = inhibitor_pid;
	else buf.mtype = 1;

	key = ftok("master.c", 'x');
	semid = semget(key, 1, 0600);
	msgid = msgget(key, 0600);

	timer.tv_sec = STEP_ATTIVATORE;
	timer.tv_nsec = TOT_NSEC;

	m_id = shmget(key, sizeof(*shared_memory), 0600);
	shared_memory = (struct SimStats*) shmat(m_id, NULL, 0);

	bzero(&sa, sizeof(sa));
	sa.sa_handler = &inhib_switch;
	sigaction(SIGIO, &sa, NULL);

	P(semid, 0);
	wait_for_zero(semid, 0);
	
	while(true)
	{
		toggle_signals(1, SIGIO);
		for(int i = 0; i < N_ACTIVATIONS; i++)
		{
			msgsnd(msgid, &buf, 0, 0);
		}

		P(semid, 1);
 		shared_memory->activation_count += N_ACTIVATIONS;
		V(semid, 1);
		toggle_signals(0, SIGIO);
		nanosleep(&timer, NULL);
	}
}

void inhib_switch(int signum){
	(void)signum;

	if(buf.mtype == 1){
		buf.mtype = inhibitor_pid;
	}
	else{
		buf.mtype = 1;
	}

	return;
}