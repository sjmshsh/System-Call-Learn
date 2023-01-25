#pragma once
#include <iostream>
#include <queue>
#include <pthread.h>

namespace ns_blockqueue
{
    const int default_cap = 5;

    template <class T>
    class BlockQueue
    {
    private:
        std::queue<T> bq_; //我们的阻塞队列
        int cap_;          //队列的元素上限
        pthread_mutex_t mtx_; //保护临界资源的锁
        //1. 当生产满了的时候，就应该不要生产了(不要竞争锁了)，而应该让消费者来消费
        //2. 当消费空了，就不应该消费（不要竞争锁了）,应该让生产者来进行生产
        pthread_cond_t is_full_; //bq_满的， 消费者在改条件变量下等待
        pthread_cond_t is_empty_; //bq_空的，生产者在改条件变量下等待
    private:
        bool IsFull()
        {
            return bq_.size() == cap_;
        }
        bool IsEmpty()
        {
            return bq_.size() == 0;
        }
        void LockQueue()
        {
            pthread_mutex_lock(&mtx_);
        }
        void UnlockQueue()
        {
            pthread_mutex_unlock(&mtx_);
        }
        void ProducterWait()
        {
            //pthread_cond_wait
            //1. 调用的时候，会首先自动释放mtx_!,然后再挂起自己
            //2. 返回的时候，会首先自动竞争锁，获取到锁之后，才能返回！
            pthread_cond_wait(&is_empty_, &mtx_);
        }
        void ConsumerWait()
        {
            pthread_cond_wait(&is_full_, &mtx_);
        }
        void WakeupComsumer()
        {
            pthread_cond_signal(&is_full_);
        }
        void WakeupProducter()
        {
            pthread_cond_signal(&is_empty_);
        }      
    public:
        BlockQueue(int cap = default_cap):cap_(cap)
        {
            pthread_mutex_init(&mtx_, nullptr);
            pthread_cond_init(&is_empty_, nullptr);
            pthread_cond_init(&is_full_, nullptr);
        }
        ~BlockQueue() 
        {
            pthread_mutex_destroy(&mtx_);
            pthread_cond_destroy(&is_empty_);
            pthread_cond_destroy(&is_full_);
        }
    public:
        //const &:输入
        //*: 输出
        //&: 输入输出
        void Push(const T &in)
        {
            LockQueue();
            //临界区
            if(IsFull()){ //bug?
                //等待的，把线程挂起，我们当前是持有锁的！！！
                ProducterWait();
            }
            //向队列中放数据，生产函数
            bq_.push(in);

            if(bq_.size() > cap_/2 ) WakeupComsumer();
            UnlockQueue();
            //WakeupComsumer();

        }

        void Pop(T *out)
        {
            LockQueue();
            //从队列中拿数据，消费函数函数
            if(IsEmpty()){ //bug?
                //无法消费
                ConsumerWait();
            }
            *out = bq_.front();
            bq_.pop();
            
            if(bq_.size() < cap_/2 ) WakeupProducter();
            UnlockQueue();
            
        }
    };
}
