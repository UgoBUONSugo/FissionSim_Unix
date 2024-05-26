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

struct msgbuf{
	long mtype;
};

#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO,		\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

void sim_print();

int counter = 0;
int semid;
struct SimStats *shared_memory;

int main(){
	int self_pid = getpid();
	key_t key = ftok("master.c", 'x');
	semid = semget(key, 1, 0600);

	int m_id = shmget(key, sizeof(*shared_memory), 0600); 
	shared_memory = (struct SimStats*) shmat(m_id, NULL, 0);

	struct msgbuf buf;
	buf.mtype = 1;

	int msgid = msgget(key, 0600);
	TEST_ERROR;

	srand(getpid());

	struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = &sim_print;
	sigaction (SIGUSR2, &sa, NULL);

	P(semid, 0);
	wait_for_zero(semid, 0);

	while(true){
start:
		msgrcv(msgid, &buf, 0, self_pid, 0);
		if (errno) {
	    	if (errno == EINTR){ errno = 0; goto start;}
		    else{TEST_ERROR}
    	}
		buf.mtype = 1;

finish:
		if( (rand()%10+1) > 6){
			msgsnd(msgid, &buf, 0, 0);
			if (errno) {
		    	if (errno == EINTR){ errno = 0; goto finish;}
			    else{TEST_ERROR}
    		}
		}
		else{
			counter++;
		}
	}
}

void sim_print(){

	P(semid, 1);

	shared_memory->absorbed_energy += (float)(shared_memory->liberated_energy)*0.5;
	shared_memory->liberated_energy -= (float)(shared_memory->liberated_energy)*0.5;
	shared_memory->activation_interrupted = counter; 

	P(semid, 2);
	
	counter=0;
}