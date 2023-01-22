#pragma once

#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>

// 一般处理IO的时候，我们只有3种接口需要处理
// 处理读取
// 处理写入
// 处理异常

class Event;
class Reactor;

// 函数指针
typedef int (*callback_t)(Event *ev);
#define SIZE 128
#define NUM 64

// 需要让epoll管理的基本节点
class Event
{
public:
    // 对应的文件描述符
    int sock;
    // 对应的sock对应的输入缓冲区
    std::string inbuffer;
    // 对应的sock对应的输出缓冲区
    std::string outbuffer;

    // sock设置回调
    callback_t recver;
    callback_t sender;
    callback_t errer;

    // 试着Event回指Reactor的指针
    Reactor *R;

    Event()
    {
        sock = -1;
        recver = nullptr;
        sender = nullptr;
        errer = nullptr;
        R = nullptr;
    }
    void RegisterCallBack(callback_t _recver, callback_t _sender, callback_t _errer)
    {
        recver = _recver;
        sender = _sender;
        errer = _errer;
    }
};

// 不需要关心任何sock的类型(listen，读，写)
// 如何进行使用该类，对Event进行管理
// Reactor: Event = 1: n
class Reactor
{
private:
    int epfd;
    // 我的Epoll类管理的所有的Event的集合
    std::unordered_map<int, Event *> events;

public:
    Reactor()
        : epfd(-1)
    {
    }
    void InitReactor()
    {
        epfd = epoll_create(SIZE);
        if (epfd < 0)
        {
            std::cerr << "epoll_create error" << std::endl;
            exit(2);
        }
        std::cout << "InitReactor success" << std::endl;
    }
    bool InsertEvent(Event *evp, uint32_t evs)
    {
        // 1. 将sock插入到epoll中
        struct epoll_event ev;
        ev.events = evs;
        ev.data.fd = evp->sock;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, evp->sock, &ev) < 0)
        {
            std::cerr << "epoll_ctl add event failed" << std::endl;
            return false;
        }
        // 2. 将ev本身插入到unordered_map中
        events.insert({evp->sock, evp});
    }
    void DeleteEvent(Event *evp)
    {
        int sock = evp->sock;
        auto iter = events.find(sock);
        if (iter != events.end())
        {
            // 1. 将sock中的sock从epoll中删除它
            epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
            // 2. 将特定的ev的sock从epoll中删除
            events.erase(iter);
            // 3. close
            close(sock);
            // 4. 删除event节点
            delete evp;
        }
    }
    // 关于修改，使其可以进行读写
    bool EnableRW(int sock, bool enable_read, bool enable_write)
    {
        struct epoll_event ev;
        ev.events = EPOLLET | (enable_read ? EPOLLIN : 0) | (enable_write ? EPOLLOUT : 0);
        ev.data.fd = sock;

        if (epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev) < 0)
        {
            std::cerr << "epoll_ctl mod event failed" << std::endl;
            return false;
        }
    }

    bool IsSockOk(int sock)
    {
        auto iter = events.find(sock);
        return iter != events.end();
    }

    // 就绪事件的派发器逻辑
    void Dispatcher(int timeout)
    {
        struct epoll_event revs[NUM];
        int n = epoll_wait(epfd, revs, NUM, timeout);
        for (int i = 0; i < n; i++)
        {
            int sock = revs[i].data.fd;
            uint32_t revents = revs[i].events;
            // 代码差错处理，将所有的错误问题全部转换成为让IO函数去解决
            if (revents & EPOLLERR)
                revents |= (EPOLLIN | EPOLLOUT);
            if (revents & EPOLLHUP)
                revents |= (EPOLLIN | EPOLLOUT);
            // 读数据就绪，可能会有bug，后面解决
            if (revents & EPOLLIN)
            {
                if (IsSockOk(sock) && events[sock]->recver)
                {
                    // 直接调用回调方法执行对应的读取
                    events[sock]->recver(events[sock]);
                }
            }
            if (revents & EPOLLOUT)
            {
                if (IsSockOk(sock) && events[sock]->sender)
                {
                    events[sock]->sender(events[sock]);
                }
            }
        }
    }

    ~Reactor() {}
};
