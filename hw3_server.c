// 2022428053 이미진
#include <stdio.h>
#include <time.h>
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

int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;
    char file_name[BUF_SIZE];
    int total_len=0;

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
    PACKET send_packet;


    printf("-----------------------------\n");
    printf("  File Transmission Server  \n");
    printf("-----------------------------\n");

    
    read(clnt_sock, &recv_packet, sizeof(recv_packet));
    strcpy(file_name, recv_packet.buf);
    FILE *fp = fopen(file_name, "r");
    if(fp==NULL){
        send_packet.seq = 0;
        send_packet.ack = 0;
        strcpy(send_packet.buf, "File Not Found");
        send_packet.buf_len = strlen(send_packet.buf);
        write(clnt_sock, &send_packet, sizeof(send_packet));
        printf("%s File Not Found\n", file_name);
        
        
    } else {
        printf("[Server] sending %s\n", file_name);
        printf("\n");
        memset(send_packet.buf, 0, sizeof(send_packet.buf));
        send_packet.seq = SEQ_START;
        
        while(1){
            send_packet.buf_len = read(fp, send_packet.buf, sizeof(send_packet.buf));
            total_len += send_packet.buf_len;
            printf("[Server] Tx: SEQ: %d, %d byte data\n", send_packet.seq, send_packet.buf_len);
            write(clnt_sock, &send_packet, sizeof(send_packet));
            if(send_packet.buf_len < BUF_SIZE){
                printf("%s sent(%d Bytes)\n", file_name, total_len);
                break;
            }
            read(clnt_sock, &recv_packet, sizeof(recv_packet));
            printf("[Server]: Rx ACK: %d\n", recv_packet.ack);
            send_packet.seq = recv_packet.ack;
        }
        
        
    }

    fclose(fp);
    printf("Exit server\n");
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