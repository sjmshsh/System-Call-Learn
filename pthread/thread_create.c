#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void* thread_run(void* args)
{
    const char* id = (const char*)args;
    while (1)
    {
        printf("我是%s线程, my pid : %d\n", id, getpid());
        sleep(1);
    }
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, thread_run, (void*)"thread_1");
    while(1)
    {
        printf("我是main线程, my pid : %d\n", getpid());
        sleep(1);
    }
    return 0;
}
