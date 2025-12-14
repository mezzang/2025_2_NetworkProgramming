// 1개의 세마포어 사용 - 동기화가 이루어지지 않음
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem; // 단일 세마포어 선언

void *thread1_func(void *arg)
{
    for (int i = 0; i < 5; i++) {
        sleep(1);
        printf("Thread 1: Finish work (i=%d)\n", i);
        sem_post(&sem); // Thread 2 실행 허용
    }
    return NULL;
}

void *thread2_func(void *arg)
{
    for (int j = 0; j < 5; j++) {
        sem_wait(&sem); // Thread 1의 신호 대기
        sleep(1);
        printf("Thread 2: Finish work (j=%d)\n", j);
    }
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    sem_init(&sem, 0, 0); // 초기값 0 (신호 대기 상태)

    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&sem);
    return 0;
}