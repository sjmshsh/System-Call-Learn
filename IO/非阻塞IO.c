#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

void SetNonBlock(int fd)
{
    // 获得对应文件描述符的文件状态标志
    int fl = fcntl(fd, F_GETFL);
    if (fl < 0)
    {
        perror("fcntl");
        return;
    }
    fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

int main()
{
    // 把read设置为非阻塞
    SetNonBlock(0);
    while(1)
    {
        char buffer[1024];
        ssize_t s = read(0, buffer, sizeof(buffer) - 1);
        if (s > 0)
        {
            buffer[s] = '\0';
            write(1, buffer, strlen(buffer));
            printf("read success, s : %d, errno : %d\n", s, errno);
        }
        else 
        {
            printf("read fail, s : %d, errno : %d\n", s, errno);
        }
        sleep(1);
    }
    return 0;
}
