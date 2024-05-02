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

#define TOT_SEC 1

struct msgbuf{
	long mtype;
};

int main(){

	key_t key = ftok("/master.c", 'x');
	int semid = semget(key, 1, 0600);
	int msgid = msgget(key, 0600);

	struct msgbuf buf;
	buf.mtype = 1;

	struct timespec timer;
	timer.tv_sec = 1;
	timer.tv_nsec = 0;

	P(semid, 0);
	wait_for_zero(semid, 0);
	/*
	while(true){
		msgsnd(msgid, &buf, sizeof(buf), 0);
		nanosleep(&timer, NULL);
	}*/
	for(int i = 0; i < 10; i++){
		msgsnd(msgid, &buf, sizeof(buf), 0);
		nanosleep(&timer, NULL);
	}
}