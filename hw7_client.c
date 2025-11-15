// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>


#define BUF_SIZE    100

#define PING_MSG 1
#define PONG_MSG 2
#define TERMINATE_MSG 3

typedef struct
{
    int cmd;
    char time_msg[BUF_SIZE];
} PACKET;

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	
    PACKET send_packet;
    PACKET recv_packet;
	
    int delay;

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");

	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	// 연결 요청 
	if(connect(sock, (struct sockaddr*)&serv_addr, 
				sizeof(serv_addr))==-1) 
		error_handling("connect() error!");
    
    srand((unsigned int)time(NULL));

    time_t t;
    time(&t);

    struct tm *p;
    char res[50];

    while(1){
        for(int i = 0; i <10; i++){
            p = localtime(&t);
            sprintf(res, "%d-%d-%d %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
            strcpy(send_packet.time_msg, res); // 현재 시간 문자열 복사

            delay = rand() % 5 + 1;
            sleep(delay);

            send_packet.cmd = PING_MSG;
            write(sock, &send_packet, sizeof(send_packet));
            printf("[Tx] Ping(%d), sleep(%d), [%d]: %s => ", send_packet.cmd, delay, i, send_packet.time_msg);

            read(sock, &recv_packet, sizeof(recv_packet));
            if(recv_packet.cmd == PONG_MSG){
                printf("[Rx] PONG(%d), time: %s\n", recv_packet.cmd, recv_packet.time_msg);
            } else if(recv_packet.cmd == TERMINATE_MSG){
                printf("[Rx] TERMINATE(%d), Connection close.\n", recv_packet.cmd);
                break;
            }
        }
        
    }
    printf("Client close\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
