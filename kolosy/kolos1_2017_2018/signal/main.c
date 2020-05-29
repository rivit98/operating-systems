#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int signo, siginfo_t *info, void *ctx){
    (void) ctx;
    if(signo == SIGUSR1){
        printf("%d\n", info->si_value.sival_int);
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(atoi(argv[2]), &action, NULL);

    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        sigset_t set;
        sigfillset(&set);
        sigdelset(&set, SIGUSR1);
        sigprocmask(SIG_BLOCK, &set, NULL);

    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        sigqueue(child, atoi(argv[2]), (const union sigval){.sival_int = atoi(argv[1])});
    }

    return 0;
}
