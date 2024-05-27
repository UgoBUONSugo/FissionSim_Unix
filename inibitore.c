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

void sim_print();

struct SimStats *shared_memory;
int counter;
int semid;

int main(){
	struct sigaction sa;
	struct msgbuf buf;
	int self_pid;
	int msgid;
	int shmid;
	key_t key;

	self_pid = getpid();
	key = ftok("master.c", 'x');
	msgid = msgget(key, 0600);
	semid = semget(key, 1, 0600);
	shmid = shmget(key, sizeof(*shared_memory), 0600); 
	shared_memory = (struct SimStats*) shmat(shmid, NULL, 0);

	counter = 0;
	buf.mtype = 1;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = &sim_print;
	sigaction (SIGUSR2, &sa, NULL);

	srand(getpid());

	P(semid, 0);
	wait_for_zero(semid, 0);

	while(true)
	{
		while(msgrcv(msgid, NULL, 0, self_pid, 0) && errno == EINTR){}

		if((rand()%10+1) > 6)
		{
			while(msgsnd(msgid, &buf, 0, 0) && errno == EINTR){}
		}
		else
		{
			counter++;
		}
	}
}

void sim_print(){

	//Mutex sem, critical section begins
	P(semid, 1);

	shared_memory->absorbed_energy += (float)(shared_memory->liberated_energy)*0.5;
	shared_memory->liberated_energy -= (float)(shared_memory->liberated_energy)*0.5;
	shared_memory->activation_interrupted = counter; 

	P(semid, 2); //Sem to communicate to the master process that the shared memory has been updated
	counter=0;
}