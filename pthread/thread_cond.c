#include <stdio.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <pthread.h>

using namespace std;

#define NUM 5

pthread_mutex_t mtx;
pthread_cond_t cond;

// ctrl thread控制work线程，让它定期运行
void *ctrl(void *args)
{
    string name = (char*)args;
    while(true)
    {
        // 唤醒在条件变量下等待的一个线程, 哪一个？
        pthread_cond_signal(&cond);
        sleep(1);
    }
}

void *work(void *args)
{
    int number = *(int*)args;
    // 我拿到之后就用不到了，所以这里就可以直接delete了
    delete (int*)args;
    while (true)
    {
        // 此处我们的mutex不用，暂时这样，后面解释
        pthread_cond_wait(&cond, &mtx);
        cout << "worker: " << number << " is working ... " << endl;
    }
}

int main()
{
    pthread_mutex_init(&mtx, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_t worker[NUM];
    pthread_t master;

    pthread_create(&master, nullptr, ctrl, (void *)"boss");
    for (int i = 0; i < NUM; i++)
    {
        int *number = new int(i);
        pthread_create(worker + i, nullptr, work, (void *)number);
    }
    for (int i = 0; i < NUM; i++)
    {
        pthread_join(worker[i], nullptr);
    }
    pthread_join(master, nullptr); 
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    return 0;
}
