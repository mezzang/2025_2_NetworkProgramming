// 2022428053 이미진
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>


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
void read_childproc(int sig);


int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    pid_t pid;

    PACKET send_packet;
    PACKET recv_packet;

    time_t t;
    struct tm *p;

    time_t start, end;
    int delay = 0;

    struct sigaction act;
    int state = 0;

    // signal handler 등록
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

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
	

    while(1)
    {
        clnt_addr_size=sizeof(clnt_addr);  
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
        if(clnt_sock==-1)
            continue;
        else 
            printf("new client connected : %d (Port: %d)\n", clnt_sock, ntohs(clnt_addr.sin_port));
        pid = fork();
        if(pid == -1){
            close(clnt_sock);
            continue;
        }

        if(pid == 0){
            close(serv_sock);
            // 최초 한 번
            read(clnt_sock, &recv_packet, sizeof(recv_packet));
            printf("[Rx] PING(%d) time: %s from port(%d) => ", recv_packet.cmd, recv_packet.time_msg, ntohs(clnt_addr.sin_port));
            send_packet.cmd = PONG_MSG;
            time(&t);
            p = localtime(&t);
            sprintf(send_packet.time_msg, "%d-%d-%d %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
            write(clnt_sock, &send_packet, sizeof(send_packet));
            printf("[Tx] PONG(%d), time: %s to Port(%d)\n", send_packet.cmd, send_packet.time_msg, ntohs(clnt_addr.sin_port));
            while(1){
                start = time(NULL);
                read(clnt_sock, &recv_packet, sizeof(recv_packet));
                end = time(NULL);
                printf("[Rx] PING(%d) time: %s from port(%d) => ", recv_packet.cmd, recv_packet.time_msg, ntohs(clnt_addr.sin_port));
                delay = difftime(end, start);
                if(delay <= 4){
                    send_packet.cmd = PONG_MSG;
                    time(&t);
                    p = localtime(&t);
                    sprintf(send_packet.time_msg, "%d-%d-%d %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
                    write(clnt_sock, &send_packet, sizeof(send_packet));
                    printf("[Tx] PONG(%d), time: %s to Port(%d)\n", send_packet.cmd, send_packet.time_msg, ntohs(clnt_addr.sin_port));
                    
                } else{
                    send_packet.cmd = TERMINATE_MSG;
                    time(&t);
                    p = localtime(&t);
                    sprintf(send_packet.time_msg, "%d-%d-%d %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
                    write(clnt_sock, &send_packet, sizeof(send_packet));
                    printf("[Tx] TERMINATE(%d), time: %s to Port(%d)\n", send_packet.cmd, send_packet.time_msg, ntohs(clnt_addr.sin_port));
                    break;
                }
                    
            }
            printf("Client disconnected.(Port: %d)\n", ntohs(clnt_addr.sin_port));
            close(clnt_sock);
            return 0;
        }
        else {
            close(clnt_sock);
        }
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

void read_childproc(int sig){
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}