
#include "common.h"


int server_socket_fd = -1;
const char *name;

void init_socket(const char *type, const char *dest){
    if(type[0] == 'l'){ //local
        struct sockaddr_un serveraddr;

        server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, dest);

        connect(server_socket_fd, (const struct sockaddr *) &serveraddr, SUN_LEN(&serveraddr));
    }else{ //network
        char ip_s[32] = {};
        char port_s[16] = {};
        char *colonPos = strchr(dest, ':');
        strncpy(port_s, colonPos + 1, sizeof(port_s));
        strncpy(ip_s, dest, (int)(colonPos - dest));

        unsigned short port = (unsigned short)atoi(port_s);
        unsigned int ip = (unsigned int)inet_addr(ip_s);
        if(ip == INADDR_NONE){
            printf("ip conv\n");
            exit(EXIT_FAILURE);
        }

        server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(server_socket_fd == -1){
            perror("Init socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in sock_info;
        sock_info.sin_family = AF_INET;
        sock_info.sin_addr.s_addr = ip;
        sock_info.sin_port = htons(port);

        int ret = connect(server_socket_fd, (struct sockaddr*)&sock_info, sizeof(struct sockaddr));
        if(ret == -1){
            perror("connect socket");
            exit(EXIT_FAILURE);
        }
    }
}

void send_message(kCommand c, const char* arg){
    char msg[MAX_MESSAGE_LEN];
    snprintf(msg, MAX_MESSAGE_LEN, "%d|%s|%s", c, name, arg == NULL ? "" : arg);
    send(server_socket_fd, msg, MAX_MESSAGE_LEN, 0);
}

void signal_handler(){
    puts("SIGNAL HANDLER\n");
    exit(0);
}

void cleanup(void){
    if(server_socket_fd != -1){
        send_message(DISCONNECT, NULL);
        close_socket(server_socket_fd);
    }

    printf("cleanup\n");
    exit(0);
}

int main(int argc, char **argv){
    if(argc != 4){
        puts("client name connection_type [ip|socket_name]");
        return 1;
    }

    atexit(cleanup);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    name = argv[1];
    char *type = argv[2];
    char *dest = argv[3];

    init_socket(type, dest);

    send_message(REGISTER_CLIENT, NULL);

    char buffer[MAX_MESSAGE_LEN] = {};
    while(1){
        if(recv(server_socket_fd, buffer, MAX_MESSAGE_LEN, 0) == 0){
            continue;
        }

        kCommand cmd = atoi(strtok(&buffer[0], "|"));
        const char *args = strtok(NULL, "|");
        printf("parsed: %d '%s'\n", cmd, (args == NULL) ? "NULL" : args);

        switch (cmd) {
            case NO_FREE_SLOTS:{
                printf("No free slots :(\n");
                exit(0);
            }
            case NAME_IN_USE:{
                printf("Name [%s] in use\n", name);
                exit(0);
            }
            case SERVER_SHUTDOWN:{
                printf("Server shutting down!\n");
                close_socket(server_socket_fd);
                _exit(0); // cleanup() cannot be used
            }

            case TIMEOUT:{
                printf("Kicked due to inactivity!\n");
                close_socket(server_socket_fd);
                _exit(0); // cleanup() cannot be used
            }

            case OPP_DISCONNECTED:{
                printf("Your opponent has disconnected\n");
                exit(0);
            }

            case START_GAME:{
                //TODO: start new thread with game routine
                break;
            }

            case PING:{
                send_message(PING, NULL);
                break;
            }

            default:
                break;
        }
    }

    return 0;
}
