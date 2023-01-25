#pragma once 

#include <iostream>
#include <vector>
#include <semaphore.h>

namespace ns_ring_queue
{
    const int g_cap_default = 10;

    template<class T>
    class RingQueue
    {
    private:
        std::vector<T> ring_queue_;
        int cap_;
        // 生产者关心空位置资源
        sem_t blank_sem_;
        // 消费者关心数据资源
        sem_t data_sem_;

        int c_step_;
        int p_step_;
    public:
        RingQueue(int cap = g_cap_default)
            :cap_(cap),
            ring_queue_(cap)
        {
            sem_init(&blank_sem_, 0, cap);
            sem_init(&data_sem_, 0, 0);
            c_step_ = p_step_ = 0;
        }
        ~RingQueue() 
        {
            sem_destroy(&blank_sem_);
            sem_destroy(&data_sem_);
        }

        // 生产接口
        void Push(const T &in)
        {
            int index = 0;
            sem_wait(&blank_sem_); //P(空位置)
            // 可以生产了, 可是往哪个位置生产呢？
            ring_queue_[p_step_] = in;
            sem_post(&data_sem_); //V(数据)
            p_step_++;
            p_step_ %= cap_;
        }

        // 消费接口
        void Pop(T* out)
        {
            sem_wait(&data_sem_); 
            *out = ring_queue_[c_step_];
            sem_post(&blank_sem_);
            c_step_++;
            c_step_ %= cap_;
        }
    };
}
