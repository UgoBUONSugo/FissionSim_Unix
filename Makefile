CC = gcc
CFLAGS = -Wall -Wvla -Wextra -Werror -Wpedantic -D_GNU_SOURCE

all: master atomo attivatore alimentazione inibitore

master: master.c src/sem_sig_lib.c src/atoms_action_lib.c
	$(CC) $(CFLAGS) -o master master.c src/sem_sig_lib.c src/atoms_action_lib.c

atomo: src/atomo.c src/sem_sig_lib.c src/atoms_action_lib.c
	$(CC) $(CFLAGS) -o atomo src/atomo.c src/sem_sig_lib.c src/atoms_action_lib.c

attivatore: src/attivatore.c src/sem_sig_lib.c
	$(CC) $(CFLAGS) -o attivatore src/attivatore.c src/sem_sig_lib.c src/atoms_action_lib.c

alimentazione: src/alimentazione.c src/sem_sig_lib.c src/atoms_action_lib.c
	$(CC) $(CFLAGS) -o alimentazione src/alimentazione.c src/sem_sig_lib.c src/atoms_action_lib.c

inibitore: src/inibitore.c src/sem_sig_lib.c
	$(CC) $(CFLAGS) -o inibitore src/inibitore.c src/sem_sig_lib.c src/atoms_action_lib.c

clean:
	rm -f master atomo attivatore alimentazione inibitore

run: master
	./master