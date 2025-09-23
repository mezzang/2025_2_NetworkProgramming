// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define SEQ_START 1000

void error_handling(char *message);

typedef struct 
{
    int seq;
    int ack;
    int buf_len;
    char buf[BUF_SIZE+1];
} PACKET;


void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	int total_len=0;
    char file_name[BUF_SIZE];
    PACKET recv_packet;
    PACKET send_packet;
	
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
	
    printf("Input file name: ");
    FILE *fp = fopen(file_name, "w");

    fgets(send_packet.buf, sizeof(send_packet.buf), stdin);
    strcpy(file_name, send_packet.buf);
    printf("[Client] request %s\n", file_name);
    send_packet.seq = 0;
    send_packet.ack = 0;
    send_packet.buf_len = 0;
    write(sock, &send_packet, sizeof(send_packet));

    while(1){
        read(sock, &recv_packet, sizeof(recv_packet));
        //파일 존재하지 않음
        if(recv_packet.seq == 0 && recv_packet.ack == 0){
            printf("%s\n", recv_packet.buf);
            break;
            
        } else {
            if(recv_packet.buf_len < BUF_SIZE){
                printf("[Client] Rx SEQ: %d, len: %d bytes\n", recv_packet.seq, recv_packet.buf_len);
                send_packet.buf_len = write(fp, recv_packet.buf, recv_packet.buf_len);
                total_len += send_packet.buf_len;
                printf("%s received (%d Bytes)\n", file_name, total_len);
                break;
            }
            printf("[Client] Rx SEQ: %d, len: %d bytes\n", recv_packet.seq, recv_packet.buf_len);
            send_packet.buf_len = write(fp, recv_packet.buf, recv_packet.buf_len);
            total_len += send_packet.buf_len;
            send_packet.ack = recv_packet.seq + 1;
            write(sock, &send_packet, sizeof(send_packet));
        }
    }
    close(fp);
	printf("Exit client\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
