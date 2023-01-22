#include "Sock.hpp"
#include "Reactor.hpp"
#include "Accepter.hpp"
#include "Util.hpp"

static void Usage(std::string proc)
{
    std::cout << "Usage: " << proc << std::endl;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    // 1. 创建socket，监听
    int listen_sock = Sock::Socket();
    SetNonBlock(listen_sock);
    Sock::Bind(listen_sock, (uint16_t)atoi(argv[1]));
    Sock::Listen(listen_sock);
    
    // 2. 创建Reactor对象
    Reactor *R = new Reactor();
    R->InitReactor();

    // 3. 给Reactor反应堆中加柴火
    // 3.1 有柴火
    Event *evp = new Event;
    evp->sock = listen_sock;
    evp->R = R;
    // Accepter: 链接管理器
    evp->RegisterCallBack(Accepter, nullptr, nullptr);
    // 3.2 将准备好的柴火放入感应堆
    R->InsertEvent(evp, EPOLLIN | EPOLLET);

    // 4. 开始进行事件派发
    int timeout = 1000;
    for( ; ; )
    {
        R->Dispatcher(timeout);
    }
    return 0;
}
