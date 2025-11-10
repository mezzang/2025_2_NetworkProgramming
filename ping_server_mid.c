// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>


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


int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;
    
    RES_PACKET send_packet;
    REQ_PACKET recv_packet;
    char buf[50];

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	/* 서버 소켓(리스닝 소켓) 생성 */
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	/* 주소 정보 할당 */
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 )
		error_handling("bind() error"); 
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	clnt_addr_size=sizeof(clnt_addr);  
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error"); 
    printf("Connected client_sock : %d\n", clnt_sock);


    printf("Ping Server Program\n");
    
    while(1){
        memset(&send_packet,0, sizeof(send_packet));
        memset(&recv_packet, 0, sizeof(recv_packet));
        read(clnt_sock, &recv_packet, sizeof(recv_packet));
        if(recv_packet.cmd == FIN_REQ) {
            printf("[Rx] FIN_REQ (%d)\n", recv_packet.cmd);
            break;

            
        } else if ( recv_packet.cmd == PING_REQ) {
            printf("[Rx] PING_REQ (%d)\n", recv_packet.cmd);
            printf("--------------------------\n");
            int num = recv_packet.count;
            
            for (int i = 1; i <= num; i++){
                time_t t;
                time(&t);

                struct tm *p;
                p = localtime(&t);
                sprintf(send_packet.msg, "Ping(%d:%d:%d) count: %d\n", p->tm_hour, p->tm_min, p->tm_sec, i);
                send_packet.cmd = PING_RES;
                write(clnt_sock, &send_packet, sizeof(send_packet));
                printf("[Tx] PING_RES: %s\n", send_packet.msg);
                sleep(1);
                
            }
            send_packet.cmd = PING_END;
            printf("[Tx] PING_END(%d)\n", send_packet.cmd);
            write(clnt_sock, &send_packet, sizeof(send_packet));
            sleep(1);
        } 
    }
    
        
    
    printf("Server Terminated!\n");
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}