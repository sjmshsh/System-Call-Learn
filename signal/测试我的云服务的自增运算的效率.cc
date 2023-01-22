#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int count = 0;

void handler(int signo)
{
    printf("hello count : %d\n", count);
}

int main()
{
    // 测一下1s对int能增加多少次
    signal(14, handler);
    alarm(1);
    while (1)
    {
        // 这里不能写printf，因为IO要访问外设，效率是很低的，写printf测试结果是不准确的
        ++count;
    }
    return 0;
}

// hello count : 544589087
