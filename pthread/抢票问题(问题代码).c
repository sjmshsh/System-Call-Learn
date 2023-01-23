#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;
// 抢票逻辑, 10000票, 5个线程同时在抢
// tickets是不是就是所谓的临界资源？是的
// 为了让多个线程进行切换，线程什么时候可能切换（1.时间片到了 2.检测的时间点：从内核态返回用户态的时候）
int tickets = 1000;

void *ThreadRun(void *args)
{
    int id = *(int *)args;
    delete (int *)args;
    while (true)
    {
        if (tickets > 0)
        {
            usleep(1000); // 1s == 1000ms 1ms == 1000us
            cout << "我是[" << id << "] 我要抢的票是: " << tickets << endl;
            // 抢票
            tickets--;
        }
        else
        {
            // 没有票
            break;
        }
    }
}

int main()
{
    pthread_t tid[5];
    for (int i = 0; i < 5; i++)
    {
        int *id = new int(i);
        pthread_create(tid + i, nullptr, ThreadRun, id);
    }
    for (int i = 0; i < 5; i++)
    {
        pthread_join(tid[i], nullptr);
    }
    return 0;
}
