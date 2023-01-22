#pragma once

#include <vector>
#include <string>
#include <cerrno>
#include "Util.hpp"
#include "Reactor.hpp"

#define ONCE_SIZE 128

// 1 本轮读取全部完成
// -1 读取出错
// 0 对端关闭连接
static int RecverCore(int sock, std::string &inbuffer)
{
    while (true)
    {
        char buffer[ONCE_SIZE];
        ssize_t s = recv(sock, buffer, ONCE_SIZE, 0);
        if (s > 0)
        {
            buffer[s] = '\0';
            // 读取成功
            inbuffer += buffer;
        }
        else if (s < 0)
        {
            if (errno == EINTR)
            {
                // IO被信号打断，概率特别低
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 1. 读完，底层没有数据了
                return 1; // success
            }
            // 2. 真正出错了
            return -1;
        }
        else
        {
            // s == 0
            return 0;
        }
    }
}

int Recver(Event *evp)
{
    std::cout << "Recver been called" << std::endl;
    // 1. 真正的读取
    int result = RecverCore(evp->sock, evp->inbuffer);
    if (result <= 0)
    {
        // 差错处理
        if (evp->errer)
        {
            evp->errer(evp);
        }
        return -1;
    }
    // 1+2X33+4X3

    // 2. 分包-- 一个或者多个报文 --解决粘包问题
    std::vector<std::string> tokens;
    std::string sep = "X";
    SplitSegment(evp->inbuffer, &tokens, sep);
    // 3. 反序列化 -- 针对一个报文 -- 提取有效参与计算或者存储的信息
    for (auto &seg : tokens)
    {
        std::string data1, data2;
        // 就是和业务强相关了
        if (Deserialize(seg, &data1, &data2))
        {
            // 4. 业务逻辑-- 得到结果
            int x = atoi(data1.c_str());
            int y = atoi(data2.c_str());
            int z = x + y;
            // 5. 构建响应 -- 添加到evp->outbuffer
            std::string res = data1;
            res += "+";
            res += data2;
            res += "=";
            res += std::to_string(z);
            res += sep;

            evp->outbuffer += res; // 发送数据
        }
    }
    // 6. 尝试直接间接进行发送 -- 后续说明
    // 必须条件成熟了(写事件就绪)，你才能发送？？
    // 一般只要将报文处理完毕，才需要发送
    // 写事件一般基本都是就绪的，但是用户不一定是就绪的！
    // 对于写事件，我们通常是按需设置！！
    if (!(evp->outbuffer).empty())
    {
        // 写打开的时候，默认就是就绪的！即便是发送缓冲区已经满了
        // epoll只要用户重新设置了OUT事件，EPOLLOUT至少会再触发一次！
        evp->R->EnableRW(evp->sock, true, true);
    }
}

// 1：全部将数据发送完成
// 0：数据没有发完，但是不能再发了
// -1：发送失败哦
int SenderCore(int sock, std::string &outbuffer)
{
    while (true)
    {
        int total = 0; // 本轮累计发送的数据量
        const char *start = outbuffer.c_str();
        int size = outbuffer.size();
        ssize_t curr = send(sock, start + total, size - total, 0);
        if (curr > 0)
        {
            total += curr; // 累计
            if(total == size)
            {
                // 全部将数据发送完成
                outbuffer.clear();
                return 1;
            }
        }
        else 
        {
            // 数据没有发完，但是不能再发了!
            if (errno == EINTR)
            {
                continue;
            }
            // 数据没发完，但是不能再发了
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                outbuffer.erase(0, total);
                return 0;
            }
            return -1;
        }
    }
}

int Sender(Event *evp)
{
    std::cout << "Sender been called" << std::endl;
    int result = SenderCore(evp->sock, evp->outbuffer);
    if (result == 1)
    {
        evp->R->EnableRW(evp->sock, true, false); // 按需设置
    }
    else if(result == 0)
    {
        // 你可以什么都不做
        evp->R->EnableRW(evp->sock, true, true); // 按需设置
    }
    else 
    {
        if(evp->errer)
        {
            evp->errer(evp);
        }
    }
}

int Errorer(Event *evp)
{
    std::cout << "Errorer been called" << std::endl;
    evp->R->DeleteEvent(evp);
}
