// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100

#define PING_REQ 1
#define PING_RES 2
#define PING_END 3
#define FIN_REQ 4

void error_handling(char *message);

typedef struct 
{
    int cmd;
    int count;
} REQ_PACKET;

typedef struct
{
    int cmd;
    char msg[BUF_SIZE];
} RES_PACKET;

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
    char buf[50];
	
    RES_PACKET recv_packet;
    REQ_PACKET send_packet;
    

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
	printf("Connected .....\n");
    while(1){
        printf("Enter a Ping count number (0~10): ");
        scanf("%d", &send_packet.count);
            
        if(send_packet.count < 0 || send_packet.count > 10){
            continue;
        }

        if(send_packet.count == 0) {
            send_packet.cmd = FIN_REQ;
            printf("[Tx] FIN_REQ (%d)\n", send_packet.cmd);
            write(sock,&send_packet, sizeof(send_packet));
            break;
        } else {
            send_packet.cmd = PING_REQ;
            printf("[Tx]PING_REQ (count: %d)\n", send_packet.count);
            write(sock, &send_packet, sizeof(send_packet));
            for(int i = 1; i <= send_packet.count; i++){
                read(sock, &recv_packet, sizeof(recv_packet));
                if(recv_packet.cmd = PING_RES){
                    printf("[Rx] %s\n",recv_packet.msg);
                } 
            
                
            }
            read(sock, &recv_packet, sizeof(recv_packet));
            if( recv_packet.cmd = PING_END){
                printf("[Rx]: PING_END(%d)\n", recv_packet.cmd);
                
            }
        }
    }

    
	printf("Client Terminated!\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
