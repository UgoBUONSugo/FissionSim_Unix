#include <stdlib.h>
#include <sys/sem.h>
#include <signal.h>

int P(int sem_id, int n){
	struct sembuf op;
	op.sem_num = n;
	op.sem_op = -1;
	op.sem_flg = 0;
	return semop(sem_id, &op, 1);
}

int V(int sem_id, int n){
	struct sembuf op;
	op.sem_num = n;
	op.sem_op = 1;
	op.sem_flg = 0;
	return semop(sem_id, &op, 1);
}

int wait_for_zero(int sem_id, int n){
	struct sembuf op;
	op.sem_num = n;
	op.sem_op = 0;
	op.sem_flg = 0;
	return semop(sem_id, &op, 1);
}

void toggle_signals(int block, int sig){
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, sig); 
  sigprocmask(block ? SIG_BLOCK : SIG_UNBLOCK, &set, NULL);
}