#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "../include/external.h"
#include "../include/sem_sig_lib.h"

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

	TEST_ERROR
	srand(getpid());

	P(semid, 0);
	wait_for_zero(semid, 0);

	while(1)
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
	//Sem mutex, inizio sezione critica
	P(semid, 1);

	shared_memory->absorbed_energy += (float)(shared_memory->liberated_energy)*0.5;
	shared_memory->liberated_energy -= (float)(shared_memory->liberated_energy)*0.5;
	shared_memory->activation_interrupted = counter; 

	P(semid, 2); //Sem per comunicare al processo master che la memoria condivisa è stata aggiornata
	counter=0;
}