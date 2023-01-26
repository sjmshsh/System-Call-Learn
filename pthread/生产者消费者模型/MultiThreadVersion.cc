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

        pthread_mutex_t c_mtx_;
        pthread_mutex_t p_mtx_;
    public:
        RingQueue(int cap = g_cap_default)
            :cap_(cap),
            ring_queue_(cap)
        {
            sem_init(&blank_sem_, 0, cap);
            sem_init(&data_sem_, 0, 0);
            c_step_ = p_step_ = 0;

            pthread_mutex_init(&c_mtx_, nullptr);
            pthread_mutex_init(&p_mtx_, nullptr);
        }
        ~RingQueue() 
        {
            sem_destroy(&blank_sem_);
            sem_destroy(&data_sem_);

            pthread_mutex_destroy(&blank_sem_);
            pthread_mutex_destroy(&data_sem_);
        }

        // 生产接口
        // 多生产和多消费的优势不在这里，而在于并发的获取和处理任务
        void Push(const T &in)
        {
            // 我先把信号量申请好，这样锁释放了之后信号量已经
            // 准备完毕了
            sem_wait(&blank_sem_); //P(空位置)
            pthread_mutex_lock(&p_mtx_);
            // 可以生产了, 可是往哪个位置生产呢？
            ring_queue_[p_step_] = in;
            
            // 它也变成了临界资源
            p_step_++;
            p_step_ %= cap_;
            pthread_mutex_unlock(&p_mtx_);
            sem_post(&data_sem_);
        }

        // 消费接口
        void Pop(T* out)
        {
            sem_wait(&data_sem_); 
            pthread_mutex_lock(&c_mtx_);
            *out = ring_queue_[c_step_];
            c_step_++;
            c_step_ %= cap_;

            pthread_mutex_unlock(&c_mtx_);
            sem_post(&blank_sem_);
        }
    };
}
