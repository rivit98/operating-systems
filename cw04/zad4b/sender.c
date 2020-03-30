#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "common.h"


bool got_last_message = false;
int received_sig_counter = 0;

void sigusr1_handler(int signo){
	received_sig_counter++;
}

void sigusr1_sigqueue_handler(int signo, siginfo_t *info, void *context){
	printf("Received signal no. %d\n", info->si_value.sival_int);

	received_sig_counter++;
}

void sigusr2_handler(int signo){
	got_last_message = true;
}

void setup_signals(MODE mode){
	sigset_t block_mask;
	sigfillset(&block_mask);
	sigdelset(&block_mask, get_signal1(mode));
	sigdelset(&block_mask, get_signal2(mode));

	if(sigprocmask(SIG_SETMASK, &block_mask, NULL) != 0){
		puts("sigprocmask error");
		exit(EXIT_FAILURE);
	}


	struct sigaction action;
	if(mode == SIGQUEUE){
		action.sa_flags = SA_SIGINFO;
		action.sa_sigaction = sigusr1_sigqueue_handler;
	}else{
		action.sa_flags = 0;
		action.sa_handler = sigusr1_handler;
	}
	sigemptyset(&action.sa_mask);

	if(sigaction(get_signal1(mode), &action, NULL) == -1){
		printf("Cant catch signal: %d\n", get_signal1(mode));
		return;
	}


	action.sa_handler = sigusr2_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);

	if(sigaction(get_signal2(mode), &action, NULL) == -1){
		printf("Cant catch signal: %d\n", get_signal2(mode));
		return;
	}
}

int main(int argc, char** argv) {
	if(argc != 4){
		printf("Usage: sender pid num mode\n");
		return 1;
	}

	int pid = atoi(argv[1]);
	int num = atoi(argv[2]);
	MODE mode = parse_mode(argv[3]);

	if(pid <= 0 || num < 0){
		printf("Invalid mode!\n");
		return 0;
	}

	setup_signals(mode);

	for(int i = 0; i < num; i++){
		send_signal(i, pid, mode, get_signal1(mode));

		
		while(received_sig_counter < i + 1){
			//wait for ack
		}
	}
	send_signal(0, pid, mode, get_signal2(mode));

	while(!got_last_message){

	}

	printf("Sent: %d, got %d\n", num, received_sig_counter);

	return 0;
}
