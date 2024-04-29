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

int main(int argc, char* argv[]){
	(void)argc;
	int atomic_number = atoi(argv[1]);
	key_t key = ftok("/master.c", 'x');
	int semid = semget(key, 1, 0600);
	P(semid, 0);
	wait_for_zero(semid, 0);
	printf("NUM: %d, ATOM: %d\n", getpid(), atomic_number);
	//while(1){
	exit(0);
}

void split_atom(){
  int atomic_number;
	int file_pipes[2];
	pid_t kid_pid;

	if (pipe(file_pipes) == 0) { //METTI ANCHE QUA UN TEST_ERROR
			switch (kid_pid = fork()) {
				case -1:
					//TEST_ERRORS
					break;
				case 0:
					close(file_pipes[1]);
					read(file_pipes[0], &atomic_number, sizeof(int));
					printf("Numero atomico del processo %d: %d\n", getpid(), atomic_number);
					close(file_pipes[0]);  
					char *argv[3];
					argv[0] = "atomo.c";
					*argv[1] = (char)atomic_number;
					argv[2] = NULL;
					execve("atomo.c", argv, NULL); //Secondo parametro = args (array di char)
					exit(0); //QUANDO FAI L'EXECVE COMMENTA STO EXIT
					break;
				default:  
					close(file_pipes[0]);  
					srand(kid_pid);
					atomic_number = (rand()%70) + 1; ///MODIFICA 70 N ATOM MAX
					write(file_pipes[1], &atomic_number, sizeof(int));
					break;
			}
		close(file_pipes[1]); // ? Assicurarsi che dopo questa istruzione la PIPE non ci sia piu'
	}
}