#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <unistd.h>
#include <pthread.h>

namespace ns_threadpool
{
    const int g_num = 5;

    template <class T>
    class ThreadPool
    {
    private:
        int num_;
        std::queue<T> task_queue_; //该成员是一个临界资源

        pthread_mutex_t mtx_;
        pthread_cond_t cond_;

        static ThreadPool<T> *ins;

    private:
        // 构造函数必须得实现，但是必须的私有化
        ThreadPool(int num = g_num) : num_(num)
        {
            pthread_mutex_init(&mtx_, nullptr);
            pthread_cond_init(&cond_, nullptr);
        }
        ThreadPool(const ThreadPool<T> &tp) = delete;
        //赋值语句
        ThreadPool<T> &operator=(ThreadPool<T> &tp) = delete;

    public:
        static ThreadPool<T> *GetInstance()
        {
            static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            // 当前单例对象还没有被创建
            if (ins == nullptr) //双判定，减少锁的争用，提高获取单例的效率！
            {
                pthread_mutex_lock(&lock);
                if (ins == nullptr)
                {
                    ins = new ThreadPool<T>();
                    ins->InitThreadPool();
                    std::cout << "首次加载对象" << std::endl;
                }
                pthread_mutex_unlock(&lock);
            }

            return ins;
        }

        void Lock()
        {
            pthread_mutex_lock(&mtx_);
        }
        void Unlock()
        {
            pthread_mutex_unlock(&mtx_);
        }
        void Wait()
        {
            pthread_cond_wait(&cond_, &mtx_);
        }
        void Wakeup()
        {
            pthread_cond_signal(&cond_);
        }
        bool IsEmpey()
        {
            return task_queue_.empty();
        }

    public:
        // 在类中要让线程执行类内成员方法，是不可行的！
        // 必须让线程执行静态方法
        static void *Rountine(void *args)
        {
            pthread_detach(pthread_self());
            ThreadPool<T> *tp = (ThreadPool<T> *)args;

            while (true)
            {
                tp->Lock();
                while (tp->IsEmpey())
                {
                    //任务队列为空，线程该做什么呢？？
                    tp->Wait();
                }
                //该任务队列中一定有任务了
                T t;
                tp->PopTask(&t);
                tp->Unlock();

                t();
            }
        }
        void InitThreadPool()
        {
            pthread_t tid;
            for (int i = 0; i < num_; i++)
            {
                pthread_create(&tid, nullptr, Rountine, (void *)this /*?*/);
            }
        }
        void PushTask(const T &in)
        {
            Lock();
            task_queue_.push(in);
            Unlock();
            Wakeup();
        }
        void PopTask(T *out)
        {
            *out = task_queue_.front();
            task_queue_.pop();
        }
        ~ThreadPool()
        {
            pthread_mutex_destroy(&mtx_);
            pthread_cond_destroy(&cond_);
        }
    };

    template <class T>
    ThreadPool<T> *ThreadPool<T>::ins = nullptr;
} // namespace ns_threadpool
