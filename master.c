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
#include <fcntl.h>
#include "external.h"

#define BLACKOUT -1
#define EXPLODE -2

#define TOT_SEC 1
#define TOT_NSEC 0

void sigio_handl(int signum);
void init_supply();
pid_t init_activator(char answ);
pid_t init_inhibitor();
void inhib_switch();
void get_var();
void sim_print();
void sim_term();

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
int inhib_status;
pid_t kid_pid;
key_t key;
struct sembuf sops[1];
pid_t inhibitor_pid;
pid_t activator_pid;

int main(){
	float ratio;
	char answ;

	atexit(&sim_term);
	get_var();

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = inhib_switch;
	sigaction(SIGIO, &sa, NULL);

	long rem_energy = 10000;
	struct SimStats overall_stats;
	memset(&overall_stats, 0, sizeof(overall_stats));
  
	signal(SIGUSR2, sim_print); 

	key = ftok("master.c", 'x');
	semid = semget(key, 3, IPC_CREAT | 0600);
	semctl(semid, 0, SETVAL, N_ATOMI_INIT + 4); //Sem per sync inizio
	semctl(semid, 1, SETVAL, 1);								//Sem mutex per accedere alla SM
	semctl(semid, 2, SETVAL, 1);								//Sem per comunicazione master-inibitore

  int msgid = msgget(key, IPC_CREAT | 0600); (void)msgid;

  struct SimStats *shared_memory;
  int m_id = shmget(key, sizeof(*shared_memory), IPC_CREAT | 0600);
  shared_memory = (struct SimStats*) shmat(m_id, NULL, 0);
  memset(shared_memory, 0, sizeof(*shared_memory));

  pid_t *master_pid;
  int m_id2 = shmget(ftok("master.c", 'y'), sizeof(*master_pid), IPC_CREAT | 0600);
  master_pid = (pid_t*) shmat(m_id2, NULL, 0);
  memset(master_pid, 0, sizeof(*master_pid));
  *master_pid = getpid();

	inhibitor_pid = init_inhibitor();
	init_atom(N_ATOMI_INIT, N_ATOM_MAX, "0");
	init_supply();


	printf("Attivare il processo inibitore?(y/n) ");
	scanf(" %c", &answ);
	while(answ != 'y' && answ != 'n'){
		printf("Rispondere 'y' oppure 'n': ");
		scanf(" %c", &answ);
	}
	activator_pid = init_activator(answ);
	inhib_status = answ == 'y' ? 1 : 0;

	signal(SIGALRM, sim_print);

	struct timespec timer;
	timer.tv_sec = TOT_SEC;
	timer.tv_nsec = TOT_NSEC;

	fcntl(0, F_SETOWN, getpid());
	fcntl(0, F_SETFL, O_NONBLOCK | O_ASYNC);

	P(semid, 0);
	wait_for_zero(semid, 0);

	alarm(SIM_DURATION);

	while(true)
	{ 

		toggle_signals(1, SIGIO);
		if(inhib_status == 1){
			semctl(semid, 2, SETVAL, 1);
			kill(inhibitor_pid, SIGUSR2);
			wait_for_zero(semid, 2);
		}
		else{
			P(semid, 1);
		}

		overall_stats.activation_count += shared_memory->activation_count;
		overall_stats.split_count += shared_memory->split_count;
		overall_stats.activation_interrupted += shared_memory->activation_interrupted;
		overall_stats.waste_count += shared_memory->waste_count;
		overall_stats.liberated_energy += shared_memory->liberated_energy;
		overall_stats.consumed_energy += shared_memory->consumed_energy;
		overall_stats.absorbed_energy += shared_memory->absorbed_energy;

		rem_energy += shared_memory->liberated_energy;
		
		printf(" ------------------------------------------------ \n");
		printf("|                          IN TOTALE  /SECONDO   |\n");
		printf("|Numero di attivazioni %10i  %10i    |\n", overall_stats.activation_count, shared_memory->activation_count);
		printf("|Scissioni effettuate  %10i  %10i    |\n", overall_stats.split_count, shared_memory->split_count);
		printf("|Rifiuti generati      %10i  %10i    |\n", overall_stats.waste_count, shared_memory->waste_count);
		printf("|Scissioni negate      %10i  %10i    |\n", overall_stats.activation_interrupted, shared_memory->activation_interrupted);
		printf("|Energia liberata      %10li  %10li    |\n", overall_stats.liberated_energy, shared_memory->liberated_energy);
		printf("|Energia consumata     %10li  %10li    |\n", overall_stats.consumed_energy, shared_memory->consumed_energy);
		printf("|Energia assorbita     %10li  %10li    |\n", overall_stats.absorbed_energy, shared_memory->absorbed_energy);
		printf("|Energia disponibile:  %10li                |\n", rem_energy);

		ratio = (float)(rem_energy) / (float)(ENERGY_EXPLODE_THRESHOLD);

		if((rem_energy -= ENERGY_DEM) < 0)
		{
			SIM_PRINT;
			sim_print(BLACKOUT);
		}

		bzero(shared_memory, sizeof(struct SimStats)); //Reset a zero della sm
		shared_memory->consumed_energy = ENERGY_DEM;

		V(semid, 1);

		SIM_PRINT;
		if(rem_energy > ENERGY_EXPLODE_THRESHOLD) sim_print(EXPLODE);

		toggle_signals(0, SIGIO); 
		nanosleep(&timer, NULL);

		/*while(nanosleep(&timer, &rem) && errno==EINTR){
        timer = rem;
    }
    timer.tv_sec = 1;*/
	}
	
}

