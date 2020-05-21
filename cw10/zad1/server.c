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


int network_socket_fd = -1;
int local_socket_fd = -1;


void signal_handler(){
    puts("SIGNAL HANDLER");
}

void cleanup(void){
    if(network_socket_fd != -1){
        shutdown(network_socket_fd, SHUT_RDWR);
        close(network_socket_fd);
    }

    if(local_socket_fd != -1){
        shutdown(local_socket_fd, SHUT_RDWR);
        close(local_socket_fd);
    }

    printf("cleanup\n");
    exit(0);
}

void init_local_socket(const char *path){
    struct sockaddr_un serveraddr;

    local_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, path);

    unlink(path);

    bind(local_socket_fd, (const struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
}

void init_network_socket(const char * port){
    struct addrinfo hints, *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    getaddrinfo(NULL, port, &hints, &result);

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        network_socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (network_socket_fd == -1)
            continue;

        if (bind(network_socket_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(network_socket_fd);
    }

    freeaddrinfo(result);
}

void init_sockets(const char * port, const char *path){
    init_local_socket(path);
    init_network_socket(port);

    if(network_socket_fd != -1){
        listen(network_socket_fd, MAX_CONNECTIONS);
    }

    if(local_socket_fd != -1){
        listen(local_socket_fd, MAX_CONNECTIONS);
    }
}

int main(int argc, char **argv){
    if(argc != 3){
        puts("server port socket_name");
        return 1;
    }

    atexit(cleanup);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    char *port = argv[1];
    char *socket_path = argv[2];

    init_sockets(port, socket_path);
    printf("net: %d local: %d\n", network_socket_fd, local_socket_fd);

    struct sockaddr_in desc;
    socklen_t size;
    while(1){
        int remote = accept(network_socket_fd, NULL, NULL);
        printf("Remote: %d", remote);
        break;


    }



    return 0;
}