#pragma once
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>


typedef enum{
	KILL = 0,
	SIGQUEUE,
	SIGRT
} MODE;

MODE parse_mode(char *mode){
	if(strcmp(mode, "KILL") == 0){
		return KILL;
	}else if(strcmp(mode, "SIGQUEUE") == 0){
		return SIGQUEUE;
	}else if(strcmp(mode, "SIGRT") == 0){
		return SIGRT;
	}else{
		printf("Invalid mode!\n");
		exit(EXIT_FAILURE);
	}
}

int get_signal1(MODE mode){
	return (mode == SIGRT) ? SIGRTMIN : SIGUSR1;
}

int get_signal2(MODE mode){
	return (mode == SIGRT) ? SIGRTMIN + 1 : SIGUSR2;
}

void send_signal(int i, int pid, MODE mode, int signal){
	if(mode == SIGQUEUE){
		const union sigval temp = {.sival_int = i};
		sigqueue(pid, signal, temp);
	}else{
		kill(pid, signal);
	}
}
