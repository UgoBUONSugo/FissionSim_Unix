# Makefile for compiling master and atomo programs

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: master atomo

master: master.c external.c
	$(CC) $(CFLAGS) -o master master.c external.c

atomo: atomo.c external.c
	$(CC) $(CFLAGS) -o atomo atomo.c external.c

clean:
	rm -f master atomo

