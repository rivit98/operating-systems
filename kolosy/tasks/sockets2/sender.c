#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

#define UNIX_PATH_MAX 108
#define SOCK_PATH "sock_path"

int main(int argc, char *argv[]) {
    

   if(argc !=2){
    printf("Not a suitable number of program parameters\n");
    return(1);
   }
    sleep(1);


    /*********************************************
    Utworz socket domeny unixowej typu datagramowego
    Utworz strukture adresowa ustawiajac adres/sciezke komunikacji na SOCK_PATH
    Polacz sie korzystajac ze zdefiniowanych socketu i struktury adresowej
    ***********************************************/
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd == -1){
        perror("socket");
        return 1;
    }

    struct sockaddr_un serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, SOCK_PATH, UNIX_PATH_MAX);

    int ret = connect(fd, (const struct sockaddr *) &serveraddr, SUN_LEN(&serveraddr));
    if(ret == -1){
        perror("connect");
        return 1;
    }

    char buff[20];
    int to_send = sprintf(buff, argv[1]);

    if(write(fd, buff, to_send+1) == -1) {
        perror("Error sending msg to server");
    }


    /*****************************
    posprzataj po sockecie
    ********************************/
    close(fd);
    return 0;
}

