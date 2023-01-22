#include <stdio.h>
#include <signal.h>

int flag = 0;

void handler(int signo)
{
    flag = 1;
    printf("change flag 0 to 1.\n");
}

int main()
{
    signal(2, handler);

    while (!flag);

    printf("这个进程是正常退出的.\n");
    
    return 0;
}
