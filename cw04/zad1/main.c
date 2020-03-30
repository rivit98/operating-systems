#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>


bool pause_loop = false;

void sigtstp_handler(int signo){
	if(signo == SIGTSTP){
		pause_loop = !pause_loop;
		if(pause_loop){
			puts("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu");
		}
	}
}

void sigint_handler(int signo){
	if(signo == SIGINT){
		puts("Odebrano sygnal SIGINT");
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char** argv) {
	struct sigaction action;
	action.sa_handler = sigtstp_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if(sigaction(SIGTSTP, &action, NULL) == -1){
		printf("Cant catch SIGTSTP");
		return 1;
	}

    if(signal(SIGINT, sigint_handler)){
		printf("Cant catch SIGINT");
		return 1;
	}
	
	while(1){
		if(!pause_loop){
			system("ls");
		}
		sleep(1);
	}

	return 0;
}
