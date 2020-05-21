#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <sys/un.h>

#include "common.h"


int remote_socket = -1;

void init_socket(const char *type, const char *dest){
    if(type[0] == 'n'){
        struct sockaddr_un serveraddr;

        remote_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, dest);

        connect(remote_socket, (const struct sockaddr *) &serveraddr, SUN_LEN(&serveraddr));
    }else{

    }
}

void signal_handler(){
    puts("SIGNAL HANDLER");
}

void cleanup(void){
    if(remote_socket != -1){
        shutdown(remote_socket, SHUT_RDWR);
        close(remote_socket);
    }

    printf("cleanup\n");
    exit(0);
}

int main(int argc, char **argv){
    if(argc != 3){
        puts("client name connection_type [ip|socket_name]");
        return 1;
    }

    atexit(cleanup);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    char *name = argv[1];
    char *type = argv[2];
    char *dest = argv[3];

    init_socket(type, dest);



    return 0;
}