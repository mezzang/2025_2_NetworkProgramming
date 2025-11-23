// 2022428053 이미진
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#define BUF_SIZE 1024
#define SENDER 1
#define RECEIVER 2

#define CMD_SEND 1
#define TERMINATE 2

typedef struct
{
int cmd;
char buf[BUF_SIZE+1];
}PACKET;

void error_handling(char * message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    int fd_max;
    int fd_num;
	int client_cnt = 0; 
	int end_server = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    fd_set reads, cpy_reads;
    int str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;

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
    
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads); 
    fd_max = serv_sock;


    while(1){
        cpy_reads = reads;

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)
			break;
		
		if(fd_num==0)
			continue;

		for(int i=0; i<fd_max+1; i++)
		{
			if(FD_ISSET(i, &cpy_reads))
			{
				if(i==serv_sock)     // connection request!
				{
					clnt_addr_size=sizeof(clnt_addr);
					clnt_sock=
						accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
					FD_SET(clnt_sock, &reads);
					if(fd_max<clnt_sock)
						fd_max=clnt_sock;
					client_cnt++;
					printf("connected client: %d fd_max: %d\n", clnt_sock, fd_max);
				}
				else    // read message!
				{	
					PACKET recv_packet;

					str_len = read(i, &recv_packet, sizeof(PACKET));
					if(str_len==0)    // close request!
					{
						FD_CLR(i, &reads);
						close(i);
						client_cnt--;
						printf("closed client: %d \n", i);
						if (client_cnt == 0) {
							end_server = 1;
							break;  
						}
					}
					else
					{
						// i번 클라이언트에서 온 패킷을 다른 모든 클라이언트에게 그대로 전달
						if(recv_packet.cmd == CMD_SEND){
							for (int j = 0; j <= fd_max; j++)
							{
								if (FD_ISSET(j, &reads)) {
									// 서버 소켓/자기 자신은 제외
									if (j != serv_sock && j != i) {
										write(j, &recv_packet, sizeof(recv_packet));
										printf("Forward  client [%d] ---> client [%d] (%d Bytes)\n",i, j, str_len);
									}
								}
							}
						} else if(recv_packet.cmd == TERMINATE){
							for (int j = 0; j <= fd_max; j++)
							{
								if (FD_ISSET(j, &reads)) {
									// 서버 소켓/자기 자신은 제외
									if (j != serv_sock && j != i) {
										write(j, &recv_packet, sizeof(recv_packet));
										printf("[Tx] TERMINATE to [%d]\n",j);
										
									}
								}
							}
						}
					}
				}
			}
		}
		if(end_server)
			break;
	}
	printf("Server Closed.\n");
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}