pid_t init_activator(char answ){
	switch (kid_pid = fork()) {
		case -1:
			sim_print(SIGUSR2);
			break;

		case 0:
			char inhib_pid[20]; 
			sprintf(inhib_pid, "%d", inhibitor_pid);
			char *argv[] = {STEP_ATTIVATORE, inhib_pid, &answ, NULL};
			execve("attivatore", argv, NULL);
			break;

		default:  
			break;
	}

	return kid_pid;
}

void init_supply(){
	switch (kid_pid = fork()) {
		case -1:
			sim_print(SIGUSR2);
			break;

		case 0:
			char atom_max[20]; 
			sprintf(atom_max, "%d", N_ATOM_MAX);
			char *argv[] = {STEP_ALIMENTAZIONE, atom_max, NULL};
			execve("alimentazione", argv, NULL);
			break;

		default:  
			return;
			break;
		}
}

pid_t init_inhibitor(){
	switch (kid_pid = fork()) {
		case -1:
			sim_print(SIGUSR2);
			break;

		case 0:
			char *argv[] = {"inibitore", NULL};
			execve("inibitore", argv, NULL);
			break;

		default:
			break;
	}

	return kid_pid;
}

void sim_print(int signum){
	printf("\n-----------------------------------");
	printf("\nSimulazione terminata per: ");

	switch(signum)
	{
		case SIGALRM:
			printf("Timeout\n");
			break;

		case SIGUSR2:
			printf("Meltdown\n");
			break;

		case -1:
			printf("BLACKOUT\n");
			break;

		case -2:
			printf("EXPLODE\n");
			break;

		default:
			break;
	}

	char c;
	while ((c = getchar()) != EOF) { }

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

	setenv("ENERGY_EXPLODE_THRESHOLD", "30000", 0);
	ENERGY_EXPLODE_THRESHOLD = atoi(getenv("ENERGY_EXPLODE_THRESHOLD"));
}

void sigio_handl(int signum){
	(void)signum;
	inhib_switch();
}

void inhib_switch(){
	kill(activator_pid, SIGIO);
	inhib_status = (inhib_status+1)%2;
}

void sim_term(){
	signal(SIGUSR1, SIG_IGN);
	kill(0, SIGUSR1);

	key = ftok("/master.c", 'x');
	semctl(semget(key, 2, 0600), 0, IPC_RMID);
	msgctl(msgget(key, 0600), IPC_RMID, NULL);
	shmctl(shmget(key, sizeof(struct SimStats), IPC_CREAT | 0600), IPC_RMID, NULL);
	shmctl(shmget(ftok("master.c", 'y'), sizeof(pid_t), IPC_CREAT | 0600), IPC_RMID, NULL);
	printf("Memoria disallocata\n");
}