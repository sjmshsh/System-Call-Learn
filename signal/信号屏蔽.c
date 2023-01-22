#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


void show_pending(sigset_t* pending)
{
    printf("cur process pending set : ");
    for (int i = 1; i <= 31; i++)
    {
        if (sigismember(pending, i))
        {
            printf("1");
        }
        else 
        {
            printf("0");
        }
    }
    printf("\n");
}


int main()
{
    // sigset_t是一个位图结构，但是不同的OS的操作是不同的
    // 不能让用户直接修改该变量
    // 需要使用特定的系统调用
    // set是一个变量，这个变量保存在哪里呢
    // 肯定是栈区（用户栈）
    // sigset_t set;
    sigset_t inset, outset;
    // 先清空位图
    sigemptyset(&inset);
    sigemptyset(&outset);
    // 屏蔽2号信号
    sigaddset(&inset, 2);
    // 添加到位图中，并且获得当前进程老的屏蔽字
    sigprocmask(SIG_SETMASK, &inset, &outset);
    sigset_t pending;
    int count = 0;
    while (1)
    {
        // 清空位图
        sigemptyset(&pending);
        // 获取当前pending位图
        sigpending(&pending);
        // 打印pending位图
        show_pending(&pending);
        sleep(1);
        count++;
        if (count == 20)
        {
            sigprocmask(SIG_SETMASK, &outset, NULL);
            printf("对2号信号的屏蔽已经恢复");
        }
    }
    return 0;
}
