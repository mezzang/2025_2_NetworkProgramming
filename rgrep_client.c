// 2022428053 이미진
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define GREP_REQ 1
#define GREP_RES 2
#define GREP_END 3
#define GREP_END_ACK 4

typedef struct 
{
    int cmd;
    char options[100];
} REQ_PACKET;

typedef struct
{
    int cmd;
    int result;
    char matched_lines[2048];
} RES_PACKET;

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;

    REQ_PACKET send_packet;
    RES_PACKET recv_packet;

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
	while(1) {
        printf("Type [option] [keyword] [filename]: ");
        fgets(send_packet.options, sizeof(send_packet.options), stdin);

        if(strcmp(send_packet.options,"quit") == 0){
            send_packet.cmd = GREP_END;
            printf("[Tx] GREP_END(%d)\n", send_packet.cmd);
            write(sock, &send_packet, sizeof(send_packet));
            read(sock, &recv_packet, sizeof(recv_packet));
            printf("[Rx] GREP_END_ACK(%d)\n", recv_packet.cmd);
            break;

        }
        else {
        send_packet.cmd = GREP_REQ;
        write(sock, &send_packet, sizeof(send_packet));
        printf("[Tx] grep_req(%d) options: %s\n", send_packet.cmd, send_packet.options);
        printf("-------------------------------------------\n");
        read(sock, &recv_packet, sizeof(recv_packet));
        printf("[Rx] GREP_RES(%d), result: %d\n", recv_packet.cmd, recv_packet.result);
        printf("-------------------------------------------\n");
        if(recv_packet.result == 0){
            printf("-------------------------------------------\n");
        } else {
            printf("%s", recv_packet.matched_lines);
        }
        printf("-------------------------------------------\n");

        }

        

    }
    printf("\nExit rGrep client\n");
    close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
