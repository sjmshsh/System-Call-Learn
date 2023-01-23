#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void* thread_run(void* args)
{
    pthread_detach(pthread_self());
    int num = *(int*)args;
    while (1)
    {
        printf("我是新线程%d, 我的id是%u\n", num, pthread_self());
        sleep(10);
        break;
    }
    return (void*)111;
}

#define NUM 1

int main()
{
    // 创建多个线程
    pthread_t tid[NUM];
    for (int i = 0; i < NUM; i++)
    {
        pthread_create(&tid[i], NULL, thread_run, (void*)&i);
        sleep(1);
    }
    // 等待tid[0]线程退出
    void* status = NULL;
    pthread_join(tid[0], &status);
    printf("ret : %d\n", (int*)status);
    return 0;
}
