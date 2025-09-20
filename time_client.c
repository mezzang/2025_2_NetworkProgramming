// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define TIME_REQ 1
#define TIME_RES 2
#define TIME_END 3

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
	int str_len;
    char message[BUF_SIZE];
	
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
	while(1){
        printf("Type a message(time or q): ");
        fgets(message, sizeof(message), stdin);
        message[strlen(message)-1]=0;
        if(!strcmp(message,"q")||!strcmp(message,"Q")){
            PACKET packet = {0};
            packet.cmd=TIME_END;
            write(sock, &packet, sizeof(packet));
            printf("[Client] Tx TIME_END \n");
            printf("Exit client\n");
            break;
        }
        else if(!strcmp(message,"time")){
            PACKET packet;
            packet.cmd=TIME_REQ;
            printf("[Client] Tx TIME_REQ \n");
            write(sock, &packet, sizeof(packet));
            read(sock, &packet, sizeof(packet));
            if(packet.cmd==TIME_RES){
                printf("[Client] Rx TIME_RES: %s \n", packet.time_msg);
                }
        }
        else{
            printf("Wrong message.\n");
        }

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
