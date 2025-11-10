// 2022428053 이미진
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define BUF_SIZE    100

#define MSG_REQ     1   // Client -> Server
#define MSG_RES     2   // Server -> Client
#define MSG_ACK     3   // Client -> Server
#define MSG_RETRANS 4   // Client -> Server
#define MSG_END     5   // Client -> Server

// 클라이언트에서 사용하는 요청 패킷
typedef struct
{
    int cmd;   // 메시지 종류(MSG_REQ, MSG_ACK, MSG_RETRANS, MSG_END 등)
    int ack;   // 서버로부터 받은 seq 번호에 대한 ACK 등으로 사용
} REQ_PACKET;

// 서버에서 사용하는 응답 패킷
typedef struct
{
    int cmd;   // MSG_RES 등
    int seq;   // 서버가 보내는 패킷의 시퀀스 번호
} RES_PACKET;


void error_handling(char *message);


int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;

    REQ_PACKET req;
    RES_PACKET res;
    int seq = 0;                  // 서버에서 관리하는 시퀀스 번호
    int delay;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

    srand((unsigned int)time(NULL));

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

  

    while (1) {
        int n = recv(clnt_sock, &req, sizeof(req), 0);
        

        switch (req.cmd) {
        case MSG_REQ:

            delay = rand() % 5 + 1;   // 1~5
            seq = req.ack;                 // 새 SEQ

            sleep(delay);

            res.cmd = MSG_RES;
            res.seq = seq;
            send(clnt_sock, &res, sizeof(res), 0);

            printf("[Rx] MSG_REQ (ACK:%d) ---> [Tx] MSG_RES (SEQ: %d, delay: %d)\n",req.ack, seq, delay);

            break;

        case MSG_ACK:
            delay = rand() % 5 + 1;
            seq = req.ack;     // 다음 SEQ
            
            sleep(delay);

            res.cmd = MSG_RES;
            res.seq = seq;
            send(clnt_sock, &res, sizeof(res), 0);

            printf("[Rx] MSG_ACK (ACK:%d) ---> [Tx] MSG_RES (SEQ: %d, delay: %d)\n",req.ack, seq, delay);

            break;

        case MSG_RETRANS:
            seq = req.ack;   // 클라이언트가 기대하는 번호로 설정
            res.cmd = MSG_RES;
            res.seq = seq;
            send(clnt_sock, &res, sizeof(res), 0);
            printf("[Rx] MSG_RETRANS (ACK:%d) ---> [Tx] MSG_RES (SEQ: %d)\n",req.ack, seq);
            break;

        case MSG_END:
            printf("[Rx] MSG_END\n");
            printf("Server Exit\n");
            close(clnt_sock);
            close(serv_sock);
            return 0;

        
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