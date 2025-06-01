#include "../include/mutex.h"
namespace Xten
{
    Semaphore::Semaphore(uint32_t count )
    {
        if (sem_init(&_semaphore, 0, count))
        {
            std::cout<<"init semaphore error"<<std::endl;
        }
    }
    void Semaphore::wait() // 等待信号量
    {
        if (sem_wait(&_semaphore))
        {
            throw std::logic_error("wait semaphore error");
        }
    }
    void Semaphore::post() // 释放信号量
    {
        if (sem_post(&_semaphore))
        {
            throw std::logic_error("post semaphore error");
        }
    }
    Semaphore::~Semaphore()
    {
        if (sem_destroy(&_semaphore))
        {
            std::cout<<"destory semaphore error"<<std::endl;
        }
    }
}