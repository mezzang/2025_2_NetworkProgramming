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
    int sock;
    char client_board[BOARD_SIZE][BOARD_SIZE];
    REQ_PACKET send_packet;
    RES_PACKET recv_packet;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;

    if(argc!=3){
        exit(1);
        printf("Usage : %s <IP> <port>\n", argv[0]);
    }

    sock=socket(PF_INET, SOCK_DGRAM, 0);
    if(sock==-1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    printf("---------------------------------\n");
    printf("   Finding Alphabet Game Server  \n");
    printf("---------------------------------\n");

    for(int i = 0; i <BOARD_SIZE; i++){
        for(int j = 0; j <  BOARD_SIZE; j++){
            client_board[i][j] = ' ';
        }
    }

    srand(time(NULL));

    while(1)
    {   
        
        send_packet.cmd = GAME_REQ;
        send_packet.ch = 'A' + (rand() % 26);

        sendto(sock, &send_packet, sizeof(send_packet), 0,(struct sockaddr*)&serv_adr, sizeof(serv_adr));
        printf("[Client] Tx cmd = %d, ch = %c\n", send_packet.cmd, send_packet.ch);

        adr_sz = sizeof(from_adr);
        recvfrom(sock, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&from_adr, &adr_sz);

        printf("[Client] Rx cmd = %d, result = %d\n", recv_packet.cmd, recv_packet.result);

        if(recv_packet.cmd == GAME_END){
            printf("No empty space. Exit this program.\n");
            printf("Exit Client Program\n");
            break;
        }

        
        for(int i=0; i< BOARD_SIZE; i++){
            printf("+---------+\n");
            for(int j = 0; j <BOARD_SIZE; j++){
                client_board[i][j] = recv_packet.board[i][j];
                printf("|%c", client_board[i][j]);
            }
            printf("|\n");
        }
        printf("+---------+\n");
        
        sleep(1);
    }   
    
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}