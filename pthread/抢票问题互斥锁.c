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

// 对临界区加锁
class Ticket
{
private:
    int tickets;
    pthread_mutex_t mtx;
public:
    Ticket() : tickets(1000)
    {
        pthread_mutex_init(&mtx, nullptr);
    }

    bool GetTicket()
    {
        // res变量是否被所有线程共享呢？不是的
        // 因为这个东西是局部变量，我在线程的私有栈里面创建的，没有线程安全问题
        bool res = true;
        pthread_mutex_lock(&mtx);
        // 执行这部分代码的执行流就互斥的，是串行执行的
        if (tickets > 0)
        {
            usleep(1000); // 1s == 1000ms 1ms == 1000us
            cout << "我是[" << pthread_self() << "] 我要抢的票是: " << tickets << endl;
            // 抢票
            tickets--;
        }
        else
        {
            // 没有票
            printf("票已经被抢空了");
            res = false;
        }
        pthread_mutex_unlock(&mtx);
        return res;
    }

    ~Ticket()
    {
        pthread_mutex_destroy(&mtx);
    }
};

void *ThreadRun(void *args)
{
    Ticket *t = (Ticket *)args;
    while (true)
    {
        if (!t->GetTicket())
        {
            break;
        }
    }
}

int main()
{
    Ticket *t = new Ticket();
    pthread_t tid[5];
    for (int i = 0; i < 5; i++)
    {
        int *id = new int(i);
        pthread_create(tid + i, nullptr, ThreadRun, (void *)t);
    }
    for (int i = 0; i < 5; i++)
    {
        pthread_join(tid[i], nullptr);
    }
    return 0;
}
