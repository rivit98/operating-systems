#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <wait.h>


typedef enum{
	IGNORE = 0,
	HANDLER,
	MASK,
	PENDING
} MODE;

MODE get_mode(const char *arg){
	if(strcmp(arg, "ignore") == 0){
		return IGNORE;
	}else if(strcmp(arg, "handler") == 0){
		return HANDLER;
	}else if(strcmp(arg, "mask") == 0){
		return MASK;
	}else if(strcmp(arg, "pending") == 0){
		return PENDING;
	}else{
		printf("Bad arg!\n");
		exit(EXIT_FAILURE);
	}
}

void sigusr1_handle(int sigint) {
    printf("SIGUSR1 received\n");
}

int main(int argc, char** argv) {
	if(argc != 3){
		printf("Usage: main [ignore|handler|mask|pending] [fork|exec]\n");
		return 1;
	}
	MODE mode = get_mode(argv[1]);

	switch(mode){
		case IGNORE:
			signal(SIGUSR1, SIG_IGN);
			raise(SIGUSR1);	//this should be ignored
			break;

		case HANDLER:
			signal(SIGUSR1, sigusr1_handle);
			raise(SIGUSR1);
			break;

		case MASK:
		case PENDING:
			; //stupid compiler
			sigset_t block_mask;
			sigemptyset(&block_mask);
			sigaddset(&block_mask, SIGUSR1);
			sigprocmask(SIG_BLOCK, &block_mask, NULL);
			raise(SIGUSR1); //this should be ignored (added to signal queue)
			if(mode == PENDING){
				sigset_t current_signals;
				sigpending(&current_signals);
				printf("SIGUSR1 pending in parent: %s\n", sigismember(&current_signals, SIGUSR1) ? "yes" : "no");
			}
			break;
	}

	if(strcmp(argv[2], "fork") == 0){
		pid_t child = fork();
		if(child == 0){
			switch(mode){
				case IGNORE:
					raise(SIGUSR1); //this should be ignored
					printf("SIGUSR1 ignored in child\n");
					break;
				
				case HANDLER:
					raise(SIGUSR1);
					break;

				case MASK:
					raise(SIGUSR1); //this should be ignored
					printf("SIGUSR1 masked in child\n");
					break;

				case PENDING:
					;
					sigset_t current_signals;
					sigpending(&current_signals);
					printf("SIGUSR1 pending in child: %s\n", sigismember(&current_signals, SIGUSR1) ? "yes" : "no");
			}
		}else if(child > 0){
			wait(0);
		}

	}else if(strcmp(argv[2], "exec") == 0){
		execl("./tester", "./tester", argv[1], NULL);
	}else{
		printf("Bad arg!\n");
		return 1;
	}

	return 0;
}
