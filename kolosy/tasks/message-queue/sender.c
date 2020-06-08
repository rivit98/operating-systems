#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define KEY "./queuekey"

struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }

    /******************************************************
    Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
    Wyslij do niej wartosc przekazana jako parametr wywolania programu
    Obowiazuja funkcje systemu V
    ******************************************************/

    key_t k = ftok(KEY, 'a');
    if(k == -1){
        perror("ftok");
        return 1;
    }
    int msgid = msgget(k, IPC_CREAT | 0666);
    if(msgid == -1){
        perror("msgget");
        return 1;
    }

    struct msgbuf m;
    m.mtype = 1;
    strcpy(m.mtext, argv[1]);

    int ret = msgsnd(msgid, &m, strlen(argv[1]) - sizeof(long), 0);
    if(ret == -1){
        perror("msgsnd");
        return 1;
    }

    return 0;
}



