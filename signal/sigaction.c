#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

void handler(int signo)
{
    printf("Get a signo :%d\n", signo);
}

int main()
{
    struct sigaction act;
    // 全部初始化成0
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigaction(2, &act, NULL);
    while (1)
    {
        printf("hello zyq\n");
        sleep(1);
    }
    return 0;
}
