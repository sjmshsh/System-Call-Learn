#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include "Sock.hpp"
#include <cstdlib>

#define SIZE 128
#define NUM 64

static void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " port" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    // 1.建立tcp 监听socket
    uint16_t port = (uint16_t)atoi(argv[1]);
    int lisen_sock = Sock::Socket();
    Sock::Bind(lisen_sock, port);
    Sock::Listen(lisen_sock);

    // 2. 创建epoll模型，获得epfd(文件描述符)
    int epfd = epoll_create(SIZE);

    // 3. 先添加listen_sock和它所关心的事件添加到内核
    struct epoll_event ev;
    ev.events = EPOLLIN;
    // ev.data.
    epoll_ctl(epfd, EPOLL_CTL_ADD, lisen_sock, &ev);
    // 4. 事件循环
    volatile bool quit = false;
    struct epoll_event revs[NUM];
    while (!quit)
    {
        int timeout = 1000;
        // 这里传入的数组，仅仅是尝试从内核中拿回来已经就绪的事件
        int n = epoll_wait(epfd, revs, NUM, timeout);
        switch (n)
        {
        case 0:
            std::cout << "time out ..." << std::endl;
            break;
        case -1:
            std::cerr << "epoll error ... " << std::endl;
            break;
        default: // 有事件就绪
            std::cout << "有事件就绪!" << std::endl;
            break;
        }
    }

    close(epfd);
    close(lisen_sock);
    return 0;
}
