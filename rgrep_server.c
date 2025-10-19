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

int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    REQ_PACKET recv_packet;
    RES_PACKET send_packet;

    char *option;
    char *word;
    char *filename;
    char line[256];
    int line_num = 0;
    char result_line[320];
    int count;

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
    
    while(1){
        memset(&send_packet, 0, sizeof(send_packet));
        memset(&recv_packet, 0, sizeof(recv_packet));
        count = 0;
        read(clnt_sock, &recv_packet, sizeof(recv_packet));
        if(recv_packet.cmd == GREP_END){
            printf("[Rx] GREP_END(%d)\n", send_packet.cmd);
            send_packet.cmd = GREP_END_ACK;
            write(clnt_sock, &send_packet, sizeof(send_packet));
            printf("[Tx] GREP_END_ACK(%d)\n", send_packet.cmd);
            break;
        }
        printf("[Rx] GREP_REQ(%d), options: %s\n", recv_packet.cmd, recv_packet.options);
        
        option = strtok(recv_packet.options, " ");
        word = strtok(NULL, " ");
        filename = strtok(NULL, " ");

        FILE *fp = fopen(filename, "rb");
        if(fp==NULL){
            // 파일 없음
            send_packet.result = -1;
            strcpy(send_packet.matched_lines,"File not found!");
            printf("%s: %s\n", send_packet.matched_lines, filename);
            printf("-------------------------------------------\n");
            send_packet.cmd = GREP_RES;
            write(clnt_sock, &send_packet, sizeof(send_packet));
            printf("[Tx] GREP_RES(%d), result: %d\n", send_packet.cmd, send_packet.result);
            printf("-------------------------------------------\n");

        } else {
            if(strcmp(option,"-n") == 0) {
                
                size_t offset = 0;
                send_packet.matched_lines[0] = '\0';  // 처음에 비워두기

                while (fgets(line, sizeof(line), fp) != NULL) {
                    line_num++;

                    if (strstr(line, word) != NULL) {
                        count++;
                        // fgets가 넣은 개행 제거
                        size_t L = strlen(line);
                        if (L && line[L-1] == '\n') line[L-1] = '\0';
                        int n = snprintf(send_packet.matched_lines + offset,
                                        sizeof(send_packet.matched_lines) - offset,
                                        "%d: %s\n", line_num, line);
                        if (n < 0 || (size_t)n >= sizeof(send_packet.matched_lines) - offset) {
                            // 공간 부족 → 중단 또는 적절히 처리
                            break;
                        }
                        offset += (size_t)n;
                    }
                }
                send_packet.result = count;
                printf("%s", send_packet.matched_lines);
                printf("-------------------------------------------\n");

            } else if(strcmp(option, "-v") == 0){
                size_t offset = 0;
                send_packet.matched_lines[0] = '\0';  // 처음에 비워두기

                while (fgets(line, sizeof(line), fp) != NULL) {
                    line_num++;

                    if (strstr(line, word) == NULL) {
                        count++;
                        // fgets가 넣은 개행 제거
                        size_t L = strlen(line);
                        if (L && line[L-1] == '\n') line[L-1] = '\0';
                        int n = snprintf(send_packet.matched_lines + offset,
                                        sizeof(send_packet.matched_lines) - offset,
                                        "%d: %s\n", line_num, line);
                        if (n < 0 || (size_t)n >= sizeof(send_packet.matched_lines) - offset) {
                            // 공간 부족 → 중단 또는 적절히 처리
                            break;
                        }
                        offset += (size_t)n;
                    }
                }
                send_packet.result = count;
                printf("%s", send_packet.matched_lines);
                printf("-------------------------------------------\n");

            } else if(strcmp(option, "-i") == 0) {
                size_t offset = 0;
                send_packet.matched_lines[0] = '\0';  // 처음에 비워두기

                while (fgets(line, sizeof(line), fp) != NULL) {
                    line_num++;

                    if (strcasestr(line, word) != NULL) {
                        count++;
                        // fgets가 넣은 개행 제거
                        size_t L = strlen(line);
                        if (L && line[L-1] == '\n') line[L-1] = '\0';
                        int n = snprintf(send_packet.matched_lines + offset,
                                        sizeof(send_packet.matched_lines) - offset,
                                        "%d: %s\n", line_num, line);
                        if (n < 0 || (size_t)n >= sizeof(send_packet.matched_lines) - offset) {
                            // 공간 부족 → 중단 또는 적절히 처리
                            break;
                        }
                        offset += (size_t)n;
                    }
                }
                send_packet.result = count;
                printf("%s", send_packet.matched_lines);
                printf("-------------------------------------------\n");

            } else {
                // 잘못된 옵션
                send_packet.result = -2;
                strcpy(send_packet.matched_lines,"Invalid option");
                printf("%s: %s\n", send_packet.matched_lines, option);
                printf("-------------------------------------------\n");
                
            }
            send_packet.cmd = GREP_RES;
            write(clnt_sock, &send_packet, sizeof(send_packet));
            printf("[Tx] GREP_RES(%d), result: %d\n", send_packet.cmd, send_packet.result);
            printf("-------------------------------------------\n");

        }
        
    }
    printf("\nExit rGrep Server\n");
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

