#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

void sigusr1_handler(int signo, siginfo_t *info, void *context){
	if(signo != SIGUSR1){
		return;
	}

	if(info->si_code == SI_USER){
		printf("SIGUSR1 was sent using kill [real user id: %d]\n", info->si_uid);
	}
}

void test_case1(void){
	struct sigaction action;
	action.sa_sigaction = sigusr1_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if(sigaction(SIGUSR1, &action, NULL) == -1){
		printf("Cant catch SIGUSR1\n");
		return;
	}

	kill(getpid(), SIGUSR1);
}

void sigchild_handler(int signo, siginfo_t *info, void *context){
	if(signo != SIGCHLD){
		return;
	}

	printf("SIGCHLD received child exit code: %d\n", info->si_status);
}

int test_case2(void){
	struct sigaction action;
	action.sa_sigaction = sigchild_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if(sigaction(SIGCHLD, &action, NULL) == -1){
		printf("Cant catch SIGCHLD\n");
		return 0;
	}

	pid_t pid = fork();
	if(pid < 0){
		printf("fork error\n");
		return 0;
	}else if(pid == 0){
		exit(0xCC);
	}

	wait(NULL);

	return 0;
}

void sigfpe_handler(int signo, siginfo_t *info, void *context){
	if(signo != SIGFPE){
		return;
	}

	if(info->si_code == FPE_INTDIV){
		printf("SIGFPE caused by instruction at: %p\n", info->si_addr);
		exit(EXIT_FAILURE);
	}
}

void test_case3(void){
	struct sigaction action;
	action.sa_sigaction = sigfpe_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if(sigaction(SIGFPE, &action, NULL) == -1){
		printf("Cant catch SIGFPE\n");
		return;
	}

	int c = 0;
	printf("%d", 5 / c);
}

int main(int argc, char** argv) {

	test_case1();
	test_case2();
	test_case3();

	return 0;
}
