#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>

using namespace std;

int main()
{
    // 创建套接字（打开网络文件）
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "socket create error : " << errno << std::endl;
        return 1;
    }
    cout << "sock = " << sock << endl;
    return 0;
}
