#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
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

void init_atom(int atoms_n);
void init_activator();
void init_supply();
void timeout();
void get_var();

struct msgbuf{
	long mtype;
};

int ENERGY_DEM;
int N_ATOMI_INIT;
int N_ATOM_MAX;
int SIM_DURATION;
int ENERGY_EXPLODE_THRESHOLD;
char *STEP_ATTIVATORE;
char *STEP_ALIMENTAZIONE;

int semid;
pid_t kid_pid;
key_t key;
struct sembuf sops[1];

int main(){
	get_var();

	long rem_energy = 1000;
	struct SimStats recents;
	memset(&recents, 0, sizeof(recents));

	signal(SIGQUIT, timeout);  
	signal(SIGUSR2, timeout); 

	printf("Processo numero: %d\nPremere CTRL+\\ per terminale il programma\n", getpid());

	key = ftok("/master.c", 'x');
	semid = semget(key, 2, IPC_CREAT | 0600);
	semctl(semid, 0, SETVAL, N_ATOMI_INIT + 3);
	semctl(semid, 1, SETVAL, 1);

  int msgid = msgget(key, IPC_CREAT | 0600); (void)msgid;

  struct SimStats *shared_memory;
  int m_id = shmget(key, sizeof(*shared_memory), IPC_CREAT | 0600);
  shared_memory = (struct SimStats*) shmat(m_id, NULL, 0);

	init_atom(N_ATOMI_INIT);
	init_activator();
	init_supply();

	signal(SIGALRM, timeout);shared_memory->tot_energy = 290;

	P(semid, 0);
	wait_for_zero(semid, 0);
	alarm(SIM_DURATION);

	while(true)
	{ 
		sleep(1);

		P(semid, 1);

		rem_energy += shared_memory->tot_energy - recents.tot_energy;
		
		printf("\nNumero di attivazioni occorse in TOTALE: %i. Nell'ultimo secondo %i\n", shared_memory->activation_count, shared_memory->activation_count - recents.activation_count);
		printf("Scissioni effettuate in TOTALE: %i. Nell'ultimo secondo: %i\n", shared_memory->split_count, shared_memory->split_count-recents.split_count);
		printf("Energia prodotta in TOTALE: %li. Nell'ultimo secondo: %li\n", shared_memory->tot_energy, shared_memory->tot_energy-recents.tot_energy);
		printf("Energia consumata in TOTALE: %li. Nell'ultimo secondo: %li\n", shared_memory->consumed_energy, recents.consumed_energy);
		printf("Rifiuti generati in TOTALE: %i. Nell'ultimo secondo: %i\n", shared_memory->waste_count, shared_memory->waste_count-recents.waste_count);
		printf("Energia disponibile: %li.\n", rem_energy);

		if((rem_energy -= ENERGY_DEM) < 0)
		{
			timeout(0);
		}
		shared_memory->consumed_energy += ENERGY_DEM;
		
		recents = *shared_memory;
		V(semid, 1);

		if(rem_energy > ENERGY_EXPLODE_THRESHOLD){
			timeout(-2);
		}
	
		recents.consumed_energy = ENERGY_DEM;
	}
	
}

void init_atom(int n){
  int atomic_number;
	int file_pipes[2];

	if (pipe(file_pipes) == 0)
	{ 

		for(int i = 0; i < n; i++)
		{
			switch (kid_pid = fork()){
				case -1:
					kill(getpid(), SIGUSR2);
					break;

				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					close(file_pipes[0]);  

					char atomic_number_str[10];
					sprintf(atomic_number_str, "%d", atomic_number);
					char *argv[] = {"atomo", atomic_number_str, "0", NULL};
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

void init_activator(){
	switch (kid_pid = fork()) {
		case -1:
			//TEST_ERRORS
			break;

		case 0:
			char *argv[] = {STEP_ATTIVATORE, NULL};
			execve("attivatore", argv, NULL);
			break;

		default:  
			break;
	}
}

void init_supply(){
	switch (kid_pid = fork()) {
		case -1:
			//TEST_ERRORS
			break;

		case 0:
			char *atom_max[20];
			sprintf(*atom_max, "%d", N_ATOM_MAX);
			char *argv[] = {STEP_ALIMENTAZIONE, *atom_max, NULL};
			execve("alimentazione", argv, NULL);
			break;

		default:  
			return;
			break;
		}
}

void timeout(int signum){

	signal(SIGQUIT, SIG_IGN);
	kill(0, SIGQUIT);

	key = ftok("/master.c", 'x');
	semctl(semget(key, 1, 0600), 0, IPC_RMID);
	msgctl(msgget(key, 0600), IPC_RMID, NULL);
	shmctl(shmget(key, sizeof(struct SimStats), IPC_CREAT | 0600), IPC_RMID, NULL);

	printf("Simulazione terminata per: ");

	switch(signum)
	{
		case SIGALRM:
			printf("TIMEOUT\n");
			break;

		case SIGUSR2:
			printf("Meltdown\n");
			break;

		case -2:
			printf("EXPLODE\n");
			break;

		case 0:
			printf("BLACKOUT\n");
			break;

		default:
			printf("Sconosciuto\n");
			break;
	}

	exit(EXIT_SUCCESS);
}

void get_var(){
	setenv("ENERGY_DEM", "600", 0);
	ENERGY_DEM = atoi(getenv("ENERGY_DEM"));

	setenv("N_ATOMI_INIT", "50", 0);
	N_ATOMI_INIT = atoi(getenv("N_ATOMI_INIT"));

	setenv("N_ATOM_MAX", "100", 0);
	N_ATOM_MAX = atoi(getenv("N_ATOM_MAX"));

	setenv("STEP_ATTIVATORE", "1", 0);
	STEP_ATTIVATORE = getenv("STEP_ATTIVATORE");

	setenv("STEP_ALIMENTAZIONE", "3", 0);
	STEP_ALIMENTAZIONE = getenv("STEP_ALIMENTAZIONE");

	setenv("SIM_DURATION", "30", 0);
	SIM_DURATION = atoi(getenv("SIM_DURATION"));

	setenv("ENERGY_EXPLODE_THRESHOLD", "10000", 0);
	ENERGY_EXPLODE_THRESHOLD = atoi(getenv("ENERGY_EXPLODE_THRESHOLD"));
}