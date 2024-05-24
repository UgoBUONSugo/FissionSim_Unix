#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

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

void sim_end(){
	exit(1);
}

void toggle_signals(int block, int sig){
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, sig); 
  sigprocmask(block ? SIG_BLOCK : SIG_UNBLOCK, &set, NULL);
}

void init_atom(int n, int N_ATOM_MAX, char *init){
	pid_t kid_pid;
  int atomic_number;
	int file_pipes[2];

	if (pipe(file_pipes) == 0)
	{ 
		for(int i = 0; i < n; i++)
		{
			switch (kid_pid = fork()){
				case -1:
					key_t key = ftok("master.c", 'y');
					pid_t *master_pid;
				  int m_id = shmget(key, sizeof(*master_pid), 0600);
				  master_pid = (pid_t*) shmat(m_id, NULL, 0);
				  kill(*master_pid, SIGUSR2);
					break;

				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					close(file_pipes[0]);  

					char atomic_number_str[10];
					sprintf(atomic_number_str, "%d", atomic_number);
					char *argv[] = {"atomo", atomic_number_str, init, NULL};
					execve("atomo", argv, NULL);
					break;

				default:  
					srand(kid_pid);
					atomic_number = (rand()%N_ATOM_MAX) + 1;
					write(file_pipes[1], &atomic_number, sizeof(int));
					break;
			}
		}
		close(file_pipes[0]);  
		close(file_pipes[1]); 
	}
}