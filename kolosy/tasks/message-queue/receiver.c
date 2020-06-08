#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <asm/errno.h>


#define KEY  "./queuekey"


int main() {
    sleep(1);
    int val = 0;


    /**********************************
    Otworz kolejke systemu V "reprezentowana" przez KEY
    **********************************/
    key_t k = ftok(KEY, 'a');
    if(k == -1){
        perror("ftok");
        return 1;
    }
    int msgid = msgget(k, 0666);
    if(msgid == -1){
        perror("msgget");
        return 1;
    }

    char buffer[256];
    while (1) {
        /**********************************
        Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
        obowiazuja funkcje systemu V
        ************************************/

        msgrcv(msgid, buffer, sizeof(buffer), 0, IPC_NOWAIT);

        if(errno == ENOMSG){
            continue;
        }

        val = atoi(buffer);
        printf("%d square is: %d\n", val, val * val);
        break;
    }

    /*******************************
    posprzataj
    ********************************/

    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}
