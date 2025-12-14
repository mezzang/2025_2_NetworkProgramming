#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem;

void *thread1_func(void *arg)
{
    printf("Thread 1: Start work\n");
    sleep(1);
    printf("Thread 1: Finish work\n");
    sem_post(&sem);
    return NULL;
}

void *thread2_func(void *arg)
{
    sem_wait(&sem);
    printf("Thread 2: Start work\n");
    sleep(1);
    printf("Thread 2: Finish work\n");
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    sem_init(&sem, 0, 0);

    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&sem);
    return 0;
}