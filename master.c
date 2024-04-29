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

//#define ATOM_NAME "atomo.out"
#define N_ATOMI_INIT 5
//#define TEST_ERROR;
#define N_ATOM_MAX 70

void init_atom(int energy_quantity);
void init_supply();

struct overall_stats{
	long energy_quantity;
	long energy_consumed;
	long inibitor_energy_absorbed; //---Normal---
	int activations_counter;
	int splits_number;
	int radiaction_waste_quantity;
	//Stampare anche i log di inibitor
};

struct msgbuf{
	long mtype;
};

int shmid;
int semid;
long *energy_quantity_pointer;
pid_t kid_pid;
key_t key;
struct sembuf sops[1];

int main(){

	//---------SEMAFORI--------//
	key = ftok("/master.c", 'x');
	semid = semget(key, 1, IPC_CREAT | 0600);
	//TEST_ERRORS
	if(semctl(semid, 0, SETVAL, N_ATOMI_INIT + 1)==-1){//if(semctl(semid, 0, SETVAL, N_ATOMI_INIT+3)==-1){
      printf("Error semctl\n");
      exit(100);
  }


  //--------MSGQ------//
  int msgid = msgget(key, IPC_CREAT | 0600);	
  //TEST ERROR

	//---------INIT ATOMO---------------//
	init_atom(N_ATOMI_INIT);

	//---------INIT alimentazione---------------//
	//init_supply();





	//------------EXIT-----------------//
	semctl(semid, 0, IPC_RMID);
	msgctl(msgid, IPC_RMID, NULL);
}

void init_atom(int n){
  int atomic_number;
	int file_pipes[2];

	if (pipe(file_pipes) == 0) { //METTI ANCHE QUA UN TEST_ERROR
		for(int i = 0; i < n; i++) {
			switch (kid_pid = fork()) {
				case -1:
					//TEST_ERRORS ---> MELTDOWN
					break;
				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					printf("Numero atomico del processo %d: %d\n", getpid(), atomic_number);
					close(file_pipes[0]);  
					char atomic_number_str[10];
					sprintf(atomic_number_str, "%d", atomic_number);
					char *argv[] = {"atomo", atomic_number_str, NULL};
					execve("atomo", argv, NULL); //Secondo parametro = args (array di char)
					exit(0); //QUANDO FAI L'EXECVE COMMENTA STO EXIT
					break;
				default:  
					srand(kid_pid);
					atomic_number = (rand()%N_ATOM_MAX) + 1;
					write(file_pipes[1], &atomic_number, sizeof(int));
					break;
			}
		}
		P(semid, 0);
		wait_for_zero(semid, 0);
		close(file_pipes[0]);  
		close(file_pipes[1]); // ? Assicurarsi che dopo questa istruzione la PIPE non ci sia piu'
	}
}

void init_supply(){
	switch (kid_pid = fork()) {
				case -1:
					//TEST_ERRORS
					break;
				case 0:
						//execve(SUPPLY_NAME, args, NULL)
						break;
				default:  

					break;
			}
}