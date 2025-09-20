// 2022428053 이미진
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define BUF_SIZE 100
#define TIME_REQ 1
#define TIME_RES 2
#define TIME_END 3


void error_handling(char *message);

typedef struct 
{
    int cmd;
    char time_msg[BUF_SIZE];
} PACKET;

int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;

    int str_len;

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

    PACKET recv_packet;

    while(1)
    {
        str_len = read(clnt_sock, &recv_packet, sizeof(recv_packet));
        
        if(str_len == 0) {
            printf("client closed: %d\n", clnt_sock);
            break;
        }
        if(recv_packet.cmd == TIME_REQ)
        {
            printf("[Server] Rx TIME_REQ \n");
            time_t t;
            time(&t);

            struct tm *p;
            p = localtime(&t);
            char res[50];
            sprintf(res, "%d-%d-%d %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
            
            PACKET send_packet = {0};
            send_packet.cmd = TIME_RES;
            strcpy(send_packet.time_msg, res); // 현재 시간 문자열 복사
            write(clnt_sock, &send_packet, sizeof(send_packet));
            printf("[Server] Tx TIME_RES time: %s \n", send_packet.time_msg);
        }
        else if(recv_packet.cmd == TIME_END)
        {
            printf("[Server] Rx TIME_END \n");
            printf("Server closed \n");
            break;
        }
        else
        {
            printf("Received a wrong message(cmd: %d)\n", recv_packet.cmd);
        }
    }
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