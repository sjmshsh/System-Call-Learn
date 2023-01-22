#pragma once

#include "Reactor.hpp"
#include "Sock.hpp"
#include "Service.hpp"

int Accepter(Event *evp)
{
    std::cout << "有新的链接到来了, 就绪的sock是: " << evp->sock << std::endl;
    while(true)
    {
        int sock = Sock::Accept(evp->sock);
        if (sock < 0)
        {
            std::cout << "Accept Done!" << std::endl;
            break;
        }
        std::cout << "Accept success" << sock << std::endl;
        // 这里呢？？
        // 获取链接成功
        Event *other_ev = new Event();
        other_ev->sock = sock;
        other_ev->R = evp->R; // 为什么要让所有的Event指向自己所属的Reactor呢？
        // recver, sender, errorer就是我们代码中的较顶层，只负责真正的读取！
        other_ev->RegisterCallBack(Recver, Sender, Errorer);

        evp->R->InsertEvent(other_ev, EPOLLIN|EPOLLET);
    }
}
