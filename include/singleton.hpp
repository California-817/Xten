#ifndef __XTEN_SINGLETON_H__
#define __XTEN_SINGLETON_H__
#include "const.h"
namespace Xten // 封装单例基类
{
    template <class T>
    class singleton
    {
    public:
        static std::shared_ptr<T> &GetInstance()
        {
            static std::once_flag s_flag;
            std::call_once(s_flag, [&]()
                           {  // 不能使用make_shared 因为无法访问T的私有构造函数
                               _instance = std::shared_ptr<T>(new T()); 
                           });
            return _instance;
        }
    protected:
        singleton()
        {
        }
        singleton(const singleton<T> &) = delete;
        singleton<T> &operator=(const singleton<T> &) = delete;

    protected:
        static std::shared_ptr<T> _instance;
    };
    template <class T>
    std::shared_ptr<T> singleton<T>::_instance = nullptr;
}
#endif