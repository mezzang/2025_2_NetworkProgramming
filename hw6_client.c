// 2022428053 이미진
// TCP	통신을 이용한 수신 타임아웃 설정 및 패킷 재전송 기능
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>


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

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");

    struct timeval rcv_timeout = {3, 0};    // 3 seconds
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                   &rcv_timeout, sizeof(rcv_timeout)) < 0) {
        perror("setsockopt");
        close(sock);
        exit(1);
    }
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	// 연결 요청 
	if(connect(sock, (struct sockaddr*)&serv_addr, 
				sizeof(serv_addr))==-1) 
		error_handling("connect() error!");
    
    int total_packets;
    printf("Enter a number of requesting packet (1~10): ");
    scanf("%d", &total_packets);

    if (total_packets < 1 || total_packets > 10) {
        printf("잘못된 값입니다.\n");
        close(sock);
        return 0;
    }

    REQ_PACKET req;
    RES_PACKET res;

	int recv_count   = 0;   // 정상 수신한 패킷 수
    int timeout_cnt  = 0;   // 타임아웃 횟수
    int last_seq    = -1;

    // 최초 MSG_REQ 전송
    req.cmd = MSG_REQ;
    req.ack = 0;
    send(sock, &req, sizeof(req), 0);
    printf("[Tx] MSG_REQ (ACK=%d)\n", req.ack);

    // 반복 수신/응답
    while (recv_count < total_packets) {
        int n = recv(sock, &res, sizeof(res), 0);

        if (n > 0) {   // 정상 수신
            if (res.cmd != MSG_RES) {
                // 방어 코드
                continue;
            }

            recv_count++;
            last_seq = res.seq;

            /* 마지막 패킷이면 MSG_END 전송 */
            if (recv_count == total_packets) {
                req.cmd = MSG_END;
                req.ack = 0;

                printf("[Rx] MSG_RES (SEQ: %d) rx_count: %d --> [Tx] MSG_END\n",
                       res.seq, recv_count);

                send(sock, &req, sizeof(req), 0);
                break;
            }
            else {
                /* 다음에 기대하는 SEQ 번호 = 현재 seq + 1 */
                req.cmd = MSG_ACK;
                req.ack = res.seq + 1;

                printf("[Rx] MSG_RES (SEQ: %d) rx_count: %d --> [Tx] MSG_ACK (ACK: %d)\n",
                       res.seq, recv_count, req.ack);

                send(sock, &req, sizeof(req), 0);
            }
        }
        else if (n == 0) {   // 서버가 연결 종료
            printf("Server closed connection.\n");
            break;
        }
        else {               // n < 0  
            /* ★ 수신 타임아웃 발생 → 재전송 요청 */
            timeout_cnt++;
            printf("\nReceive Timeout!. Request Retransmission\n");

            req.cmd = MSG_RETRANS;
            req.ack = last_seq + 1;    // 수신 예정 SEQ 번호

            printf("[Tx] MSG_RETRAN (ACK: %d)\n", req.ack);

            send(sock, &req, sizeof(req), 0);
            
            
        }
    }
    printf("Timeout count: %d\n", timeout_cnt);
    printf("Client Exit\n");

	
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
