// 멀티 프로세스(ch10)
// 다중 접속 에코 서버의 구현  - fork(), SIGCHLD
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    pid_t pid;
    struct sigaction act;
    socklen_t adr_sz;
    int str_len;
    char buf[BUF_SIZE];

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    /* SIGCHLD 시그널 등록 (좀비 프로세스 방지) */
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    /* 서버 소켓 생성 */
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1)
    {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
        if (clnt_sock == -1)
            continue;

        printf("new client connected: %d\n", clnt_sock);

        pid = fork();
        if (pid == -1)
        {
            close(clnt_sock);
            continue;
        }

        if (pid == 0)   /* Child process */
        {
            close(serv_sock);
            // 자식 프로세스는 연결 설정을 위한 serv_sock이 필요가 없다.
            while ((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0)
            {
                write(clnt_sock, buf, str_len);
            }
            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }
        else            /* Parent process */
        {
            close(clnt_sock);
            // 부모 프로세스는 accept()만 수행하므로 통신 용도는 필요 없다.
        }
    }

    close(serv_sock);
    return 0;
}

void read_childproc(int sig)
{
    pid_t pid;
    int status;

    pid = waitpid(-1, &status, WNOHANG);
    // 자식 프로세스 종료 대기
    if (pid > 0)
        printf("removed proc id: %d\n", pid);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}