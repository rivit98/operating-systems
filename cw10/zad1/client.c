#include <stdbool.h>

#include "common.h"

typedef enum {
    YOUR_MOVE = 0,
    OPP_MOVE,
    GAME_FINISHED
} kGameStatus;

int server_socket_fd = -1;
const char *name;
int opp_move = -1;
kGameStatus game_status;

#define BOARD_SIZE 3
typedef enum{
    EMPTY = 0,
    X,
    O
} kSymbol;
const char symbols_str[3] = {' ', 'X', 'O'};


kSymbol board[BOARD_SIZE][BOARD_SIZE];
kSymbol playing_as = EMPTY;

pthread_mutex_t message_loop = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t opp_made_move = PTHREAD_COND_INITIALIZER;

void init_socket(const char *type, const char *dest){
    if(type[0] == 'l'){ //local
        struct sockaddr_un serveraddr;

        server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(server_socket_fd == -1){
            perror("Init socket");
            exit(EXIT_FAILURE);
        }

        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, dest);

        int ret = connect(server_socket_fd, (const struct sockaddr *) &serveraddr, SUN_LEN(&serveraddr));
        if(ret == -1){
            perror("connect socket");
            exit(EXIT_FAILURE);
        }
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
    exit(0);
}

void cleanup(void){
    if(server_socket_fd != -1){
        send_message(DISCONNECT, NULL);
        close_socket(server_socket_fd);
    }

    exit(0);
}

bool valid_move(int idx){
    if(idx < 0 || idx >= BOARD_SIZE * BOARD_SIZE){
        return false;
    }
    int x = idx / 3;
    int y = idx % 3;

    if(board[x][y] != EMPTY){
        return false;
    }

    return true;
}

void print_board(){
    for(int x = 0; x < BOARD_SIZE; x++){
        for(int y = 0; y < BOARD_SIZE; y++){
            if(board[x][y] == EMPTY){
                printf("|%d", x * 3 + y + 1);
            }else{
                printf("|%c", symbols_str[board[x][y]]);
            }
        }
        puts("|\n-------");
    }
    puts("");
}

void make_move(int idx){
    int x = idx / 3;
    int y = idx % 3;

    board[x][y] = playing_as;
}

void *game_thread(){
    while(game_status != GAME_FINISHED){
        switch (game_status) {
            case YOUR_MOVE:{

                print_board();

                int move;
                do{
                    printf("Enter your move: ");
                    fflush(stdout);
                    scanf("%d", &move);
                    move--;
                }while(!valid_move(move));

                make_move(move);

                char move_c[3];
                sprintf(move_c, "%d", move);
                move_c[2] = '\0';
                send_message(MOVE, &move_c[0]);

                print_board();

                game_status = OPP_MOVE;
                break;
            }

            case OPP_MOVE:{
                pthread_mutex_lock(&message_loop);
                while(opp_move == -1){
                    pthread_cond_wait(&opp_made_move, &message_loop);
                }

                printf("OPP MOVE: %d\n", opp_move);
                make_move(opp_move);
                print_board();

                opp_move = -1;
                pthread_mutex_unlock(&message_loop);
                game_status = YOUR_MOVE;
                break;
            }

            default:
                break;
        }


        //TODO: check status of the game

    }

    //TODO: print winner
    //TODO: disconnect
    cleanup();

    return NULL;
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
    pthread_t game_thread_id = 0;
    while(1){
        if(recv(server_socket_fd, buffer, MAX_MESSAGE_LEN, 0) == 0){
            continue;
        }

        kCommand cmd = atoi(strtok(&buffer[0], "|"));
        const char *args = strtok(NULL, "|"); //TODO: move to move handler
//        printf("parsed: %d '%s'\n", cmd, (args == NULL) ? "NULL" : args);

        pthread_mutex_lock(&message_loop);
        switch (cmd) {
            case NO_FREE_SLOTS:{
                printf("No free slots :(\n");
                exit(0);
            }
            case NAME_IN_USE:{
                printf("Name [%s] in use\n", name);
                exit(0);
            }

            case WAITING_FOR_OPPONENT:{
                printf("Waiting for opponent\n");
                break;
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
                if(game_thread_id != 0){
                    printf("Game already started\n");
                    break;
                }

                memset(board, 0, sizeof(kSymbol) * BOARD_SIZE * BOARD_SIZE);

                if(strcmp(args, "X") == 0){
                    game_status = YOUR_MOVE;
                    playing_as = X;
                }else{
                    game_status = OPP_MOVE;
                    playing_as = O;
                }
                printf("You are playing as %c\n", symbols_str[playing_as]);

                pthread_create(&game_thread_id, NULL, game_thread, NULL);
                break;
            }

            case MOVE:{
                opp_move = atoi(args);
                pthread_cond_signal(&opp_made_move);
                break;
            }

            case PING:{
                send_message(PING, NULL);
                break;
            }

            default:
                break;
        }
        pthread_mutex_unlock(&message_loop);
    }

    return 0;
}
