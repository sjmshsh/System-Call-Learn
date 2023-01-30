#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <netinet/in.h>

using namespace std;

void Usage(string proc)
{
    cout << "Use this proc as : \n\t" << proc << "server_ip server_port" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage(argv[0]);
        return 0;
    }
    // 1. 创建套接字(udp)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        cerr << "client socket create error, error code : " << errno << endl;
        return 1;
    }
    // 2. 客户端需要显示bind吗？
    // 首先客户端也是必须要有port的，但是客户端不需要显示绑定
    // 因为如果客户端显示bind端口，那么久把客户端程序和一个端口绑定了
    // 而这个端口可能被其他程序占用
    // 服务端口是被公司严格管理的，而客户端端口并不会被明确指名
    // 所以客户端对端口的要求是有就行，端口不需要特定绑定

    struct sockaddr_in sever;
    sever.sin_family = AF_INET;
    sever.sin_port = htons(atoi(argv[2]));
    sever.sin_addr.s_addr = inet_addr(argv[1]);
    const int NUM = 1024;
    char buffer[NUM];
    while(1)
    {
        // a.客户端的数据从哪里来？
        string message;
        cout << "客户端请输入#: ->" << endl;
        cin >> message;

        // 客户端发送数据
        sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr *)&sever, sizeof(sever));
        // 客户端接收数据
        struct sockaddr_in tmp;
        socklen_t len = sizeof(tmp);
        recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&tmp, &len);
        cout << "server say # : " << buffer << endl;
    }
    return 0;
}
