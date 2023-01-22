#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int signal)
{
    printf("I got a signal, signal id : %d, pid : %d \n", signal, getpid());
}

int main()
{
    // 通过signal函数，把2号动作处理为我们的特定动作
    signal(2, handler);
    while(1) 
    {
        sleep(1);
    }
    return 0;
}
