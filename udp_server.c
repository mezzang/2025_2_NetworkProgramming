// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <stdlib.h>


#define BOARD_SIZE 5
#define GAME_REQ 1
#define GAME_RES 2
#define GAME_END 3

typedef struct {
    int cmd;
    char ch;
} REQ_PACKET;

typedef struct {
    int cmd;
    char board[BOARD_SIZE][BOARD_SIZE];
    int result;
} RES_PACKET;

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock;
    char board[BOARD_SIZE][BOARD_SIZE];
    char result_board[BOARD_SIZE][BOARD_SIZE];
    REQ_PACKET recv_packet;
    RES_PACKET send_packet;

    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;

    if(argc!=2){
        
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
    if(serv_sock==-1)
        error_handling("UDP socket creation error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");

    srand(time(NULL));
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j <BOARD_SIZE; j++){
            board[i][j] = 'A' + (rand() % 26);
            result_board[i][j] = ' ';
        }
    }

    printf("---------------------------------\n");
    printf("   Finding Alphabet Game Server  \n");
    printf("---------------------------------\n");
    for(int i = 0; i < BOARD_SIZE; i++){
        printf("+---------+ +---------+\n");
        for(int j = 0; j < BOARD_SIZE; j++){
            printf("|%c",board[i][j]);
        }
        printf("| ");
        for(int k = 0; k < BOARD_SIZE; k++){
            printf("|%c", result_board[i][k]);
        }
        printf("|\n");
    }
    printf("+---------+ +---------+\n");

    int filled = 0;
    

    while(1)
    {
        int result = 0;

        clnt_adr_sz = sizeof(clnt_adr);
        recvfrom(serv_sock, &recv_packet, sizeof(recv_packet),0,(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        printf("[Server] Rx cmd = %d, ch = %c\n", recv_packet.cmd, recv_packet.ch);
        if(filled == BOARD_SIZE*BOARD_SIZE){
            send_packet.cmd = GAME_END;
            send_packet.result = 0;
            sendto(serv_sock, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
            printf("[Server] Tx cmd = %d result = %d\n", send_packet.cmd, send_packet.result);
            printf("No empty space. Exit this program.\n");
            printf("Exit Server Program\n");
            break;
        }

        char target = recv_packet.ch;

        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                if(board[i][j] == target && result_board[i][j] == ' '){
                    result++;
                    filled++;
                    result_board[i][j] = target;
                } 
            }
        }

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                send_packet.board[i][j] = result_board[i][j];
            }
        }

        send_packet.cmd = GAME_RES;
        send_packet.result = result;

        sendto(serv_sock, &send_packet, sizeof(send_packet), 0,(struct sockaddr*)&clnt_adr, clnt_adr_sz);
        printf("[Server] Tx cmd = %d result = %d\n", send_packet.cmd, send_packet.result);
        for(int i = 0; i < BOARD_SIZE; i++){
            printf("+---------+ +---------+\n");
            for(int j = 0; j < BOARD_SIZE; j++){
                printf("|%c",board[i][j]);
            }
            printf("| ");
            for(int k = 0; k < BOARD_SIZE; k++){
                printf("|%c", result_board[i][k]);
            }
            printf("|\n");
        }
        printf("+---------+ +---------+\n");
        sleep(1);
        
        
    }

    
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}