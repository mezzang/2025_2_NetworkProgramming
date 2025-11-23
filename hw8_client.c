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
#include <fcntl.h> // open 함수

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

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
	int str_len;
    char role[10];
	struct sockaddr_in serv_adr;
    

    PACKET send_packet;
    PACKET recv_packet;

    char buf[BUF_SIZE];
    struct timeval timeout;


    if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error\n");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!\n");
	else
		puts("Connected...........\n\n");
        
    printf("---------------------------\n");
    printf("Choose function\n");
    printf("1. Sender   2. Receiver\n");
    printf("---------------------------\n");

    fgets(role, sizeof(role), stdin);

    memset(&send_packet, 0, sizeof(send_packet)); 

    printf("Connected....\n");

    if(!strcmp(role,"1")){
        printf("File Sender start!\n");
        int fd2 = open("test.txt", O_RDONLY);
        if(fd2 < 0){
            printf("File not Found.\n");
            exit(1); 
        } else {
            while(1){
                fd_set reads, temps;
                struct timeval timeout;

                FD_ZERO(&reads);
                FD_SET(fd2, &reads);

                timeout.tv_sec = 3;
                timeout.tv_usec = 0;
                
                temps = reads;
                int result = select(fd2 + 1, &temps, NULL, NULL, &timeout);
                if(result < 0){
                    break;
                } else if (result == 0){
                    continue; //timeout
                }
                if(FD_ISSET(fd2, &temps)){
                    int str_len = read(fd2, send_packet.buf, BUF_SIZE);
                    if (str_len == 0) {
                        // EOF → 파일 끝
                        send_packet.cmd = TERMINATE;
                        write(sock, &send_packet, sizeof(send_packet));
                        close(fd2);
                        break;
                    } else {
                        send_packet.cmd = CMD_SEND;
                        write(sock, &send_packet, sizeof(send_packet));
                        printf("[Tx] File Read len: %d\n", str_len);
                    }
                } 
            }
                
        }              
    } else if (!strcmp(role,"2")){
        while(1){
            fd_set reads, temps;
            struct timeval timeout;

            FD_ZERO(&reads);
            FD_SET(sock, &reads);

            timeout.tv_sec = 3;
            timeout.tv_usec = 0;

            temps = reads;
            int result = select(sock + 1, &temps, NULL, NULL, &timeout);
            if( result < 0)
                break;
            else if (result == 0)
                continue;
            if(FD_ISSET(sock, &temps)){
                read(sock, &recv_packet, sizeof(recv_packet));
                if(recv_packet.cmd == MSG_SEND){
                    printf("%s", recv_packet.buf);
                } else if(recv_packet.cmd == TERMINATE){
                    printf("%s", recv_packet.buf);
                    printf("[Rx] TERMINATE(%d)\n",recv_packet.cmd);
                    break;
                }
            }
            
        }
    }
    printf("Socket Close (%d)\n", sock);
    printf("Exit Client.\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
    
