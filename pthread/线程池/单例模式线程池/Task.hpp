#pragma once

#include <iostream>
#include <pthread.h>

namespace ns_task
{
    class Task
    {
    private:
        int x_;
        int y_;
        char op_; //+/*/%
    public:
        // void (*callback)();
        Task() {}
        Task(int x, int y, char op) : x_(x), y_(y), op_(op)
        {
        }
        std::string Show()
        {
            std::string message = std::to_string(x_);
            message += op_;
            message += std::to_string(y_);
            message += "=?";
            return message;
        }
        int Run()
        {
            int res = 0;
            switch (op_)
            {
            case '+':
                res = x_ + y_;
                break;
            case '-':
                res = x_ - y_;
                break;
            case '*':
                res = x_ * y_;
                break;
            case '/':
                res = x_ / y_;
                break;
            case '%':
                res = x_ % y_;
                break;
            default:
                std::cout << "bug??" << std::endl;
                break;
            }
            std::cout << "当前任务正在被: " << pthread_self() << " 处理: " \
            << x_ << op_ << y_ << "=" << res << std::endl;
            return res;
        }
        int operator()()
        {
            return Run();
        }
        ~Task() {}
    };
}
