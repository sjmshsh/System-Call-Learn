#pragma once

#include <iostream>
#include <unistd.h>
#include <string>
#include <queue>
#include <pthread.h>

namespace ns_threadpool
{
    const int g_num = 5;
    template <class T>
    class ThreadPool
    {
    private:
        int num_;
        std::queue<T> task_queue_; // 该成员是一个临界资源
        pthread_mutex_t mtx_;
        pthread_cond_t cond_;

    public:

        void Lock()
        {
            pthread_mutex_lock(&mtx_);
        }

        void Unlock()
        {
            pthread_mutex_unlock(&mtx_);
        }

        ThreadPool(int num = g_num)
            : num_(num)
        {
            pthread_mutex_init(&mtx_, nullptr);
            pthread_cond_init(&cond_, nullptr);
        }

        bool IsEmpty()
        {
            return task_queue_.empty();
        }

        void Wait()
        {
            pthread_cond_wait(cond_, mtx_);
        }

        void PopTask(T *out)
        {
            *out = task_queue_.front();
            task_queue_.pop();
        }

        void Wakeup()
        {
            pthread_cond_signal(&cond_, &mtx_);
        }

        // 在类中要让线程执行类内成员方法是不可行的
        // 因为类内的话会有一个默认的this指针，因此语法层面就报错了
        // 因此必须让线程执行静态方法
        static void *Rountine(void *args)
        {
            pthread_detach(pthread_self());
            ThreadPool<T> *tp = (ThreadPool<T> *)args;
            while (true)
            {
                tp->Lock();
                while (tp->IsEmpty())
                {
                    // 任务队列为空，线程该做什么呢?
                    tp->Wait();
                }
                // 该任务队列中一定有任务了
                T t;
                tp->PopTask(&t);
                tp->Unlock();
                // 在锁外面处理任务
                t();
            }
        }

        void InitThreadPool()
        {
            pthread_t tid;
            for (int i = 0; i < num_; i++)
            {
                pthread_create(&tid, nullptr, Rountine, (void*)this);
            }
        }

        void PushTask(const T& in)
        {
            Lock();
            task_queue_.push(in);
            Unlock();
            Wakeup();
        }

        ~ThreadPool() 
        {
            pthread_mutex_destroy(&mtx_);
            pthread_cond_destroy(&cond_);
        }
    };
}
