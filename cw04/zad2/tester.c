#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


typedef enum{
	IGNORE = 0,
	MASK,
	PENDING
} MODE;

MODE get_mode(const char *arg){
	if(strcmp(arg, "ignore") == 0){
		return IGNORE;
	}else if(strcmp(arg, "mask") == 0){
		return MASK;
	}else if(strcmp(arg, "pending") == 0){
		return PENDING;
	}else{
		printf("Bad tester arg!\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char** argv) {
	MODE mode = get_mode(argv[1]);

	switch(mode){
		case IGNORE:
			raise(SIGUSR1); //this should be ignored
			printf("SIGUSR1 ignored in child\n");
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


	return 0;
}
