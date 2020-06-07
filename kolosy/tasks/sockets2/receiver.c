#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define UNIX_PATH_MAX 108
#define SOCK_PATH "sock_path"


int main() {
    /*********************************************
    Utworz socket domeny unixowej typu datagramowego
    Utworz strukture adresowa ustawiajac adres/sciezke komunikacji na SOCK_PATH
    Zbinduj socket z adresem/sciezka SOCK_PATH
    **********************************************/
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd == -1){
        perror("socket");
        return 1;
    }

    struct sockaddr_un serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, SOCK_PATH, UNIX_PATH_MAX);
    bind(fd, (const struct sockaddr *) &serveraddr, SUN_LEN(&serveraddr));

    char buf[20];
    if(read(fd, buf, 20) == -1){
        perror("Error receiving message");
        return 1;
    }
    int val = atoi(buf);
    printf("%d square is: %d\n",val,val*val);

    /***************************
    Posprzataj po sockecie
    ****************************/
    shutdown(fd, SHUT_RDWR);
    close(fd);
    unlink(SOCK_PATH);
    return 0;
}

