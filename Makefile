# Makefile for compiling master and subfiles

CC = gcc
CFLAGS = -Wall -Wvla -Wextra -Werror -std=gnu99

all: master atomo attivatore alimentazione

master: master.c external.c
	$(CC) $(CFLAGS) -o master master.c external.c

atomo: atomo.c external.c
	$(CC) $(CFLAGS) -o atomo atomo.c external.c

attivatore: attivatore.c external.c
	$(CC) $(CFLAGS) -o attivatore attivatore.c external.c

alimentazione: alimentazione.c external.c
	$(CC) $(CFLAGS) -o alimentazione alimentazione.c external.c

clean:
	rm -f master atomo attivatore alimentazione

