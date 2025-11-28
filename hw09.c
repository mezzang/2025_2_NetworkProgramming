// 2022428053 이미진
// hw09.c : Multicast 기반 채팅 프로그램

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>


#define BUF_SIZE 256
#define TTL_VAL  64

void error_handling(char *message);

int recv_sock;
struct ip_mreq join_adr;


void sigterm_handler(int signo)
{
    if (signo == SIGTERM) {
        printf("SIGTERM: Multicast Receiver terminate!\n");

        setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,(void *)&join_adr, sizeof(join_adr));
        printf("Multicast drop membership and Exit\n");

        close(recv_sock);
        printf("[Child] recv_sock closed\n");

        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int port;
    char *group_ip;
    char *user_name;

    if (argc != 4) {
        printf("Usage: ./hw07 <GroupIP> <PORT> <Name>\n");
        return 1;
    }

    group_ip  = argv[1];
    port      = atoi(argv[2]);
    user_name = argv[3];

    pid_t pid = fork();
    if (pid < 0)
        error_handling("fork() error");

    // 자식 프로세스
    if (pid == 0) {
        struct sockaddr_in adr;
        struct sigaction act;
        int str_len;
        char buf[BUF_SIZE];
        int reuse = 1;

        // SIGTERM 핸들러 등록
        act.sa_handler = sigterm_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGTERM, &act, NULL);

        recv_sock = socket(PF_INET, SOCK_DGRAM, 0);

        setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse));

        memset(&adr, 0, sizeof(adr));
        adr.sin_family = AF_INET;
        adr.sin_addr.s_addr = htonl(INADDR_ANY);
        adr.sin_port = htons(port);

        if (bind(recv_sock, (struct sockaddr *)&adr, sizeof(adr)) == -1)
            error_handling("bind() error");

        // 멀티캐스트 그룹 가입 
        join_adr.imr_multiaddr.s_addr = inet_addr(group_ip);
        join_adr.imr_interface.s_addr = htonl(INADDR_ANY);

        setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&join_adr, sizeof(join_adr));

        // 메시지 수신
        while (1) {
            str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
            if (str_len < 0)
                continue;

            buf[str_len] = 0;
            fputs(buf, stdout);
            fflush(stdout);
        }
    }

    // 부모 프로세스 : Multicast Sender
    else {
        int send_sock;
        struct sockaddr_in mul_adr;
        int time_live = TTL_VAL;
        char msg[BUF_SIZE];
        char sendbuf[BUF_SIZE + 64];

        // 송신용 소켓
        send_sock = socket(PF_INET, SOCK_DGRAM, 0);

        memset(&mul_adr, 0, sizeof(mul_adr));
        mul_adr.sin_family = AF_INET;
        mul_adr.sin_addr.s_addr = inet_addr(group_ip);
        mul_adr.sin_port = htons(port);

        // TTL 설정 
        setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&time_live, sizeof(time_live));

        // 메시지 전송
        while (1) {
            if (fgets(msg, sizeof(msg), stdin) == NULL)
                break;

            // q 또는 Q 입력 시 종료 
            if (msg[0] == 'q' || msg[0] == 'Q') {
                kill(pid, SIGTERM);          
                close(send_sock);
                printf("[Parent] send_sock closed\n");
                exit(0);
            }

            time_t t;
            time(&t);
            struct tm *p;
            p = localtime(&t);
            char timestr[16];
            sprintf(timestr, "%d:%d:%d", p->tm_hour, p->tm_min, p->tm_sec);
            

            sprintf(sendbuf, "%s [%s] %s", timestr, user_name, msg);

            sendto(send_sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&mul_adr, sizeof(mul_adr));
        }
    }

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}