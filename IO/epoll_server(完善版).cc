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
    ev.data.fd = lisen_sock;

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
            // 5. 处理就绪事件
            for (int i = 0; i < n; i++)
            {
                int sock = revs[i].data.fd; // 暂时方案
                std::cout << "文件描述符: " << sock << "上面有事件就绪了" << std::endl;
                if (revs[i].events & EPOLLIN)
                {
                    std::cout << "文件描述符: " << sock << "读事件就绪" << std::endl;
                    if (sock == lisen_sock)
                    {
                        std::cout << "文件描述符: " << sock << "链接数据就绪" << std::endl;
                        // 5.1 处理链接事件
                        int fd = Sock::Accept(lisen_sock);
                        if (fd >= 0)
                        {
                            std::cout << "获取新链接成功了: " << fd << std::endl;
                            // 能不能立即读取呢？？不能！
                            struct epoll_event _ev;
                            _ev.events = EPOLLIN;
                            _ev.data.fd = fd;
                            epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &_ev); // 新的fd托管给了epoll
                            std::cout << "已经将" << fd << "托管给epoll了" << std::endl;
                        }
                        else
                        {
                            // DO nothing!
                        }
                    }
                    else
                    {
                        // 5.2 正常的读取处理
                        std::cout << "文件描述符: " << sock << "正常数据就绪" << std::endl;
                        char buffer[1024];
                        ssize_t s = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (s > 0)
                        {
                            buffer[s] = 0;
                            std::cout << "client [" << sock << "]# " << buffer << std::endl;
                        }
                        else if (s == 0)
                        {
                            // 对端关闭链接 
                            std::cout << "client quit " << sock << std::endl;
                            close(sock);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
                            std::cout << "sock: " << sock << "delete from epoll success" << std::endl;
                        }
                        else 
                        {
                            // 读取失败
                            std::cout << "recv error " << sock << std::endl;
                            close(sock);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
                            std::cout << "sock: " << sock << "delete from epoll success" << std::endl;
                        }
                    }
                }
                else if (revs[i].events & EPOLLOUT)
                {
                    // 处理写事件
                }
                else
                {
                    // TODO
                }
            }
            break;
        }
    }

    close(epfd);
    close(lisen_sock);
    return 0;
}
