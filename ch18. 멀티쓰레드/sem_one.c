#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem;

void *thread_func1(void *arg)
{
    int thread_num = *(int *)arg;

    sem_wait(&sem);

    printf("Thread %d: Entered the critical section\n", thread_num);
    for(int i = 0; i < 5; i++)
    {
        printf("[%d] count: %d\n", thread_num, i);
        sleep(1);
    }
    printf("Thread %d: Exit the critical section\n", thread_num);

    sem_post(&sem);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int num1 = 1, num2 = 2;

    sem_init(&sem, 0, 1);

    pthread_create(&t1, NULL, thread_func1, &num1);
    pthread_create(&t2, NULL, thread_func1, &num2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&sem);
    return 0;
}